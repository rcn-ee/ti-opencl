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

#pragma once

#include <cstdlib>
#include "../tiocl_types.h"
#include "../memory_range.h"

using tiocl::MemoryRange;

namespace tiocl {

// Handles memory related functionality for a given range
// - Mapping/unmapping of memory ranges into the host address space
// - Cache operations
// - Properties of a memory range (alignment, block size etc.)
class MemoryProvider
{
public:
    MemoryProvider(const MemoryRange &r) :
                            range(r) {}

    virtual ~MemoryProvider() {}

    virtual void *MapToHostAddressSpace (DSPDevicePtr64 dsp_addr, size_t size,
                                                 bool is_read) const = 0;
    virtual void  UnmapFromHostAddressSpace (void* host_addr, size_t size,
                                                bool is_write) const = 0;

    virtual bool   CacheInv  (void *host_addr, std::size_t size) const = 0;
    virtual bool   CacheWb   (void *host_addr, std::size_t size) const = 0;
    virtual bool   CacheWbInv(void *host_addr, std::size_t size) const = 0;

    virtual size_t MinAllocationBlockSize() const = 0;
    virtual size_t MinAllocationAlignment() const = 0;

    bool IsAddressInRange(DSPDevicePtr64 addr) const {
        return range.IsAddressInRange(addr);
    }

private:
    const MemoryRange range;

};

} // namespace tiocl