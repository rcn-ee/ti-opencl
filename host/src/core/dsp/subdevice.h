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
#ifndef __DSP_SUB_DEVICE_H__
#define __DSP_SUB_DEVICE_H__

#include "../device.h"
#include "../rootdevice.h"

namespace Coal
{

class DSPSubDevice : public DSPDevice
{
public:
    DSPSubDevice(DSPDevice*                          parent_device,
                 const DSPCoreSet&                   compute_units,
                 const cl_device_partition_property* partition_properties);
    virtual ~DSPSubDevice() {}

    /* Disable default constructor, and assignment */
    DSPSubDevice()                            = delete;
    DSPSubDevice(const DSPSubDevice&)         = delete;
    DSPSubDevice& operator=(const DSPDevice&) = delete;

    void             init()                  override { p_parent->init();                        }
    void             pushEvent(Event* event) override { p_parent->pushEvent(event);              }
    bool             stop()                  override { return p_parent->stop();                 }
    Event*           getEvent(bool& stop)    override { return p_parent->getEvent(stop);         }
    bool             gotEnoughToWorkOn()     override { return p_parent->gotEnoughToWorkOn();    }
    bool             mail_query()            override { return p_parent->mail_query();           }
    float            dspMhz() const          override { return p_parent->dspMhz();               }
    unsigned char    dspID()  const          override { return p_parent->dspID();                }
    int              num_complete_pending()  override { return p_parent->num_complete_pending(); }
    void             dump_complete_pending() override { p_parent->dump_complete_pending();       }
    bool             any_complete_pending()  override { return p_parent->any_complete_pending(); }
    pthread_cond_t*  get_worker_cond()       override { return p_parent->get_worker_cond();      }
    pthread_mutex_t* get_worker_mutex()      override { return p_parent->get_worker_mutex();     }

    DeviceInterface* GetRootDevice()   override { return p_root; }
    const DeviceInterface* GetRootDevice() const override { return p_root; }

    const std::vector<KernelEntry*>* getKernelEntries() const override
    { return static_cast<const DSPRootDevice *>(p_root)->getKernelEntries(); }

    void push_complete_pending(uint32_t idx,
                               class Event* const data,
                               unsigned int cnt = 1) override
    { p_parent->push_complete_pending(idx, data, cnt); }

    bool get_complete_pending(uint32_t idx,
                              class Event*& data)    override
    { return p_parent->get_complete_pending(idx, data); }

    int  mail_from(const DSPCoreSet& compute_units,
                   int* retcode = nullptr)           override
    { return p_parent->mail_from(compute_units, retcode); }

    void mail_to(Msg_t& msg,
                 const DSPCoreSet& compute_units)    override
    { p_parent->mail_to(msg, compute_units); }

    bool IsDeviceType(Type type) override
    {
      if (type == DeviceInterface::T_SubDevice && p_type == type)
        return true;

      return p_parent->IsDeviceType(type);
    }

private:
    DSPDevice* p_root;
};

}
#endif
