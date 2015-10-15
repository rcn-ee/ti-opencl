/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include "device.h"
#include "buffer.h"
#include "kernel.h"
#include "driver.h"

#include "../commandqueue.h"
#include "../events.h"
#include "../memobject.h"
#include "../kernel.h"

#include <stdlib.h>
#include <iostream>
#include <string.h>

#include <unistd.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sched.h>
#include <errno.h>

#include "u_locks_pthread.h"

using namespace Coal;

#define ERR(status, msg) if (status) { printf("OCL ERROR: %s\n", msg); exit(-1); }

#if defined(DEVICE_AM57)
#define MAX_NUM_COMPLETION_PENDING  16
#elif defined(DEVICE_K2X) || defined(DSPC868X)
#define MAX_NUM_COMPLETION_PENDING  32
#else
#error  MAX_NUM_COMPLETION_PENDING not determined for the platform.
#endif

/******************************************************************************
* handle_event_completion
* Blocks on: 1) worker_cond: not stop and no complete_pending is available
*            2) MessageQ: mail_from() on AM57 if no mail is available
* Signals to: 1) worker_cond: after a mail slot becomes available
*                             (could wake up handle_dispatch thread)
*             2) events_cond: when completing current event triggers more
*                             events being pushed to device queue
*                             (could wake up handle_dispatch thread)
******************************************************************************/
bool handle_event_completion(DSPDevice *device)
{
    /*---------------------------------------------------------------------
    * If device is not stopping, and there is no complete_pending available,
    * wait. The handle_dispatch thread will wake me up when either stop is
    * true or complete_pending becomes available.
    *--------------------------------------------------------------------*/
    pthread_mutex_lock(device->get_worker_mutex());
    while ((! device->stop()) && device->num_complete_pending() == 0)
        pthread_cond_wait(device->get_worker_cond(),
                          device->get_worker_mutex());
    pthread_mutex_unlock(device->get_worker_mutex());
    if (device->stop() && !device->any_complete_pending())  return true;

    /*---------------------------------------------------------------------
    * When we can make mail_from() blocking wait on mpm mailbox (K2X), there
    * will be no need to mail_query and sleep here.  Stay tuned. (TODO)
    *--------------------------------------------------------------------*/
    if (! device->mail_query())
    {
#if defined(DEVICE_K2X) || defined(DSPC868X)
        usleep(1);
#endif
        return false;
    }

    int k_id  = device->mail_from();

    /*-------------------------------------------------------------------------
    * If this is a false completion message due to prinft traffic, etc.
    *------------------------------------------------------------------------*/
    if (k_id < 0) return false;

    Event* event;
    bool   done = device->get_complete_pending(k_id, event);
    if (!done) return false;

    /*-------------------------------------------------------------------------
    * A mailbox slot just becomes available, signal the handle_dispatch thread.
    *------------------------------------------------------------------------*/
    pthread_mutex_lock(device->get_worker_mutex());
    if (device->num_complete_pending() < MAX_NUM_COMPLETION_PENDING)
        pthread_cond_broadcast(device->get_worker_cond());
    pthread_mutex_unlock(device->get_worker_mutex());

    KernelEvent    *e  = (KernelEvent *) event;
    DSPKernelEvent *ke = (DSPKernelEvent *)e->deviceData();
    ke->free_tmp_bufs();

    CommandQueue *queue = 0;
    cl_command_queue d_queue = 0;
    cl_command_queue_properties queue_props = 0;

    event->info(CL_EVENT_COMMAND_QUEUE, sizeof(cl_command_queue), &d_queue, 0);
    queue = pobj(d_queue);

    if (queue)
       queue->info(CL_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties),
                   &queue_props, 0);

    // an event may be released once it is Complete
    if (queue_props & CL_QUEUE_PROFILING_ENABLE)
       event->updateTiming(Event::End);
    event->setStatus(Event::Complete);

    return false;
}


/******************************************************************************
* handle_event_dispatch
* Blocks on: 1) events_cond: when no available events are in device queue
*            2) worker_cond: number of msgs in mail has exceeded threshold
* Signals to: 1) worker_cond: after stop becomes true
*                             (could wake up handle_completion thread)
*             2) worker_cond: after an event is dispatch to device
*                             (could wake up handle_completion thread)
******************************************************************************/
bool handle_event_dispatch(DSPDevice *device)
{
    bool       stop = false;
    cl_int     errcode;
    Event *    event;

    /*---------------------------------------------------------------------
    * Blocking if no available event in device queue.  This handle_dispatch
    * will be waken up once events are pushed into device queue.
    *--------------------------------------------------------------------*/
    event = device->getEvent(stop);

    /*---------------------------------------------------------------------
    * Ensure we have a good event and we don't have to stop.
    * When we need to stop, also signal the handle_completion thread.
    *--------------------------------------------------------------------*/
    if (stop)
    {
        pthread_mutex_lock(device->get_worker_mutex());
        pthread_cond_broadcast(device->get_worker_cond());
        pthread_mutex_unlock(device->get_worker_mutex());
        return true;
    }
    if (!event) return false;

    /*---------------------------------------------------------------------
    * Get info about the event and its command queue
    *--------------------------------------------------------------------*/
    Event::Type                 t = event->type();

    /*---------------------------------------------------------------------
    * If there are enough MSGs in the mail for DSP to run, do not dispatch.
    * Otherwise, we might overrun the available mail slots, MPM mail (K2X)
    * will be busy waiting until an empty mail slot becomes available, while
    * MessageQ mail (AM57) will cause a hang in events conformance test.
    * Note that waiting here will NOT create a deadlock, for two reasons:
    * 1) In critical section, it does not try to acquire another lock.
    * 2) Only handle_completion thread can wake up this thread.  Because
    *    num_complete_pending >= MAX_NUM_COMPLETION_PENDING, handle_completion
    *    thread will NOT be waiting for this handle_dispatch thread
    *    and will be waiting for mails from DSP.
    *--------------------------------------------------------------------*/
    pthread_mutex_lock(device->get_worker_mutex());
    while ((t == Event::NDRangeKernel || t == Event::TaskKernel) &&
           device->num_complete_pending() >= MAX_NUM_COMPLETION_PENDING)
        pthread_cond_wait(device->get_worker_cond(),
                          device->get_worker_mutex());
    pthread_mutex_unlock(device->get_worker_mutex());

    CommandQueue *              queue = 0;
    cl_command_queue            d_queue = 0;
    cl_command_queue_properties queue_props = 0;

    errcode = CL_SUCCESS;

    event->info(CL_EVENT_COMMAND_QUEUE, sizeof(cl_command_queue), &d_queue, 0);
    queue = pobj(d_queue);

    if (queue)
       queue->info(CL_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties),
                   &queue_props, 0);

    if (queue_props & CL_QUEUE_PROFILING_ENABLE)
        event->updateTiming(Event::Start);

    /*---------------------------------------------------------------------
    * Execute the action
    *--------------------------------------------------------------------*/
    switch (t)
    {
        case Event::ReadBuffer:
        case Event::WriteBuffer:
        {
            ReadWriteBufferEvent *e = (ReadWriteBufferEvent *)event;

            if (e->buffer()->flags() & CL_MEM_USE_HOST_PTR) 
            {
                if (t == Event::ReadBuffer)
                     memcpy(e->ptr(), e->buffer()->host_ptr(), e->cb());
                else memcpy(e->buffer()->host_ptr(), e->ptr(), e->cb());
                break;
            }

            DSPBuffer *buf = (DSPBuffer *)e->buffer()->deviceBuffer(device);
            DSPDevicePtr64 data = (DSPDevicePtr64)buf->data() + e->offset();

            if (t == Event::ReadBuffer)
                 Driver::instance()->read(device->dspID(), data, 
					  (uint8_t*)e->ptr(), e->cb());

            else 
                 Driver::instance()->write(device->dspID(), data, 
                                  (uint8_t*)e->ptr(), e->cb());

            break;
        }

        case Event::CopyBuffer:
        {
            CopyBufferEvent *e = (CopyBufferEvent *)event;

            DSPDevicePtr64 src_addr;
            DSPDevicePtr64 dst_addr;

            void *psrc;
            void *pdst;

            if (e->source()->flags() & CL_MEM_USE_HOST_PTR)
                 psrc = (char*)e->source()->host_ptr() + e->src_offset();
            else 
            {
                DSPBuffer *src = (DSPBuffer*)e->source()->deviceBuffer(device);
                src_addr = (DSPDevicePtr64)src->data() + e->src_offset();
                psrc = Driver::instance()->map(device, src_addr, e->cb(), true);
            }

            if (e->destination()->flags() & CL_MEM_USE_HOST_PTR)
                 pdst = (char *)e->destination()->host_ptr() + e->dst_offset();
            else 
            {
                DSPBuffer *dst = (DSPBuffer*)e->destination()->deviceBuffer(device);
                dst_addr = (DSPDevicePtr64)dst->data() + e->dst_offset();
                pdst = Driver::instance()->map(device, dst_addr, e->cb(), false);
            }

            memcpy(pdst, psrc, e->cb());

            if (!(e->source()->flags() & CL_MEM_USE_HOST_PTR))
                Driver::instance()->unmap(device, psrc, src_addr, e->cb(), false);

            if (!(e->destination()->flags() & CL_MEM_USE_HOST_PTR))
                Driver::instance()->unmap(device, pdst, dst_addr, e->cb(), true);
            break;
        }

        case Event::ReadBufferRect:
        case Event::WriteBufferRect:
	{
	   ReadWriteBufferRectEvent *e = (ReadWriteBufferRectEvent *)event;
	   
	   // Calculate the start points for each block of memory referenced
	   DSPDevicePtr64 buf_start;
           uint8_t *      host_start;

	   if (e->buffer()->flags() & CL_MEM_USE_HOST_PTR) 
	      buf_start = (DSPDevicePtr64)e->buffer()->host_ptr();
	   else
	      buf_start = ((DSPBuffer *)e->source()->deviceBuffer(device))
                                                                 ->data();
	   
	   buf_start += e->src_origin(2) * e->src_slice_pitch() + 
	                e->src_origin(1) * e->src_row_pitch()   + 
	                e->src_origin(0);

	   host_start  = (uint8_t *)e->ptr() + 
	                 e->dst_origin(2) * e->dst_slice_pitch() + 
	                 e->dst_origin(1) * e->dst_row_pitch()   + 
		         e->dst_origin(0);

	   // Map the device/host buffers to the appopriate src/dst operands
	   // based on the requested operation (read vs write)
	   DSPDevicePtr64 src_start, dst_start;

	   size_t src_row_pitch, dst_row_pitch;
	   size_t src_slice_pitch, dst_slice_pitch;

	   if (t == Event::ReadBufferRect)
	   {
	      src_start       = buf_start;
	      src_row_pitch   = e->src_row_pitch();
	      src_slice_pitch = e->src_slice_pitch();
	      
	      dst_start       = (DSPDevicePtr64) host_start;
	      dst_row_pitch   = e->dst_row_pitch();
	      dst_slice_pitch = e->dst_slice_pitch();
	   }
	   else
	   {
	      src_start       = (DSPDevicePtr64) host_start;
	      src_row_pitch   = e->dst_row_pitch();
	      src_slice_pitch = e->dst_slice_pitch();
	      
	      dst_start       = buf_start;
	      dst_row_pitch   = e->src_row_pitch();
	      dst_slice_pitch = e->src_slice_pitch();
	   }

	   // The dimensions of the region to be copied gives us our
	   // loop boundaries for copying
	   cl_ulong xdim = e->region(0);
	   cl_ulong ydim = e->region(1);
	   cl_ulong zdim = e->region(2);

	   // Set up the start point
	   DSPDevicePtr64 src_cur_slice = src_start;
	   DSPDevicePtr64 dst_cur_slice = dst_start;

	   // The outer loop handles each z-axis slice
	   // For 2-D copy, will only iterate once (zdim=1)
	   for(cl_uint z = 0; z < zdim; z++)
	   {
	      DSPDevicePtr64 src_cur_row = src_cur_slice;
	      DSPDevicePtr64 dst_cur_row = dst_cur_slice;

	      // The inner loop handles each row of the current slice
	      for(cl_uint y = 0; y < ydim; y++)
	      {
		 // Copy a row
		 if (e->buffer()->flags() & CL_MEM_USE_HOST_PTR) 
		    memcpy((void *)dst_cur_row, (void *)src_cur_row, xdim);
		 else
		 {
		    if (t == Event::ReadBufferRect)
		       Driver::instance()->read(device->dspID(), 
				src_cur_row, (uint8_t *)dst_cur_row, xdim);
		    else
		       Driver::instance()->write(device->dspID(), 
				dst_cur_row, (uint8_t *)src_cur_row, xdim);
		    }
		    
		    // Proceed to next row
		    src_cur_row += src_row_pitch;
		    dst_cur_row += dst_row_pitch;
		}

		// Proceed to next slice
		src_cur_slice += src_slice_pitch;
		dst_cur_slice += dst_slice_pitch;
	    }
            break;
	}

        case Event::CopyBufferRect:
        {
	   CopyBufferRectEvent *e = (CopyBufferRectEvent *)event;

	   // Calculate the offsets into each buffer
	   size_t src_offset, dst_offset;
	   
	   src_offset = e->src_origin(2) * e->src_slice_pitch() + 
	                e->src_origin(1) * e->src_row_pitch()   + 
		        e->src_origin(0);

	   dst_offset = e->dst_origin(2) * e->dst_slice_pitch() + 
		        e->dst_origin(1) * e->dst_row_pitch()   + 
		        e->dst_origin(0);

	   // Set up start points for the copy. If it is a DSP buffer, we'll
	   // need to map the buffer before copying (done in copy loop below)
	   DSPDevicePtr64 src_start, dst_start;

	   if (e->source()->flags() & CL_MEM_USE_HOST_PTR)
	      src_start = (DSPDevicePtr64)e->source()->host_ptr() + src_offset;
	   else 
	   {
	      DSPBuffer *src = (DSPBuffer*)e->source()->deviceBuffer(device);
	      src_start = src->data() + src_offset;
	   }

	   if (e->destination()->flags() & CL_MEM_USE_HOST_PTR)
	      dst_start = (DSPDevicePtr64)e->destination()->host_ptr() + dst_offset;
	   else 
	   {
	      DSPBuffer *dst=(DSPBuffer*)e->destination()->deviceBuffer(device);
	      dst_start = dst->data() + dst_offset;
	   }

	   // The dimensions of the region to be copied 
	   cl_ulong xdim = e->region(0);
	   cl_ulong ydim = e->region(1);
	   cl_ulong zdim = e->region(2);
	   
	   // If we need to map memory we will currently map a slice
	   // at a time.  So determine the size of a 2D slice
	   size_t src_slice_size = ydim * e->src_row_pitch()-e->src_origin(0);
	   size_t dst_slice_size = ydim * e->dst_row_pitch()-e->dst_origin(0);
	    
	   // Set up the initial copy point
	   DSPDevicePtr64 src_cur_slice = src_start;
	   DSPDevicePtr64 dst_cur_slice = dst_start;

	   // The outer loop handles each z-axis slice
	   // For 2-D copy, will only iterate once (zdim=1)
	   for(cl_ulong z = 0; z < zdim; z++)
	   {
	      uint8_t *src_cur_row = (uint8_t *)src_cur_slice;
	      uint8_t *dst_cur_row = (uint8_t *)dst_cur_slice;
	      uint8_t *src_cur_mslice, *dst_cur_mslice;

	      // If necessary, memory map a slice of buffer
	      if (!(e->source()->flags() & CL_MEM_USE_HOST_PTR))
		 src_cur_row = src_cur_mslice = (uint8_t *)
		   Driver::instance()->map(device, src_cur_slice, src_slice_size,true);
		
	      if (!(e->destination()->flags() & CL_MEM_USE_HOST_PTR))
		 dst_cur_row = dst_cur_mslice = (uint8_t *)
		   Driver::instance()->map(device, dst_cur_slice, dst_slice_size,false);

	      // The inner loop handles each row of the current slice
	      for(cl_ulong y = 0; y < ydim; y++)
	      {
		 // Copy current row
		 memcpy(dst_cur_row, src_cur_row, xdim);
		 
		 // Proceed to next row
		 src_cur_row += e->src_row_pitch();
		 dst_cur_row += e->dst_row_pitch();
	      }
		
	      // If necessary, unmap the current slice
	      if (!(e->source()->flags() & CL_MEM_USE_HOST_PTR))
		 Driver::instance()->unmap(device, src_cur_mslice,
                                         src_cur_slice, src_slice_size, false);

	      if (!(e->destination()->flags() & CL_MEM_USE_HOST_PTR))
		 Driver::instance()->unmap(device, dst_cur_mslice,
                                         dst_cur_slice, dst_slice_size, true);

	      // Proceed to next slice
	      src_cur_slice += e->src_slice_pitch();
	      dst_cur_slice += e->dst_slice_pitch();
	    }
            break;
        }

        case Event::ReadImage:
        case Event::WriteImage:
        case Event::CopyImage:
        case Event::CopyBufferToImage:
        case Event::CopyImageToBuffer:
        case Event::MapImage:
        {
            std::cerr << "Images are not supported" << std::endl;
            break;
        }

        case Event::MapBuffer:
        {
            MapBufferEvent *e = (MapBufferEvent *)event;

            /*-----------------------------------------------------------
            * for USE_HOST_PTR, the buffer store is already on the host and
            * map should not be needed.  
            -----------------------------------------------------------*/
            if (e->buffer()->flags() & CL_MEM_USE_HOST_PTR) break;

            if(! e->buffer()->addMapEvent(e))
                ERR(1, "MapBuffer: Range conflicts with previous maps");
            if ((e->flags() & CL_MAP_READ) != 0)
            {
                DSPBuffer *buf = (DSPBuffer *)e->buffer()->deviceBuffer(device);
                DSPDevicePtr64 data = (DSPDevicePtr64)buf->data() + e->offset();
#ifdef DSPC868X
                Driver::instance()->read((int32_t)device->dspID(), data,
                                         (uint8_t *)e->ptr(), e->cb());
#else
                Driver::instance()->cacheInv(data, e->ptr(), e->cb());
#endif
            }
            break;
        }
        case Event::UnmapMemObject:
        {
            UnmapBufferEvent *e = (UnmapBufferEvent *)event;

            /*-----------------------------------------------------------
            * for USE_HOST_PTR, the buffer store is already on the host and
            * unmap should not be needed.  
            -----------------------------------------------------------*/
            if (e->buffer()->flags() & CL_MEM_USE_HOST_PTR) break;

            if (e->buffer()->type() != Coal::MemObject::Buffer &&
                e->buffer()->type() != Coal::MemObject::SubBuffer)
                ERR(1, "UnmapMemObject: MapImage/Unmap not support yet");
            MapBufferEvent *mbe = (MapBufferEvent *) 
                                  e->buffer()->removeMapEvent(e->mapping());
            if (mbe == NULL)
                ERR(1, "UnmapMemObject: host_ptr not from previous maps");

            DSPBuffer *buf = (DSPBuffer *)e->buffer()->deviceBuffer(device);
            DSPDevicePtr64 map_dsp_addr = (DSPDevicePtr64)buf->data()
                                          + mbe->offset();
            Driver::instance()->unmap(device, e->mapping(), map_dsp_addr,
                              mbe->cb(), ((mbe->flags() & CL_MAP_WRITE) != 0));

            if (queue) queue->releaseEvent(mbe); 
            break;
        }

        case Event::NativeKernel:
        {
            std::cerr << "Native Kernels not supported on the DSP" << std::endl;
            break;
        }

        case Event::NDRangeKernel:
        case Event::TaskKernel:
        {
            KernelEvent        *e  = (KernelEvent *) event;
            DSPKernelEvent     *ke = (DSPKernelEvent *)e->deviceData();

            errcode = ke->run(t);
            if (errcode == CL_SUCCESS)
            {
               /*-------------------------------------------------------------
                * If handle_completion thread is waiting on complete_pending
                * becoming available, now it is time to wake it up.
                *------------------------------------------------------------*/
                pthread_mutex_lock(device->get_worker_mutex());
                pthread_cond_broadcast(device->get_worker_cond());
                pthread_mutex_unlock(device->get_worker_mutex());
                return false;
            }
            break;
        }
        default: break;
    }

    /*---------------------------------------------------------------------
    * Cleanup
    *--------------------------------------------------------------------*/

    // an event may be released once it is Complete
    if (queue_props & CL_QUEUE_PROFILING_ENABLE)
       event->updateTiming(Event::End);
    event->setStatus((errcode == CL_SUCCESS) ?  Event::Complete : 
                                               (Event::Status)errcode);

    return false;
}

/******************************************************************************
* dsp_worker_event_dispatch
******************************************************************************/
void *dsp_worker_event_dispatch(void *data)
{
    char *str_nice  = getenv("TI_OCL_WORKER_NICE");
    char *str_sleep = getenv("TI_OCL_WORKER_SLEEP");
    int   env_nice  = (str_nice)  ? atoi(str_nice)  : 4;
    int   env_sleep = (str_sleep) ? atoi(str_sleep) : -1;
    pid_t tid       = syscall(SYS_gettid);

    setpriority(PRIO_PROCESS, tid, env_nice);
    DSPDevice *device = (DSPDevice *)data;

    while (true)
    {
        /*---------------------------------------------------------------------
        * 1. Return true if device->stop() is true, time to stop working.
        * 2. If there is available event, handle the event dispatch:
        *    EITHER an event is dispatched, OR it waits for a mail slot to
        *    become available to dispatch (will be waken up by the completion
        *    thread).
        * 3. If there is no available event, wait for an event to become
        *    available (either the application thread the completion thread
        *    will push more events onto the device queue), or wait for stop
        *    command (will be waken up by application thread).
        *--------------------------------------------------------------------*/
        if (handle_event_dispatch(device))  break;

        if (env_sleep >= 0) usleep(env_sleep);
    }
}

/******************************************************************************
* dsp_worker_event_dispatch
******************************************************************************/
void *dsp_worker_event_completion(void *data)
{
    char *str_nice  = getenv("TI_OCL_WORKER_NICE");
    char *str_sleep = getenv("TI_OCL_WORKER_SLEEP");
    int   env_nice  = (str_nice)  ? atoi(str_nice)  : 4;
    int   env_sleep = (str_sleep) ? atoi(str_sleep) : -1;
    pid_t tid       = syscall(SYS_gettid);

    setpriority(PRIO_PROCESS, tid, env_nice);
    DSPDevice *device = (DSPDevice *)data;

    while (true)
    {
        /*---------------------------------------------------------------------
        * 1. Return true if device->stop() is true and there are no more
        *    complete pending, time to stop working.
        * 2. If there is complete pending, try receive mail and handle
        *    completion.
        *--------------------------------------------------------------------*/
        if (handle_event_completion(device))  break;

        if (env_sleep >= 0) usleep(env_sleep);
    }
}
