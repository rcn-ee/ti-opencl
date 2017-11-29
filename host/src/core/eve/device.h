/******************************************************************************
 * Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef __EVE_DEVICE_H__
#define __EVE_DEVICE_H__

#include "../deviceinterface.h"
#include "message.h"
#include "u_lockable.h"
#include "u_concurrent_map.h"
#include "kernel.h"
#include "../tiocl_thread.h"
#include <string>
#include <list>
#include "../dsp/mbox_interface.h"
#include "../shared_memory_interface.h"
#include "../dsp/device_manager_interface.h"
#include "../kernelentry.h"

namespace Coal
{

class MemObject;
class Event;
class Program;
class Kernel;

using tiocl::SharedMemory;
using tiocl::MemoryRange;
using tiocl::DeviceManager;

class EVEDevice : public DeviceInterface, public Lockable
{
    public:
        EVEDevice(unsigned char eve_id, SharedMemory* shm);
        ~EVEDevice();

        // Disable default constructor, copy constuction and assignment
        EVEDevice()                            =delete;
        EVEDevice(const EVEDevice&)            =delete;
        EVEDevice& operator=(const EVEDevice&) =delete;

        void init();

        cl_int info(cl_device_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const;

        std::vector<KernelEntry*> *getKernelEntries();

        DeviceBuffer *createDeviceBuffer(MemObject *buffer, cl_int *rs);
        DeviceProgram *createDeviceProgram(Program *program);
        DeviceKernel *createDeviceKernel(Kernel *kernel,
                                         llvm::Function *function);
        DeviceKernel *createDeviceBuiltInKernel(Kernel *kernel,
                                                KernelEntry *builtin_kernel);

        cl_int initEventDeviceData(Event *event);
        void freeEventDeviceData(Event *event);

        void   pushEvent(Event *event);
        bool   stop();
        bool   availableEvent();
        Event *getEvent(bool &stop);

        bool hostSchedule() const;
        unsigned char GetEveId() const  { return p_eve_id_; }

        bool  isInClMallocedRegion(void *ptr);

        int  numHostMails(Msg_t& msg) const;
        void mail_to   (Msg_t& msg, unsigned core = 0);
        bool mail_query();
        int  mail_from (int *retcode = nullptr);

        void push_complete_pending(uint32_t idx, class Event* const data,
                                   unsigned int cnt = 1);
        bool get_complete_pending(uint32_t idx, class Event* &data);
        int  num_complete_pending();
        void dump_complete_pending();
        bool any_complete_pending();
        bool gotEnoughToWorkOn();
        pthread_cond_t  *get_worker_cond()  { return &p_worker_cond;  }
        pthread_mutex_t *get_worker_mutex() { return &p_worker_mutex; }

        std::string builtinsHeader(void) const { return "dsp.h"; }

        void init_ulm();

        SharedMemory* GetSHMHandler() const { return p_shmHandler; }

        MemoryRange::Location ClFlagToLocation(cl_mem_flags flags) const;

    private:
        unsigned int       p_cores;
        unsigned int       p_num_events;
        float              p_eve_mhz_;
        pthread_t          p_worker_dispatch;
        pthread_t          p_worker_completion;
        std::list<Event *> p_events;
        pthread_cond_t     p_events_cond;
        pthread_mutex_t    p_events_mutex;
        pthread_cond_t     p_worker_cond;
        pthread_mutex_t    p_worker_mutex;
        bool               p_stop;
        volatile bool      p_exit_acked;
        bool               p_initialized;
        unsigned char      p_eve_id_;
        uint32_t           p_size_local_mem_;

        concurrent_map<uint32_t, class Event*> p_complete_pending;

        MBox              *p_mb;

        SharedMemory      *p_shmHandler;

        const DeviceManager *device_manager_;
        uint32_t           p_pid;
        std::vector<KernelEntry*> p_kernel_entries;
};

}
#endif
