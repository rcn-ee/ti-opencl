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
#include <ti/cmem.h>

#include "memory_provider_cmem.h"
#include "dspmem.h"
#include "../error_report.h"

#include "shmem_init_policy_cmem.h"

using namespace tiocl;


template<typename MapPolicy>
CMEM<MapPolicy>::CMEM(const MemoryRange& r) :
    MemoryProvider(r), threshold_ (32 << 20)
{
    MapPolicy::Configure(r);
}

template<typename MapPolicy>
void *CMEM<MapPolicy>::MapToHostAddressSpace (DSPDevicePtr64 dsp_addr, size_t size,
                                                 bool is_read) const
{
    void *host_addr = MapPolicy::Map(dsp_addr, size);
    if (is_read)  CacheInv(host_addr, size);

    return host_addr;
}

template<typename MapPolicy>
void  CMEM<MapPolicy>::UnmapFromHostAddressSpace (void* host_addr, size_t size,
                                                  bool is_write) const
{
    if (host_addr && is_write)  CacheWb(host_addr, size);

    MapPolicy::Unmap(host_addr, size);
}

#define CMEM_SUCCESS 0
template<typename MapPolicy>
bool CMEM<MapPolicy>::CacheInv(void *host_addr, size_t size) const
{
    // Linux 4.4.12/CMEM 4.11: need to be explicit about buffer ownership
    // by calling CMEM_cache{Wb, Inv}, can no longer optimize it away
    // with a total cache flush
    // if (false && size >= threshold_)
    // Linux 4.4.19/CMEM 4.12/PSDK3.1: reverts back to optimization
    if (size >= threshold_)
        return CMEM_cacheWbInvAll() == CMEM_SUCCESS;
    else
        return CMEM_cacheInv(host_addr, size) == CMEM_SUCCESS;
}

template<typename MapPolicy>
bool CMEM<MapPolicy>::CacheWb(void *host_addr, size_t size) const
{
    // Linux 4.4.12/CMEM 4.11: need to be explicit about buffer ownership
    // by calling CMEM_cache{Wb, Inv}, can no longer optimize it away
    // with a total cache flush
    // if (false && size >= threshold_)
    // Linux 4.4.19/CMEM 4.12/PSDK3.1: reverts back to optimization
    if (size >= threshold_)
        return CMEM_cacheWbInvAll() == CMEM_SUCCESS;
    else
        return CMEM_cacheWb(host_addr, size) == CMEM_SUCCESS;
}

template<typename MapPolicy>
bool CMEM<MapPolicy>::CacheWbInv(void *host_addr, size_t size) const
{
    if (size >= threshold_)
        return CMEM_cacheWbInvAll() == CMEM_SUCCESS;
    else
        return CMEM_cacheWbInv(host_addr, size) == CMEM_SUCCESS;
}

template<typename MapPolicy>
size_t CMEM<MapPolicy>::MinAllocationBlockSize() const
{ return 4096; }

template<typename MapPolicy>
size_t CMEM<MapPolicy>::MinAllocationAlignment() const
{ return 4096; }



CMEMMapPolicyPersistent::CMEMMapPolicyPersistent()
     : host_addr_(0), xlate_dsp_to_host_offset_(0), dsp_addr_adjust_(0)
{ }

void CMEMMapPolicyPersistent::Configure(const MemoryRange &r)
{
    dsp_addr_        = r.GetBase();
    size_            = r.GetSize();
    dsp_addr_adjust_ = r.GetAdjust();


    DSPDevicePtr64 cmem_addr = dsp_addr_;

#if !defined (DEVICE_AM57)
    if (dsp_addr_ >= 0x80000000 && dsp_addr_ < 0xFFFFFFFF)
        cmem_addr = dsp_addr_ - 0x80000000 + 0x800000000ULL;
#endif

    host_addr_ = CMEM_map(cmem_addr, size_);
    if (! host_addr_)
        ReportError(ErrorType::Fatal, ErrorKind::CMEMMapFailed,
                    dsp_addr_, size_ >> 20);
    xlate_dsp_to_host_offset_ = (int64_t)host_addr_ - dsp_addr_;

    ReportTrace("CMEM Persistent CMEM_map 0x%llx, %p, %lld KB\n",
                cmem_addr, host_addr_, size_ >> 10);
}

CMEMMapPolicyPersistent::~CMEMMapPolicyPersistent()
{
    if (dsp_addr_ == 0) return;

    if (host_addr_ != NULL) CMEM_unmap(host_addr_, size_);

    ReportTrace("CMEM Persistent CMEM_unmap %p, %lld KB\n",
                host_addr_, size_ >> 10);

#if 0
    // disable until CMEM provides a registerAllocPhys() routine
    CMEM_AllocParams params = CMEM_DEFAULTPARAMS;
    params.flags = CMEM_CACHED;

    DSPDevicePtr64 cmem_addr = dsp_addr_ - dsp_addr_adjust_;

#if !defined (DEVICE_AM57)
    if (dsp_addr_ >= 0x80000000 && dsp_addr_ < 0xFFFFFFFF)
        cmem_addr = dsp_addr_ - 0x80000000 + 0x800000000ULL;
#endif

    CMEM_freePhys(cmem_addr, &params);

    ReportTrace("CMEM Persistent CMEM_freePhys  0x%llx, %lld KB\n",
                cmem_addr, size_ >> 10);
#endif
}

void *CMEMMapPolicyPersistent::Map(DSPDevicePtr64 dsp_addr, size_t size) const
{

    if (!host_addr_ || dsp_addr < dsp_addr_ || dsp_addr + size > dsp_addr_ + size_)
        ReportError(ErrorType::Fatal,
                    ErrorKind::TranslateAddressOutsideMappedAddressRange,
                    dsp_addr);

    void *host_addr = dsp_addr + (char*)xlate_dsp_to_host_offset_;
    return host_addr;
}

void  CMEMMapPolicyPersistent::Unmap(void* host_addr, size_t size) const
{ }

void CMEMMapPolicyOnDemand::Configure(const MemoryRange &r)
{
    dsp_addr_        = r.GetBase();
    size_            = r.GetSize();
    dsp_addr_adjust_ = r.GetAdjust();
}

void *CMEMMapPolicyOnDemand::Map(DSPDevicePtr64 dsp_addr, size_t size) const
{
    int            align_offset = ((int) dsp_addr) & (MIN_CMEM_MAP_ALIGN - 1);
    DSPDevicePtr64 align_addr = dsp_addr - align_offset;
    size_t         align_size = size + align_offset;

    void* align_host_addr = CMEM_map(align_addr, align_size);

    if (! align_host_addr)  return NULL;

    void* host_addr = (char *) align_host_addr + align_offset;

    ReportTrace("CMEM OnDemand CMEM_map 0x%llx, %p, %d KB\n",
                align_addr, align_host_addr, size >> 10);

    return host_addr;
}

void  CMEMMapPolicyOnDemand::Unmap(void* host_addr, size_t size) const
{
    if (host_addr)
    {
        int      align_offset = ((int) host_addr) & (MIN_CMEM_MAP_ALIGN-1);
        void*    align_host_addr = (char *) host_addr - align_offset;
        size_t   align_size = size + align_offset;

        CMEM_unmap(align_host_addr, align_size);

        ReportTrace("CMEM OnDemand CMEM_unmap  %p, %d KB\n",
                    align_host_addr, size >> 10);
    }
}

template class CMEM<CMEMMapPolicyPersistent>;
template class CMEM<CMEMMapPolicyOnDemand>  ;

