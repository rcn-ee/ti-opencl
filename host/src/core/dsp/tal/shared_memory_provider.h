/******************************************************************************
 * Copyright (c) 2016, Texas Instruments Incorporated - http://www.ti.com/
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Texas Instruments Incorporated nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include <vector>
#include <map>
#include <cstdlib>
#include <cstdint>

#include "u_lockable.h"
#include "core/shared_memory_interface.h"
#include "memory_provider_factory.h"
#include "../../tiocl_types.h"
#include "heap_manager.h"

#pragma once

namespace tiocl {

typedef std::pair<uint64_t, size_t>                        PhysAddrSizePair;
typedef std::pair<PhysAddrSizePair, MemoryRange::Location> PhysAddrSizeFlagsTriple;
typedef std::map<void*, PhysAddrSizeFlagsTriple>           clMallocMapping;

/****************************************************************************
 * Implementation of the SharedMemory interface
 * MIPolicy: Memory Initialization policy, controls how available memory
 *           ranges are discovered
 * RWPolicy: Handles variations in Read/Write and Map/Unmap functions
 ***************************************************************************/
template <class MIPolicy, class RWPolicy>
class SharedMemoryProvider : public SharedMemory,
                             private MIPolicy, private RWPolicy,
                             public Lockable
{
public:
    SharedMemoryProvider(uint8_t device_id);
    ~SharedMemoryProvider();

    uint64_t AllocateMSMC (size_t   size) override;
    void     FreeMSMC     (uint64_t addr) override;
    uint64_t AllocateGlobal(size_t   size, bool prefer_32bit) override;
    void     FreeGlobal  (uint64_t addr) override;

    void* clMalloc     (size_t size, MemoryRange::Location l) override;
    void  clFree       (void* ptr) override;
    bool  clMallocQuery(void* ptr, uint64_t* p_addr, size_t* p_size) override;


    bool CacheInv   (uint64_t addr, void *host_addr, size_t sz) override;
    bool CacheWb    (uint64_t addr, void *host_addr, size_t sz) override;
    bool CacheWbInv (uint64_t addr, void *host_addr, size_t sz) override;
    bool CacheWbInvAll() override;

    int32_t WriteToShmem(uint64_t dst, uint8_t *src, size_t sz) override;
    int32_t ReadFromShmem (uint64_t src, uint8_t *dst, size_t sz) override ;

    void*   Map(uint64_t addr, size_t sz,
                bool is_read, bool allow_fail) override;

    int32_t Unmap(void *host_addr, uint64_t buf_addr,
                  size_t sz, bool is_write) override;

    uint64_t HeapSize(MemoryRange::Kind k, MemoryRange::Location l) override;
    uint64_t HeapMaxAllocSize(MemoryRange::Kind k, MemoryRange::Location l) override;

private:

    void dsptop_ddr_fixed();
    void dsptop_ddr_extended();
    void dsptop_msmc();

    uint8_t                      device_id_;
    std::vector<MemoryRange>     memory_ranges_;
    tiocl::MemoryProviderFactory mp_factory_;

    typedef utility::HeapManager<DSPDevicePtr,   uint32_t, 
                         utility::MultiProcess<DSPDevicePtr,  uint32_t>> Heap32Bit;
    typedef utility::HeapManager<DSPDevicePtr64, uint64_t, 
                         utility::MultiProcess<DSPDevicePtr64,uint64_t>> Heap64Bit;

    boost::interprocess::managed_shared_memory    heap_segment_;
    Heap64Bit*         ddr_heap1_;  // persistently mapped off-chip memory
    Heap64Bit*         ddr_heap2_;  // ondemand mapped off-chip memory
    Heap64Bit*         msmc_heap_;  // on-chip memory
    clMallocMapping    clMalloc_mapping_;
};

}
