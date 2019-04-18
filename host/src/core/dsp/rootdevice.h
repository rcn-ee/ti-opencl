/******************************************************************************
 * Copyright (c) 2013-2018, Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef __DSP_ROOT_DEVICE_H__
#define __DSP_ROOT_DEVICE_H__

#include "device.h"
#include "../kernelentry.h"

namespace Coal
{

class DSPRootDevice : public DSPDevice
{
public:
    DSPRootDevice(unsigned char dsp_id, SharedMemory* shm);
    virtual ~DSPRootDevice();

    /* Disable default constructor, and assignment */
    DSPRootDevice()                            = delete;
    DSPRootDevice(const DSPRootDevice&)        = delete;
    DSPRootDevice& operator=(const DSPDevice&) = delete;

    void             init()                                      override;
    void             pushEvent(Event* event)                     override;
    bool             stop()                                      override;
    Event*           getEvent(bool& stop)                        override;
    bool             gotEnoughToWorkOn()                         override;
    bool             mail_query()                                override;
    int              mail_from(const DSPCoreSet& compute_units,
                               int* retcode = nullptr)           override;
    void             mail_to(Msg_t& msg,
                             const DSPCoreSet& compute_units)    override;
    int              num_complete_pending()                      override;
    void             dump_complete_pending()                     override;
    bool             any_complete_pending()                      override;
    void             push_complete_pending(uint32_t idx,
                                           class Event* const data,
                                           unsigned int cnt = 1) override;
    bool             get_complete_pending(uint32_t idx,
                                          class Event*& data)    override;

    pthread_cond_t*  get_worker_cond()   override  { return &p_worker_cond;  }
    pthread_mutex_t* get_worker_mutex()  override  { return &p_worker_mutex; }
    DeviceInterface* GetRootDevice()  override  { return this; }
    const DeviceInterface* GetRootDevice() const override { return this; }

    void             init_ulm();
    void             setup_dsp_mhz();
    void             init_builtin_kernels();
    const std::vector<KernelEntry*>* getKernelEntries() const override
                                                 { return &p_kernel_entries; }

private:
    std::list<Event*>               p_events;
    pthread_cond_t                  p_events_cond;
    pthread_mutex_t                 p_events_mutex;
    pthread_cond_t                  p_worker_cond;
    pthread_mutex_t                 p_worker_mutex;
    bool                            p_stop;
    volatile bool                   p_exit_acked;
    bool                            p_initialized;
    MBox*                           p_mb;
    concurrent_map < uint32_t,
                   class Event* >*  p_complete_pending;
    class CoreScheduler*            core_scheduler_;
    pthread_t                       p_worker_dispatch;
    pthread_t                       p_worker_completion;
    std::vector<KernelEntry*>       p_kernel_entries;
};

}
#endif
