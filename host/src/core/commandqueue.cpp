/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2012-2014, Texas Instruments Incorporated - http://www.ti.com/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file commandqueue.cpp
 * \brief Command queue
 */

#include "commandqueue.h"
#include "context.h"
#include "deviceinterface.h"
#include "propertylist.h"
#include "events.h"
#include "util.h"

#include <cstring>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdio.h>

using namespace Coal;

#define ONLY_MAIN_THREAD_CAN_RELEASE_EVENT	0
#define OOO_QUEUE_PUSH_EVENTS_THRESHOLD		64

/******************************************************************************
* CommandQueue::CommandQueue
******************************************************************************/
CommandQueue::CommandQueue(Context *ctx,
                           DeviceInterface *device,
                           cl_command_queue_properties properties,
                           cl_int *errcode_ret)
: Object(Object::T_CommandQueue, ctx), p_device(device),
  p_num_events_in_queue(0), p_num_events_on_device(0),
  p_num_events_completed(0),
  p_properties(properties), p_flushed(true)
{
    // Initialize the locking machinery
    pthread_mutex_init(&p_event_list_mutex, 0);
    pthread_cond_init(&p_event_list_cond, 0);

    // Check that the device belongs to the context
    if (!ctx->hasDevice(device))
    {
        *errcode_ret = CL_INVALID_DEVICE;
        return;
    }
    p_device->init();

    *errcode_ret = checkProperties();
}

/******************************************************************************
* CommandQueue::~CommandQueue()
******************************************************************************/
CommandQueue::~CommandQueue()
{
    cleanReleasedEvents();
    // Free the mutex
    pthread_mutex_destroy(&p_event_list_mutex);
    pthread_cond_destroy(&p_event_list_cond);
}

/******************************************************************************
* cl_int CommandQueue::info
******************************************************************************/
cl_int CommandQueue::info(cl_command_queue_info param_name,
                          size_t param_value_size,
                          void *param_value,
                          size_t *param_value_size_ret) const
{
    void *value = 0;
    size_t value_length = 0;

    union {
        cl_uint cl_uint_var;
        cl_device_id cl_device_id_var;
        cl_context cl_context_var;
        cl_command_queue_properties cl_command_queue_properties_var;
    };

    switch (param_name)
    {
        case CL_QUEUE_CONTEXT:
            SIMPLE_ASSIGN(cl_context, parent());
            break;

        case CL_QUEUE_DEVICE:
            SIMPLE_ASSIGN(cl_device_id, p_device);
            break;

        case CL_QUEUE_REFERENCE_COUNT:
            SIMPLE_ASSIGN(cl_uint, references());
            break;

        case CL_QUEUE_PROPERTIES:
            SIMPLE_ASSIGN(cl_command_queue_properties, p_properties);
            break;

        default:
            return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < value_length)
        return CL_INVALID_VALUE;

    if (param_value_size_ret)
        *param_value_size_ret = value_length;

    if (param_value)
        std::memcpy(param_value, value, value_length);

    return CL_SUCCESS;
}

/******************************************************************************
* cl_int CommandQueue::setProperty
******************************************************************************/
cl_int CommandQueue::setProperty(cl_command_queue_properties properties,
                                 cl_bool enable,
                                 cl_command_queue_properties *old_properties)
{
    if (old_properties)
        *old_properties = p_properties;

    if (enable)
        p_properties |= properties;
    else
        p_properties &= ~properties;

    return checkProperties();
}

/******************************************************************************
* cl_int CommandQueue::checkProperties
******************************************************************************/
cl_int CommandQueue::checkProperties() const
{
    // Check that all the properties are valid
    cl_command_queue_properties properties =
        CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
        CL_QUEUE_PROFILING_ENABLE;

    if ((p_properties & properties) != p_properties)
        return CL_INVALID_VALUE;

    // Check that the device handles these properties
    cl_int result;

    result = p_device->info(CL_DEVICE_QUEUE_PROPERTIES,
                            sizeof(cl_command_queue_properties),
                            &properties,
                            0);

    if (result != CL_SUCCESS)
        return result;

    if ((p_properties & properties) != p_properties)
        return CL_INVALID_QUEUE_PROPERTIES;

    return CL_SUCCESS;
}

/******************************************************************************
* void CommandQueue::flush()
******************************************************************************/
void CommandQueue::flush()
{
    pushEventsOnDevice();
    cleanReleasedEvents();
}

/******************************************************************************
* void CommandQueue::finish()
******************************************************************************/
void CommandQueue::finish()
{
    // As pushEventsOnDevice doesn't remove SUCCESS events, we may need
    // to do that here in order not to be stuck.
    cleanEvents();

    // All the queued events must have completed. When they are, they get
    // deleted from the command queue, so simply wait for it to become empty.
    pthread_mutex_lock(&p_event_list_mutex);

    while (p_num_events_in_queue != 0)
        pthread_cond_wait(&p_event_list_cond, &p_event_list_mutex);

    pthread_mutex_unlock(&p_event_list_mutex);

    cleanReleasedEvents();
}

/******************************************************************************
* cl_int CommandQueue::queueEvent(Event *event)
******************************************************************************/
cl_int CommandQueue::queueEvent(Event *event)
{
    // Let the device initialize the event (for instance, a pointer at which
    // memory would be mapped)
    cl_int rs = p_device->initEventDeviceData(event);

    if (rs != CL_SUCCESS)
        return rs;

    // Append the event at the end of the list
    pthread_mutex_lock(&p_event_list_mutex);

    p_events.push_back(event);
    p_num_events_in_queue += 1;
    p_flushed = false;

    pthread_mutex_unlock(&p_event_list_mutex);

    // Timing info if needed
    if (p_properties & CL_QUEUE_PROFILING_ENABLE)
        event->updateTiming(Event::Queue);

    // Explore the list for events we can push on the device
    pushEventsOnDevice();

    cleanReleasedEvents();

    return CL_SUCCESS;
}

/******************************************************************************
* void CommandQueue::releaseEvent()
******************************************************************************/
void CommandQueue::releaseEvent(Event *e)
{
#if ONLY_MAIN_THREAD_CAN_RELEASE_EVENT
    pthread_mutex_lock(&p_event_list_mutex);
    p_released_events.push_back(e);
    pthread_mutex_unlock(&p_event_list_mutex);
#else
    clReleaseEvent((cl_event) e);
#endif
}

/******************************************************************************
* void CommandQueue::cleanEvents()
******************************************************************************/
void CommandQueue::cleanEvents()
{
    bool is_inorder = 
                  (p_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) == 0;

    pthread_mutex_lock(&p_event_list_mutex);

    // No need to cleanEvents() every time an event finishes, so that we can
    // save on the event traversal time.  16 is a number that can be tuned 
    // (e.g. using ooo example).
    if (p_num_events_completed < 16 && p_num_events_on_device > 0 &&
        p_num_events_in_queue - p_num_events_completed > 0)
    {
        pthread_mutex_unlock(&p_event_list_mutex);
        return;
    }

    std::list<Event *>::iterator it = p_events.begin(), oldit;

    while (it != p_events.end())
    {
        Event *event = *it;

        if (event->status() == Event::Complete)
        {
            // We cannot be deleted from inside us
            event->setReleaseParent(false);
            oldit = it;
            ++it;

            p_num_events_in_queue -= 1;
            p_num_events_completed -= 1;
            p_events.erase(oldit);
            // put Completed events into another list
            // let main thread release/delete them
#if ONLY_MAIN_THREAD_CAN_RELEASE_EVENT
            p_released_events.push_back(event);
#else
            clReleaseEvent((cl_event) event);
#endif
        }
        else if (is_inorder) 
        {
            // In Order Queue events are dispatched and completed in Order
            break;
        }
        else
        {
            ++it;
        }
    }

    // We have cleared the list, so wake up the sleeping threads
    if (p_num_events_in_queue == 0)
        pthread_cond_broadcast(&p_event_list_cond);

    pthread_mutex_unlock(&p_event_list_mutex);

    // Check now if we have to be deleted
    if (references() == 0)
    {
        delete this;
    }
}

/******************************************************************************
* void CommandQueue::cleanReleasedEvents()
* !!! Can only be called by the main thread!!! new/delete, malloc/free are not
* thread safe on ARM, so let main thread handle them SOLELY!
******************************************************************************/
void CommandQueue::cleanReleasedEvents()
{
#if ONLY_MAIN_THREAD_CAN_RELEASE_EVENT
    pthread_mutex_lock(&p_event_list_mutex);

    while (! p_released_events.empty())
    {
        Event *event = p_released_events.front();
        clReleaseEvent((cl_event)event);
        p_released_events.pop_front();
    }

    pthread_mutex_unlock(&p_event_list_mutex);
#endif
}

/******************************************************************************
* void CommandQueue::pushEventsOnDevice()
* Who is calling this function:
* (ready_event, one_event_completed_on_device)
* (not NULL, *    ): worker thread, push till this one ready event
* (    NULL, true ): worker thread, one completes, push rest on this queue
* (    NULL, false): main thread, queued a new event, push this queue
******************************************************************************/
void CommandQueue::pushEventsOnDevice(Event *ready_event,
                                      bool one_event_completed_on_device)
{
    int non_complete_events_traversed = 0;
    bool is_ooo = (p_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) != 0;
    bool do_profile = (p_properties & CL_QUEUE_PROFILING_ENABLE) != 0;

    pthread_mutex_lock(&p_event_list_mutex);

    if (one_event_completed_on_device)
    {
        p_num_events_on_device -= 1;
        p_num_events_completed += 1;
    }

    // No need to push more events on Device if 1) device has already got
    // enough to work on, and 2) not pushing won't cause starvation of this
    // commandqueue.  Not pushing can save p_event_list traversal time.
    // 2 is a QoS number, set to 2 for the time being
    // imagaine there are multiple commandqueues on same device
    if(is_ooo && ready_event == NULL &&
       p_num_events_on_device > 2 && p_device->gotEnoughToWorkOn())
    {
        pthread_mutex_unlock(&p_event_list_mutex);
        return;
    }

    // Explore the events in p_events and push on the device all of them that
    // are :
    //
    // - Not already pushed (in Event::Queued state)
    // - Not after a barrier, except if we begin with a barrier
    // - If we are in-order, only the first event in Event::Queued state can
    //   be pushed

    std::list<Event *>::iterator it = p_events.begin();
    std::list<Event *>::iterator oldit;
    bool first = true;

    // We assume that we will flush the command queue (submit all the events)
    // This will be changed in the while() when we know that not all events
    // are submitted.
    p_flushed = true;

    while (it != p_events.end())
    {
        Event *event = *it;

        cl_int e_status = (cl_int) event->status();
        // If the event is completed, remove it
        if (e_status == Event::Complete)
        {
            event->setReleaseParent(false);
            oldit = it;
            ++it;

            p_num_events_completed -= 1;
            p_num_events_in_queue -= 1;
            p_events.erase(oldit);
            // put Completed events into another list
            // let main thread release/delete them
#if ONLY_MAIN_THREAD_CAN_RELEASE_EVENT
            p_released_events.push_back(event);
#else
            clReleaseEvent((cl_event) event);
#endif
            continue;
        }
        // Question: Should we propagate error everywhere: Q, events in Q?
        //           OpenCL Spec is vague on asynchronous error handling,
        //           except for blocking waits with wait_events_list.
        else if (e_status < 0)
        {
            p_flushed = false;
            break;
        }

        // If OOO queue threshold is met, skip examining the rest of events
        if(ready_event == NULL && 
           non_complete_events_traversed > OOO_QUEUE_PUSH_EVENTS_THRESHOLD)
            break;
        non_complete_events_traversed += 1;

        // We cannot do out-of-order, so we can only push the first event.
        if (!is_ooo && !first)
        {
            p_flushed = false; // There are remaining events.
            break;
        }

        // Stop if we encounter a barrier that isn't the first event in the list.
        if (event->type() == Event::Barrier && !first)
        {
            // We have events to wait, stop
            p_flushed = false;
            break;
        }

        // Completed events and first barriers are out, it remains real events
        // that have to block in-order execution.
        first = false;

        // If the event is not "pushable" (in Event::Queued state), skip it
        // It is either Submitted or Running.
        if (event->status() != Event::Queued)
        {
            // Intended event is scheduled, skip the rest in queue
            if (event == ready_event) break;

            ++it;
            continue;
        }

        // Check that all the waiting-on events of this event are finished
        if (! event->waitEventsAllCompleted())
        {
            p_flushed = false;
            // If we encounter a WaitForEvents event that is not "finished",
            // don't push events after it.
            if (event->type() == Event::WaitForEvents)
                break;

            // The event has its dependencies not already met.
            ++it;
            continue;
        }

        if (event->isInstantaneous())
        {
            // Remove event from the queue, otherwise, another thread may
            // come in and try to set the event status to Complete again
            p_num_events_in_queue -= 1;
            p_events.erase(it);
            p_flushed = (p_num_events_in_queue == 0);
            // Pretend begin pushed to device, to maintain proper counting
            p_num_events_on_device += 1;

            // Set the event as completed. This will call pushEventsOnDevice,
            // again, so release the lock to avoid a deadlock. We also return
            // because the recursive call will continue our work.
            if (p_flushed)
                pthread_cond_broadcast(&p_event_list_cond);
            pthread_mutex_unlock(&p_event_list_mutex);
            event->setStatus(Event::Complete);
            clReleaseEvent((cl_event) event);
            return;
        }

        // The event can be pushed, if we need to
        if (do_profile) event->updateTiming(Event::Submit);

        event->setStatus(Event::Submitted);
        p_num_events_on_device += 1;
        p_device->pushEvent(event);
    }

    if (ready_event != NULL && p_flushed)
        p_flushed = (p_num_events_in_queue == 0);

    if (p_flushed)
        pthread_cond_broadcast(&p_event_list_cond);

    pthread_mutex_unlock(&p_event_list_mutex);
}

/******************************************************************************
* Event **CommandQueue::events(unsigned int &count)
******************************************************************************/
Event **CommandQueue::events(unsigned int &count,
                             bool include_completed_events)
{
    Event **result = NULL;

    pthread_mutex_lock(&p_event_list_mutex);

    count = p_num_events_in_queue;
    if (count > 0)
        result = (Event **)std::malloc(count * sizeof(Event *));

    // Copy each event of the list into result, retaining them
    unsigned int index = 0;
    std::list<Event *>::iterator it = p_events.begin();

    while (it != p_events.end())
    {
        if (! include_completed_events)
        {
            Event *e = *it;
            if (e->status() == Event::Complete)
            {
                ++it;
                continue;
            }
        }

        result[index] = *it;
        result[index]->reference();

        ++it;
        ++index;
    }
    count = index;

    // Now result contains an immutable list of events. Even if the events
    // become completed in another thread while result is used, the events
    // are retained and so guaranteed to remain valid.
    pthread_mutex_unlock(&p_event_list_mutex);

    return result;
}

/******************************************************************************
* Event::Event
******************************************************************************/
Event::Event(CommandQueue *parent,
             Status status,
             cl_uint num_events_in_wait_list,
             const Event **event_wait_list,
             cl_int *errcode_ret)
: Object(Object::T_Event, parent),
  p_status(status), p_device_data(0)
{
    // Initialize the locking machinery
    pthread_cond_init(&p_state_change_cond, 0);
    pthread_mutex_init(&p_state_mutex, 0);

    std::memset(&p_timing, 0, sizeof(p_timing));

    // Check sanity of parameters
    if (!event_wait_list && num_events_in_wait_list)
    {
        *errcode_ret = CL_INVALID_EVENT_WAIT_LIST;
        return;
    }

    if (event_wait_list && !num_events_in_wait_list)
    {
        *errcode_ret = CL_INVALID_EVENT_WAIT_LIST;
        return;
    }

    // Check that none of the events in event_wait_list is in an error state
    for (cl_uint i=0; i<num_events_in_wait_list; ++i)
    {
        if (event_wait_list[i] == 0)
        {
            *errcode_ret = CL_INVALID_EVENT_WAIT_LIST;
            return;
        }
        else if (event_wait_list[i]->status() < 0)
        {
            *errcode_ret = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
            return;
        }
    }

    if (parent && num_events_in_wait_list > 0)
    {
        bool wait_events_in_error_status = false;
        pthread_mutex_lock(&p_state_mutex);
        for (cl_uint i=0; i<num_events_in_wait_list; ++i)
        {
            // if event_wait_list[i] is already COMPLETE, don't add it!!!
            Event *wait_event = (Event *) event_wait_list[i];
            int added = wait_event->addDependentEvent((Event *) this);
            if (added > 0)
                p_wait_events.push_back(wait_event);
            else if (added < 0)
                wait_events_in_error_status = true;
        }
        pthread_mutex_unlock(&p_state_mutex);
        if (wait_events_in_error_status)
            setStatus((Status) CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
    }
}

/******************************************************************************
* void Event::freeDeviceData()
******************************************************************************/
void Event::freeDeviceData()
{
    if (parent() && p_device_data)
    {
        DeviceInterface *device = 0;
        ((CommandQueue *)parent())->info(CL_QUEUE_DEVICE, sizeof(DeviceInterface *), &device, 0);

        device->freeEventDeviceData(this);
    }
}

/******************************************************************************
* Event::~Event()
******************************************************************************/
Event::~Event()
{
    pthread_mutex_destroy(&p_state_mutex);
    pthread_cond_destroy(&p_state_change_cond);
}

/******************************************************************************
* bool Event::isInstantaneous() 
******************************************************************************/
bool Event::isInstantaneous() const
{
    // A dummy event has nothing to do on an execution device and must be
    // completed directly after being "submitted".

    switch (type())
    {
        case Marker:
        case User:
        case Barrier:
        case WaitForEvents:
            return true;

        default:
            return false;
    }
}

/******************************************************************************
* void Event::setStatus
******************************************************************************/
int Event::setStatusHelper(Status status)
{
    int num_dependent_events;
    std::list<CallbackData> callbacks;

    // TODO: If status < 0, terminate all the events depending on us.
    pthread_mutex_lock(&p_state_mutex);
    p_status = status;
    num_dependent_events = p_dependent_events.size();

    // Find the callbacks, retain event if there are callbacks
    std::multimap<Status, CallbackData>::const_iterator it;
    std::pair<std::multimap<Status, CallbackData>::const_iterator,
              std::multimap<Status, CallbackData>::const_iterator> ret;
    ret = p_callbacks.equal_range(status > 0 ? status : Complete);
    for (it=ret.first; it!=ret.second; ++it)
        callbacks.push_back((*it).second);
    if (!callbacks.empty())  clRetainEvent((cl_event) this);

    pthread_cond_broadcast(&p_state_change_cond);
    pthread_mutex_unlock(&p_state_mutex);

    // Call the callbacks, release event afterwards
    for (std::list<CallbackData>::iterator C = callbacks.begin(),
                                           E = callbacks.end(); C != E; ++C)
        (*C).callback((cl_event)this, p_status, (*C).user_data);
    if (!callbacks.empty())  clReleaseEvent((cl_event) this);

    return num_dependent_events;
}

void Event::setStatus(Status status)
{
    if (type() == Event::User || (parent() && status == Complete))
    {
        CommandQueue *cq = (CommandQueue *) parent();
        if (cq != NULL)  clRetainCommandQueue((cl_command_queue) cq);
        bool already_pushed = false;

        int num_dependent_events = setStatusHelper(status);  
        /*---------------------------------------------------------------------
        * From this point on, the event could be dereferenced to 0 and deleted!
        * Thus we cannot call flushQueues(). Need to save these queues.
        *--------------------------------------------------------------------*/

        /*---------------------------------------------------------------------
        * Notify dependent events, remove dependence, and push them if possible
        *--------------------------------------------------------------------*/
        for (int i = 0; i < num_dependent_events; i += 1)
        {
            Event *d_event = p_dependent_events[i];
            CommandQueue *q = (CommandQueue *) d_event->parent();
            if (d_event->removeWaitEvent(this) && q != NULL)  // order!
            {
                q->pushEventsOnDevice(d_event, (cq == q));
                if (cq == q)  already_pushed = true;
            }
        }

        /*---------------------------------------------------------------------
        * Inform our parent to push other events to the device if haven't done
        * so already.  UserEvent's parent is NULL.
        *--------------------------------------------------------------------*/
        if (cq != NULL)
        {
            if (!already_pushed)  cq->pushEventsOnDevice(NULL, true);
            clReleaseCommandQueue((cl_command_queue) cq);
        }
    }
    else
    {
        int num_dependent_events = setStatusHelper(status);

        /*---------------------------------------------------------------------
        * If status is error (< 0), set dependent events status to
        * CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST
        *--------------------------------------------------------------------*/
        if (status < 0)
        {
            printf("OCL ERROR: %s(%d, %s)\n", name(), status,
                                                      ocl_error_str(status));
            for (int i = 0; i < num_dependent_events; i += 1)
                p_dependent_events[i]->setStatus(
                        (Status) CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
        }
    }
}

int Event::addDependentEvent(Event *event)
{
    pthread_mutex_lock(&p_state_mutex);
    if (p_status == Event::Complete)
    {
        pthread_mutex_unlock(&p_state_mutex);
        return 0;
    }
    else if (p_status < 0)
    {
        pthread_mutex_unlock(&p_state_mutex);
        return -1;
    }

    p_dependent_events.push_back(event);
    Object::reference();  // retain this event
    pthread_mutex_unlock(&p_state_mutex);
    return 1;
}

bool Event::removeWaitEvent(Event *event)
{
    bool empty;

    pthread_mutex_lock(&p_state_mutex);
    p_wait_events.remove(event);
    empty = p_wait_events.empty();
    pthread_mutex_unlock(&p_state_mutex);

    CommandQueue *q = (CommandQueue *) event->parent();
    if (q != NULL) q->releaseEvent(event);
    return empty;
}

bool Event::waitEventsAllCompleted()
{
// YUAN TODO: p_wait_events is always shrinking, is lock necessary?
//            it is a little bit faster without having to lock!!!
#if 1
    bool empty;

    pthread_mutex_lock(&p_state_mutex);
    empty = p_wait_events.empty();
    pthread_mutex_unlock(&p_state_mutex);

    return empty;
#else
    return p_wait_events.empty();
#endif
}

/******************************************************************************
* void Event::setDeviceData
******************************************************************************/
void Event::setDeviceData(void *data)
{
    p_device_data = data;
}

/******************************************************************************
* void Event::updateTiming
******************************************************************************/
void Event::updateTiming(Timing timing)
{
    if (timing >= Max)
        return;

    pthread_mutex_lock(&p_state_mutex);

    // Don't update more than one time (NDRangeKernel for example)
    if (p_timing[timing])
    {
        pthread_mutex_unlock(&p_state_mutex);
        return;
    }

    struct timespec tp;
    cl_ulong rs;

    if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0)
        clock_gettime(CLOCK_REALTIME, &tp);

    rs = tp.tv_nsec / 1e3;  // convert to microseconds
    rs += tp.tv_sec * 1e6;  // convert to microseconds

    p_timing[timing] = rs;

    pthread_mutex_unlock(&p_state_mutex);
}

/******************************************************************************
* Event::Status Event::status() const
******************************************************************************/
Event::Status Event::status() const
{
    // HACK : We need const qualifier but we also need to lock a mutex
    Event *me = (Event *)(void *)this;

    pthread_mutex_lock(&me->p_state_mutex);

    Status ret = p_status;

    pthread_mutex_unlock(&me->p_state_mutex);

    return ret;
}

/******************************************************************************
* void Event::waitForStatus(Status status)
******************************************************************************/
void Event::waitForStatus(Status status)
{
    pthread_mutex_lock(&p_state_mutex);

    while (p_status != status && p_status > 0)
    {
        pthread_cond_wait(&p_state_change_cond, &p_state_mutex);
    }

    pthread_mutex_unlock(&p_state_mutex);
}

/******************************************************************************
* void *Event::deviceData()
******************************************************************************/
void *Event::deviceData()
{
    return p_device_data;
}

/******************************************************************************
* void Event::setCallback
******************************************************************************/
void Event::setCallback(cl_int command_exec_callback_type,
                        event_callback callback,
                        void *user_data)
{
    CallbackData data;
    bool call_now = false;

    data.callback = callback;
    data.user_data = user_data;

    pthread_mutex_lock(&p_state_mutex);

    /* if event already in or past command_exec_callback_type, call callback */
    /* cl.h: CL_COMPLETE 0, CL_RUNNING 1, CL_SUBMITTED 2, CL_QUEUED 3 */
    if (command_exec_callback_type >= p_status)
        call_now = true;
    else
        p_callbacks.insert(std::pair<Status, CallbackData>(
                                   (Status)command_exec_callback_type, data) );

    pthread_mutex_unlock(&p_state_mutex);

    if (call_now)
        data.callback((cl_event)this, p_status, data.user_data);
}

/******************************************************************************
* cl_int Event::info
******************************************************************************/
cl_int Event::info(cl_event_info param_name,
                   size_t param_value_size,
                   void *param_value,
                   size_t *param_value_size_ret) const
{
    void *value = 0;
    size_t value_length = 0;

    union {
        cl_command_queue cl_command_queue_var;
        cl_context cl_context_var;
        cl_command_type cl_command_type_var;
        cl_int cl_int_var;
        cl_uint cl_uint_var;
    };

    switch (param_name)
    {
        case CL_EVENT_COMMAND_QUEUE:
            SIMPLE_ASSIGN(cl_command_queue, parent());
            break;

        case CL_EVENT_CONTEXT:
            if (parent())
            {
	         SIMPLE_ASSIGN(cl_context, parent()->parent());
            }
            else
            {
                if (type() == User)
                    SIMPLE_ASSIGN(cl_context, ((UserEvent *)this)->context())
                else
                    SIMPLE_ASSIGN(cl_context, 0);
            }
            break;

        case CL_EVENT_COMMAND_TYPE:
            SIMPLE_ASSIGN(cl_command_type, type());
            break;

        // avoid status() call, if called from callbacks, we deadlock on mutex
        case CL_EVENT_COMMAND_EXECUTION_STATUS:
            SIMPLE_ASSIGN(cl_int, p_status);
            break;

        case CL_EVENT_REFERENCE_COUNT:
            SIMPLE_ASSIGN(cl_uint, references());
            break;

        default:
            return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < value_length)
        return CL_INVALID_VALUE;

    if (param_value_size_ret)
        *param_value_size_ret = value_length;

    if (param_value)
        std::memcpy(param_value, value, value_length);

    return CL_SUCCESS;
}

/******************************************************************************
* cl_int Event::profilingInfo(
******************************************************************************/
cl_int Event::profilingInfo(cl_profiling_info param_name,
                            size_t param_value_size,
                            void *param_value,
                            size_t *param_value_size_ret) const
{
    if (type() == Event::User)
        return CL_PROFILING_INFO_NOT_AVAILABLE;

    // Check that the Command Queue has profiling enabled
    cl_command_queue_properties queue_props;
    cl_int rs;

    rs = ((CommandQueue *)parent())->info(CL_QUEUE_PROPERTIES,
                                          sizeof(cl_command_queue_properties),
                                          &queue_props, 0);

    if (rs != CL_SUCCESS)
        return rs;

    if ((queue_props & CL_QUEUE_PROFILING_ENABLE) == 0)
        return CL_PROFILING_INFO_NOT_AVAILABLE;

    // avoid status() call, if called from callbacks, we deadlock on mutex
    if (p_status != Event::Complete)
        return CL_PROFILING_INFO_NOT_AVAILABLE;

    void *value = 0;
    size_t value_length = 0;
    cl_ulong cl_ulong_var;

    switch (param_name)
    {
        case CL_PROFILING_COMMAND_QUEUED:
            SIMPLE_ASSIGN(cl_ulong, 1000*p_timing[Queue]);
            break;

        case CL_PROFILING_COMMAND_SUBMIT:
            SIMPLE_ASSIGN(cl_ulong, 1000*p_timing[Submit]);
            break;

        case CL_PROFILING_COMMAND_START:
            SIMPLE_ASSIGN(cl_ulong, 1000*p_timing[Start]);
            break;

        case CL_PROFILING_COMMAND_END:
            SIMPLE_ASSIGN(cl_ulong, 1000*p_timing[End]);
            break;

        default:
            return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < value_length)
        return CL_INVALID_VALUE;

    if (param_value_size_ret)
        *param_value_size_ret = value_length;

    if (param_value)
        std::memcpy(param_value, value, value_length);

    return CL_SUCCESS;
}

/******************************************************************************
* const char* Event::name()
******************************************************************************/
const char* Event::name()
{
    switch (type())
    {
        case NDRangeKernel:     return "NDRangeKernel";
        case TaskKernel:        return "TaskKernel";
        case NativeKernel:      return "NativeKernel";
        case ReadBuffer:        return "ReadBuffer";
        case WriteBuffer:       return "WriteBuffer";
        case CopyBuffer:        return "CopyBuffer";
        case ReadImage:         return "ReadImage";
        case WriteImage:        return "WriteImage";
        case CopyImage:         return "CopyImage";
        case CopyImageToBuffer: return "CopyImageToBuffer";
        case CopyBufferToImage: return "CopyBufferToImage";
        case MapBuffer:         return "MapBuffer";
        case MapImage:          return "MapImage";
        case UnmapMemObject:    return "UnmapMemObject";
        case Marker:            return "Marker";
        case AcquireGLObjects:  return "AcquireGLObjects";
        case ReleaseGLObjects:  return "ReleaseGLObjects";
        case ReadBufferRect:    return "ReadBufferRect";
        case WriteBufferRect:   return "WriteBufferRect";
        case CopyBufferRect:    return "CopyBufferRect";
        case User:              return "User";
        case Barrier:           return "Barrier";
        case WaitForEvents:     return "WaitForEvents";
        default:                return "UnknownCLCommand";
    }
}
