/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
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
 * \file cpu/device.h
 * \brief CPU device
 */

#ifndef __CPU_DEVICE_H__
#define __CPU_DEVICE_H__

#include "../deviceinterface.h"

#include <pthread.h>
#include <list>

namespace Coal
{

class MemObject;
class Event;
class Program;
class Kernel;

/**
 * \brief CPU device
 *
 * This class is the base of all the CPU-accelerated OpenCL processing. It
 * creates and manages subclasses such as \c Coal::DeviceBuffer,
 * \c Coal::DeviceProgram and \c Coal::DeviceKernel.
 *
 * This class and the aforementioned ones work together to compile and run
 * kernels using the LLVM JIT, manage buffers, provide built-in functions
 * and do all of this in a multithreaded fashion using worker threads.
 *
 * \see \ref events
 */
class CPUDevice : public DeviceInterface
{
    public:
        CPUDevice();
        ~CPUDevice();

        /**
         * \brief Initialize the CPU device
         *
         * This function creates the worker threads and get information about
         * the host system for the \c numCPUs() and \c cpuMhz functions.
         */
        void init();

        cl_int info(cl_device_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const;

        DeviceBuffer *createDeviceBuffer(MemObject *buffer, cl_int *rs);
        DeviceProgram *createDeviceProgram(Program *program);
        DeviceKernel *createDeviceKernel(Kernel *kernel,
                                         llvm::Function *function);

        cl_int initEventDeviceData(Event *event);
        void freeEventDeviceData(Event *event);

        void pushEvent(Event *event);
        Event *getEvent(bool &stop);

        unsigned int numCPUs() const;   /*!< \brief Number of logical CPU cores on the system */
        float cpuMhz() const;           /*!< \brief Speed of the CPU in Mhz */

    private:
        unsigned int p_cores, p_num_events;
        float p_cpu_mhz;
        pthread_t *p_workers;

        std::list<Event *> p_events;
        pthread_cond_t p_events_cond;
        pthread_mutex_t p_events_mutex;
        bool p_stop, p_initialized;
};

}

#endif
