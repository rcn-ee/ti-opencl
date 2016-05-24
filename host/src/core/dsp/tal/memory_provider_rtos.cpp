/******************************************************************************
 * Copyright (c) 2013-2016, Texas Instruments Incorporated - http://www.ti.com/
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
#include <stdlib.h>
#include <vector>
// TODO: generic version of <ti/sysbios/xdcruntime/CacheSupport.h>
//       or <ti/sysbios/hal/Cache.h> if not A15
#include <ti/sysbios/family/arm/a15/Cache.h>

#include "memory_provider_rtos.h"
#include "dspmem.h"
#include "../error_report.h"

#include "shmem_init_policy_rtos.h"

using namespace tiocl;


template<typename MapPolicy>
RTOSMem<MapPolicy>::RTOSMem(const MemoryRange& r) :
    MemoryProvider(r), threshold_(32 << 20)
{
    MapPolicy::Configure(r.GetBase(), r.GetSize());
}

template<typename MapPolicy>
void *RTOSMem<MapPolicy>::MapToHostAddressSpace(DSPDevicePtr64 dsp_addr,
                                               size_t size, bool is_read) const
{
    void *host_addr = MapPolicy::Map(dsp_addr, size);
    if (is_read)  CacheInv(host_addr, size);

    return host_addr;
}

template<typename MapPolicy>
void  RTOSMem<MapPolicy>::UnmapFromHostAddressSpace(void* host_addr,
                                              size_t size, bool is_write) const
{
    if (host_addr && is_write)  CacheWb(host_addr, size);

    MapPolicy::Unmap(host_addr, size);
}

template<typename MapPolicy>
bool RTOSMem<MapPolicy>::CacheInv(void *host_addr, size_t size) const
{
    if (size >= threshold_)
        Cache_wbInvAll();
    else
        Cache_inv(host_addr, size, Cache_Type_ALLD, true);
    return true;
}

template<typename MapPolicy>
bool RTOSMem<MapPolicy>::CacheWb(void *host_addr, size_t size) const
{
    if (size >= threshold_)
        Cache_wbInvAll();
    else
        Cache_wb(host_addr, size, Cache_Type_ALLD, true);
    return true;
}

template<typename MapPolicy>
bool RTOSMem<MapPolicy>::CacheWbInv(void *host_addr, size_t size) const
{
    if (size >= threshold_)
        Cache_wbInvAll();
    else
        Cache_wbInv(host_addr, size, Cache_Type_ALLD, true);
    return true;
}

template<typename MapPolicy>
size_t RTOSMem<MapPolicy>::MinAllocationBlockSize() const
{ return 4096; }

template<typename MapPolicy>
size_t RTOSMem<MapPolicy>::MinAllocationAlignment() const
{ return 4096; }



RTOSMemMapPolicyPersistent::RTOSMemMapPolicyPersistent()
     : host_addr_(0), xlate_dsp_to_host_offset_(0)
{ }

void RTOSMemMapPolicyPersistent::Configure(DSPDevicePtr64 dsp_addr,
                                           uint64_t size)
{
    dsp_addr_  = dsp_addr;
    size_      = size;
    host_addr_ = (void *) ((size_t) dsp_addr);
    xlate_dsp_to_host_offset_ = 0;
}

RTOSMemMapPolicyPersistent::~RTOSMemMapPolicyPersistent()
{
}

void *RTOSMemMapPolicyPersistent::Map(DSPDevicePtr64 dsp_addr, size_t size) const
{
    if (!host_addr_ || dsp_addr < dsp_addr_ || dsp_addr + size > dsp_addr_ + size_)
        ReportError(ErrorType::Fatal,
                    ErrorKind::TranslateAddressOutsideMappedAddressRange,
                    dsp_addr);

    void *host_addr = dsp_addr + (char*)xlate_dsp_to_host_offset_;
    return host_addr;
}

void  RTOSMemMapPolicyPersistent::Unmap(void* host_addr, size_t size) const
{ }

template class RTOSMem<RTOSMemMapPolicyPersistent>;

