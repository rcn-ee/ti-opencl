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
#include <stdint.h>
#include "core/tiocl_types.h"
#include "memory_provider_interface.h"

#pragma once

namespace tiocl
{

/****************************************************************************
 * Memory provider based on Linux CMEM
 * Uses a MapPolicy policy class to implement support for Persistent and
 * On Demand variants
 ***************************************************************************/
template <typename MapPolicy>
class RTOSMem : private MapPolicy, public MemoryProvider
{
  public:
    RTOSMem(const MemoryRange &r);
    virtual ~RTOSMem() {}

    virtual void *MapToHostAddressSpace (DSPDevicePtr64 dsp_addr, size_t size,
                                                 bool is_read) const override;
    virtual void  UnmapFromHostAddressSpace (void* host_addr, size_t size,
                                                bool is_write) const override;

    virtual size_t MinAllocationBlockSize() const override;
    virtual size_t MinAllocationAlignment() const override;

    virtual bool   CacheInv  (void *host_addr, size_t size) const override;
    virtual bool   CacheWb   (void *host_addr, size_t size) const override;
    virtual bool   CacheWbInv(void *host_addr, size_t size) const override;

private:
    size_t threshold_;
};

/****************************************************************************
 * Policy class for persistent RTOSMem
 ***************************************************************************/
class RTOSMemMapPolicyPersistent
{
protected:
    RTOSMemMapPolicyPersistent();
    ~RTOSMemMapPolicyPersistent();
    void  Configure (DSPDevicePtr64 dsp_addr,  uint64_t size);
    void *Map       (DSPDevicePtr64 dsp_addr,  size_t   size) const;
    void  Unmap     (void*          host_addr, size_t   size) const;

private:
    DSPDevicePtr64 dsp_addr_;
    uint64_t       size_;
    void*          host_addr_;
    int64_t        xlate_dsp_to_host_offset_;
};

typedef RTOSMem<RTOSMemMapPolicyPersistent> RTOSMemPersistent;

}
