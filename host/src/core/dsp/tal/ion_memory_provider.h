/******************************************************************************
 *
 * Copyright (c) 2019, Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 *****************************************************************************/
#include <cstdlib>
#include <cstdint>
#include <memory>
#include <mutex>

#include "core/shared_memory_interface.h"
#include "../../tiocl_types.h"
#include "ion_allocator.h"

#pragma once

namespace tiocl {

/****************************************************************************
 * Implementation of the SharedMemory interface
 * MIPolicy: Memory Initialization policy, controls how available memory
 *           ranges are discovered
 * RWPolicy: Handles variations in Read/Write and Map/Unmap functions
 ***************************************************************************/
class SharedMemoryProvider : public SharedMemory
{
public:
    SharedMemoryProvider(uint8_t device_id);
    ~SharedMemoryProvider();

    uint64_t AllocateMSMC (size_t   size) override;
    void     FreeMSMC     (uint64_t addr) override;
    uint64_t AllocateGlobal(size_t   size, bool prefer_32bit) override;
    void     FreeGlobal  (uint64_t addr) override;
    void     FreeMSMCorGlobal  (uint64_t addr) override;

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

    uint64_t HeapBase(MemoryRange::Kind k, MemoryRange::Location l) override;
    uint64_t HeapSize(MemoryRange::Kind k, MemoryRange::Location l) override;
    uint64_t HeapMaxAllocSize(MemoryRange::Kind k, MemoryRange::Location l) override;

private:

    std::unique_ptr<IonAllocator>& FindAllocator(uint64_t addr);
    std::unique_ptr<IonAllocator>& FindAllocator(void* ptr);

    void dsptop_ddr_fixed();
    void dsptop_ddr_extended();
    void dsptop_msmc();

    uint8_t                      device_id_;
    std::unique_ptr<IonAllocator> ddr_heap_m;
    std::unique_ptr<IonAllocator> msmc_heap_m;
    std::mutex                    mutex_m;
};

}
