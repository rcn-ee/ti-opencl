/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2012-2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * \file commandqueue.h
 * \brief Command queue and base class for events
 */

#ifndef __COMMANDQUEUE_H__
#define __COMMANDQUEUE_H__

#include "object.h"
#include "icd.h"

#include <CL/cl.h>
#include "tiocl_thread.h"

#include <map>
#include <list>
#include <vector>

namespace Coal
{
  class CommandQueue;
  class Event;
}
struct _cl_command_queue: public Coal::descriptor<Coal::CommandQueue, _cl_command_queue> {};
struct _cl_event : public Coal::descriptor<Coal::Event, _cl_event> {};

namespace Coal
{

class Context;
class DeviceInterface;
class Event;

/**
 * \brief Command queue
 *
 * This class holds a list of events that will be pushed on a given device.
 *
 * More details are given on the \ref events page.
 */
class CommandQueue : public _cl_command_queue, public Object
{
    public:
        CommandQueue(Context *ctx,
                     DeviceInterface *device,
                     cl_command_queue_properties properties,
                     cl_int *errcode_ret);
        ~CommandQueue();

        /**
         * \brief Queue an event
         * \param event event to be queued
         * \return \c CL_SUCCESS if success, otherwise an error code
         */
        cl_int queueEvent(Event *event);

        /**
         * \brief Information about the command queue
         * \copydetails Coal::DeviceInterface::info
         */
        cl_int info(cl_command_queue_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const;

        /**
         * \brief Set properties of the command queue
         * \note This function is deprecated and only there for OpenCL 1.0
         *       compatibility
         * \param properties property to enable or disable
         * \param enable true to enable the property, false to disable it
         * \param old_properties old value of the properties, ignored if NULL
         * \return \c CL_SUCCESS if all is good, an error code if \p properties is
         *         invalid
         */
        cl_int setProperty(cl_command_queue_properties properties,
                           cl_bool enable,
                           cl_command_queue_properties *old_properties);

        /**
         * \brief Check the properties given
         * \return \c CL_SUCCESS if they are valid, an error code otherwise
         */
        cl_int checkProperties() const;

        /**
         * \brief Push events on the device
         *
         * This function implements a big part of what is described in
         * \ref events .
         *
         * It is called by \c Coal::Event::setStatus() when an event is
         * completed, or by \c queueEvent(). Its purpose is to explore the list
         * of queued events (\c p_events) and to call
         * \c Coal::DeviceInterface::pushEvent() for each event meeting its push
         * conditions.
         *
         * \param ready_event is know to be pushable, push events in the
         * queue till this point, skip the events after this one.
         *
         * \param one_event_completed_on_device can be used to differentiate
         * whether this function is called by worker thread when an event is
         * completed, or by main thread's queueEvent().
         *
         * \section conditions Conditions
         *
         * If the command queue has the \c CL_OUT_OF_ORDER_EXEC_MODE_ENABLE
         * property disabled, an event can be pushed only if all the previous
         * ones in the list are completed with success. This way, an event
         * must be completed before any other can be pushed. This ensures
         * in-order execution.
         *
         * If this property is enable, more complex heuristics are used.
         *
         * The event list \c p_events is explored from top to bottom. At each
         * loop iteration, checks are performed to see if the event can be pushed.
         *
         * - When a \c Coal::BarrierEvent is encountered, no more events can be
         *   pushed, except if the \c Coal::BarrierEvent is the first in the list,
         *   as that means there are no other events that can be pushed, so the
         *   barrier can go away
         * - All events that are already pushed or finished are skipped
         * - The wait list of the event is then explored to ensure that all its
         *   dependencies are met.
         * - Finally, if the events passes all the tests, it is either pushed on
         *   the device, or simply set to \c Coal::Event::Complete if it's a
         *   dummy event (see \c Coal::Event::isInstantaneous()).
         */
        void pushEventsOnDevice(Event *ready_event = NULL,
                                bool one_event_completed_on_device = false);

        /**
         * \brief Push an event onto p_release_event list
         *
         * Later main thread will perform release event action.
         */
        void releaseEvent(Event *e);

        /**
         * \brief Remove from the event list completed events
         *
         * This function is called periodically to clean the event list from
         * completed events.
         *
         * It is needed to do that out of \c pushEventsOnDevice() as deleting
         * event may \c dereference() this command queue, and also delete it. It
         * would produce crashes.
         */
        void cleanEvents();

        /**
         * \brief Release events on the released event list
         *
         * This function is called periodically to release the events on the
         * released events list.  This is only performed on the main thread
         * because deleting/freeing memory from worker thread has caused
         * weird memory problems on ARM.
         *
         */
        void cleanReleasedEvents();

        /**
         * \brief Flush the command queue
         *
         * Pushes all the events on the device, and then return. The event
         * don't need to be completed after this call.
         */
        void flush();

        /**
         * \brief Finish the command queue
         *
         * Pushes the events like \c flush() but also wait for them to be
         * completed before returning.
         */
        void finish();

        /**
         * \brief Return all the events in the command queue
         * \note Retains all the events
         * \param count number of events in the event queue
         * \param include_completed_events default to true
         * \return events currently in the event queue
         */
        Event **events(unsigned int &count,
                       bool include_completed_events = true);

#if defined(_SYS_BIOS)
        /**
         * \brief Return host frequency
         */
        cl_ulong getFreq() { return p_freq; }
#endif

    private:
        DeviceInterface *p_device;
        cl_int p_num_events_in_queue;
        cl_int p_num_events_on_device;
        cl_int p_num_events_completed;
        cl_command_queue_properties p_properties;

        std::list<Event *> p_events;
        std::list<Event *> p_released_events;
        pthread_mutex_t p_event_list_mutex;
        pthread_cond_t p_event_list_cond;
        bool p_flushed;
#if defined(_SYS_BIOS)
        cl_ulong p_freq;  // in Hz
#endif
};

/**
 * \brief Base class for all events
 *
 * This class contains logic common to all the events.
 * 
 * Beside handling OpenCL-specific stuff, \c Coal::Event objects do nothing
 * implementation-wise. They do not compile kernels, copy data around, etc.
 * They only contain static and immutable data that is then used by the devices
 * to actually implement the event.
 */
class Event : public _cl_event, public Object
{
    public:
        /**
         * \brief Event type
         * 
         * The allows objects using \c Coal::Event to know which event it is, 
         * and to cast it to the correct sub-class.
         */
        enum Type
        {
            NDRangeKernel = CL_COMMAND_NDRANGE_KERNEL,
            TaskKernel = CL_COMMAND_TASK,
            NativeKernel = CL_COMMAND_NATIVE_KERNEL,
            ReadBuffer = CL_COMMAND_READ_BUFFER,
            WriteBuffer = CL_COMMAND_WRITE_BUFFER,
            CopyBuffer = CL_COMMAND_COPY_BUFFER,
            ReadImage = CL_COMMAND_READ_IMAGE,
            WriteImage = CL_COMMAND_WRITE_IMAGE,
            CopyImage = CL_COMMAND_COPY_IMAGE,
            CopyImageToBuffer = CL_COMMAND_COPY_IMAGE_TO_BUFFER,
            CopyBufferToImage = CL_COMMAND_COPY_BUFFER_TO_IMAGE,
            MapBuffer = CL_COMMAND_MAP_BUFFER,
            MapImage = CL_COMMAND_MAP_IMAGE,
            UnmapMemObject = CL_COMMAND_UNMAP_MEM_OBJECT,
            Marker = CL_COMMAND_MARKER,
            AcquireGLObjects = CL_COMMAND_ACQUIRE_GL_OBJECTS,
            ReleaseGLObjects = CL_COMMAND_RELEASE_GL_OBJECTS,
            ReadBufferRect = CL_COMMAND_READ_BUFFER_RECT,
            WriteBufferRect = CL_COMMAND_WRITE_BUFFER_RECT,
            CopyBufferRect = CL_COMMAND_COPY_BUFFER_RECT,
            User = CL_COMMAND_USER,
            Barrier,
            WaitForEvents
        };

        /**
         * \brief Event status
         */
        enum Status
        {
            Queued = CL_QUEUED,       /*!< \brief Simply queued in a command queue */
            Submitted = CL_SUBMITTED, /*!< \brief Submitted to a device */
            Running = CL_RUNNING,     /*!< \brief Running on the device */
            Complete = CL_COMPLETE    /*!< \brief Completed */
        };

        /**
         * \brief Function that can be called when an event change status
         */
        typedef void (CL_CALLBACK *event_callback)(cl_event, cl_int, void *);

        /**
         * Structure used internally by \c Coal::Event to store for each event
         * status the callbacks to call with the corresponding \c user_data.
         */
        struct CallbackData
        {
            event_callback callback; /*!< Function to call */
            void *user_data;         /*!< Pointer to pass as its third argument */
        };

        /**
         * \brief Timing counters of an event
         */
        enum Timing
        {
            Queue,                   /*!< Time when the event was queued */
            Submit,                  /*!< Time when the event was submitted to the device */
            Start,                   /*!< Time when its execution began on the device */
            End,                     /*!< Time when its execution finished */
            Max                      /*!< Number of items in this enum */
        };

    public:
        /**
         * \brief Constructor
         * \param parent parent \c Coal::CommandQueue
         * \param status \c Status the event has when it is created
         * \param num_events_in_wait_list number of events to wait on
         * \param event_wait_list list of events to wait on
         * \param errcode_ret return value
         */
        Event(CommandQueue *parent,
              Status status,
              cl_uint num_events_in_wait_list,
              const cl_event * event_wait_list,
              cl_int *errcode_ret);

        void freeDeviceData();      /*!< \brief Call \c Coal::DeviceInterface::freeEventDeviceData() */
        virtual ~Event();           /*!< \brief Destructor */

        /**
         * \brief Type of the event
         * \return type of the event
         */
        virtual Type type() const = 0;

        /**
         * \brief Corresponding cl command name of the event
         * \return cl command name of the event
         */
        const char* name();
        
        /**
         * \brief Dummy event
         * 
         * A dummy event is an event that doesn't have to be pushed on a device,
         * it is only a hint for \c Coal::CommandQueue
         * 
         * \return true if the event is dummy
         */
        bool isInstantaneous() const;

        /**
         * \brief Set the event status
         * 
         * This function calls the event callbacks, and
         * \c Coal::CommandQueue::pushEventsOnDevice() if \p status is
         * \c Complete .
         * 
         * \param status new status of the event
         */
        void setStatus(Status status);

        /**
         * \brief Set device-specific data
         * \param data device-specific data
         */
        void setDeviceData(void *data);
        
        /**
         * \brief Update timing info
         * 
         * This function reads current system time and puts it in \c p_timing
         * 
         * \param timing timing event having just finished
         */
        void updateTiming(Timing timing);
        
        /**
         * \brief Status
         * \return status of the event
         */
        Status status() const;
        
        /**
         * \brief Wait for a specified status
         * 
         * This function blocks until the event's status is set to \p status
         * by another thread.
         * 
         * \param status the status the event must have for the function to return
         */
        void waitForStatus(Status status);
        
        /**
         * \brief Device-specific data
         * \return data set using \c setDeviceData()
         */
        void *deviceData();

        /**
         * \brief Add a callback for this event
         * \param command_exec_callback_type status the event must have in order
         *        to have the callback called
         * \param callback callback function
         * \param user_data user data given to the callback
         */
        void setCallback(cl_int command_exec_callback_type,
                         event_callback callback,
                         void *user_data);

        /**
         * \brief Info about the event
         * \copydetails Coal::DeviceInterface::info
         */
        cl_int info(cl_event_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const;

        /**
         * \brief Profiling info
         * \copydetails Coal::DeviceInterface::info
         */
        cl_int profilingInfo(cl_profiling_info param_name,
                             size_t param_value_size,
                             void *param_value,
                             size_t *param_value_size_ret) const;

        /**
         * \brief Call \c Coal::CommandQueue::pushEventsOnDevice() for each command queue 
         * in which this event is queued or each queue with an event waiting on this event
         */
        void flushQueues();       


        /**
         * \brief Add event to p_dependent_events, which will be notified when
         * current event completes. If current event is already complete,
         * no need to add and return 0.  If current event is in ERROR status,
         * no need to add and return -1.  Otherwise, add and return 1..
         * \param event the event to be notified
         */
        int addDependentEvent(Event *event);

        /**
         * \brief Remove event from p_wait_events, which should be waited on
         * before current event can start. When p_wait_events becomes empty,
         * return true to indicate that current event is ready to be pushed.
         * \param event the event to be removed from p_wait_events
         */
        bool removeWaitEvent(Event *event);

        /**
         * \brief Check if there are no more events to wait on before current
         * event can start.
         */
        bool waitEventsAllCompleted();

    private:
        /**
         * \brief Helper function for setStatus()
         * return number of dependent events
         */
        int setStatusHelper(Status status, std::list<CallbackData> &callbacks);

    private:
        pthread_cond_t p_state_change_cond;
        pthread_mutex_t p_state_mutex;

        Status p_status;
        void *p_device_data;
        std::multimap<Status, CallbackData> p_callbacks;

        cl_ulong p_timing[Max];

        // p_wait_events: I should wait after these events complete
        // p_dependent_events: when I complete, I should notify these events
        std::list<const Event *>   p_wait_events;
        std::vector<Event *> p_dependent_events;
};

}

#endif
