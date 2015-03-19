#include "device.h"
#include "buffer.h"
#include "kernel.h"
#include "driver.h"

#include "../commandqueue.h"
#include "../events.h"
#include "../memobject.h"
#include "../kernel.h"

#include <iostream>

#include "u_locks_pthread.h"

using namespace Coal;

void *dsp_worker(void *data)
{
    DSPDevice *device = (DSPDevice *)data;
    bool       stop = false;
    cl_int     errcode;
    Event *    event;

    while (true)
    {
        event = device->getEvent(stop);

        /*---------------------------------------------------------------------
        * Ensure we have a good event and we don't have to stop
        *--------------------------------------------------------------------*/
        if (stop)   break;
        if (!event) continue;

        /*---------------------------------------------------------------------
        * Get info about the event and its command queue
        *--------------------------------------------------------------------*/
        Event::Type                 t = event->type();
        CommandQueue *              queue = 0;
        cl_command_queue_properties queue_props = 0;

        errcode = CL_SUCCESS;

        event->info(CL_EVENT_COMMAND_QUEUE, sizeof(CommandQueue *), &queue, 0);

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
                DSPBuffer *buf = (DSPBuffer *)e->buffer()->deviceBuffer(device);
                DSPDevicePtr data = (DSPDevicePtr)buf->data() + e->offset();

                if (t == Event::ReadBuffer)
                     Driver::instance()->dma_read(device->dspID(), data, 
                                      (uint8_t*)e->ptr(), e->cb());

                else 
                     Driver::instance()->dma_write(device->dspID(), data, 
                                      (uint8_t*)e->ptr(), e->cb());

                break;
            }

            case Event::CopyBuffer:
            {
                std::cerr << "Event type not yet supported" << std::endl;
                break;
            }

            case Event::ReadBufferRect:
            case Event::WriteBufferRect:
            case Event::CopyBufferRect:
            case Event::ReadImage:
            case Event::WriteImage:
            case Event::CopyImage:
            case Event::CopyBufferToImage:
            case Event::CopyImageToBuffer:
            {
                std::cerr << "Event type not yet supported" << std::endl;
                break;
            }
            case Event::MapBuffer:
            case Event::MapImage:
            {
                std::cerr << "Event type not yet supported" << std::endl;
                break;
            }

            case Event::NativeKernel:
            {
                std::cerr << "Event type not yet supported" << std::endl;
                break;
            }

            case Event::NDRangeKernel:
            case Event::TaskKernel:
            {
                KernelEvent        *e  = (KernelEvent *) event;
                DSPKernelEvent     *ke = (DSPKernelEvent *)e->deviceData();

                if (!ke->run()) errcode = CL_INVALID_PROGRAM_EXECUTABLE;
                break;
            }
            default: break;
        }

        /*---------------------------------------------------------------------
        * Cleanup
        *--------------------------------------------------------------------*/
        event->setStatus((errcode == CL_SUCCESS) ?  Event::Complete : 
                                                   (Event::Status)errcode);

        if (queue_props & CL_QUEUE_PROFILING_ENABLE)
           event->updateTiming(Event::End);

        if (queue && errcode == CL_SUCCESS) queue->cleanEvents();
    }

    return 0;
}
