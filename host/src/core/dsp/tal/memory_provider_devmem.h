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
#include <unistd.h>
#include "../tiocl_types.h"
#include "../error_report.h"
#include "memory_provider_interface.h"

#pragma once

namespace tiocl
{

/****************************************************************************
 * Map contents of /dev/mem into host virtual address space
 * Templatized to enable mapping via mpm_transport (K2x)
 * or mmap (AM57x)
 ***************************************************************************/
template <typename MapPolicy>
class DevMem : private MapPolicy, public MemoryProvider
{
public:
    DevMem(const tiocl::MemoryRange &r);
    virtual ~DevMem() {}

    virtual void *MapToHostAddressSpace (DSPDevicePtr64 dsp_addr, size_t size,
                                                 bool is_read) const override;
    virtual void  UnmapFromHostAddressSpace (void* host_addr, size_t size,
                                                bool is_write) const override;

    virtual size_t MinAllocationBlockSize() const override;
    virtual size_t MinAllocationAlignment() const override;

    virtual bool   CacheInv  (void *host_addr, std::size_t size) const override;
    virtual bool   CacheWb   (void *host_addr, std::size_t size) const override;
    virtual bool   CacheWbInv(void *host_addr, std::size_t size) const override;

private:
    size_t       page_size_;
};


#define MULTIPLE_OF_POW2(x, y) (((x) & ((y)-1)) != 0 ? false : true)


template<typename MapPolicy>
DevMem<MapPolicy>::DevMem(const tiocl::MemoryRange& r) :
     MemoryProvider(r), page_size_(sysconf(_SC_PAGE_SIZE))
{
    if (page_size_ <= 0)
    {
        ReportError(ErrorType::Warning, ErrorKind::PageSizeNotAvailable);
        return;
    }

    DSPDevicePtr64 dsp_addr = r.GetBase();
    uint64_t       size     = r.GetSize();

    if (!MULTIPLE_OF_POW2(dsp_addr, page_size_))
        ReportError(ErrorType::Warning,
        ErrorKind::RegionAddressNotMultipleOfPageSize, dsp_addr, page_size_);

    if (!MULTIPLE_OF_POW2(size,     page_size_))
        ReportError(ErrorType::Warning,
        ErrorKind::RegionSizeNotMultipleOfPageSize, size, page_size_);

    MapPolicy::Configure(dsp_addr, size);
}

#undef MULTIPLE_OF_POW2

template<typename MapPolicy>
void *DevMem<MapPolicy>::MapToHostAddressSpace (DSPDevicePtr64 dsp_addr,
                                                size_t size,
                                                bool is_read) const
{
    return MapPolicy::Map(dsp_addr, size);
}

template<typename MapPolicy>
void  DevMem<MapPolicy>::UnmapFromHostAddressSpace (void* host_addr,
                                                    size_t size,
                                                    bool is_write) const
{
    MapPolicy::Unmap(host_addr, size);
}

// Access via /dev/mem is not cached on the host side. No need for
// cache operations
template<typename MapPolicy>
bool DevMem<MapPolicy>::CacheInv(void *host_addr, std::size_t size) const
{ return true; }

template<typename MapPolicy>
bool DevMem<MapPolicy>::CacheWb(void *host_addr, std::size_t size) const
{ return true; }

template<typename MapPolicy>
bool DevMem<MapPolicy>::CacheWbInv(void *host_addr, std::size_t size) const
{ return true; }

template<typename MapPolicy>
size_t DevMem<MapPolicy>::MinAllocationBlockSize() const
{ return 128; }

template<typename MapPolicy>
size_t DevMem<MapPolicy>::MinAllocationAlignment() const
{ return 128; }

} // namespace tiocl
