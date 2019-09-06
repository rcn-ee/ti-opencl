/******************************************************************************
 *
 * Copyright (c) 2019, Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 *****************************************************************************/

#include <cstddef>
#include <algorithm>
#include <assert.h>
#include <cstring>
#include "ion_memory_provider.h"
#include "../error_report.h"
#include "dspmem.h"

/*-----------------------------------------------------------------------------
* Add ULM memory state messages if ULM library is available
*----------------------------------------------------------------------------*/
#if defined (ULM_ENABLED)
extern "C" {
#include "tiulm.h"
}
#else
#define ulm_put_mem(a,b,c)
#define ulm_config()
#define ulm_term()
#endif

/* ION APIs do not support queries for base address and size. The base address
 * and size is used to program the EVE MMU. The size is also used for OpenCL
 * API global memory size queries
 * Need a better approach than hard-coding the address ranges in this file.
 */
#define ION_DDR_MEM_BLOCK_BASE   (0xa00000000)
#define ION_DDR_MEM_BLOCK_SIZE   (0x00c000000)
#define ION_MSMC_MEM_BLOCK_BASE  (0x040500000)
#define ION_MSMC_MEM_BLOCK_SIZE  (0x000100000)

using namespace tiocl;

SharedMemoryProvider::SharedMemoryProvider(uint8_t device_id) :
    device_id_(device_id)
{
    const int DDR_HEAP_ID = 2;
    const int MSMC_HEAP_ID = 3;

    std::lock_guard<std::mutex> lock(mutex_m);
    ddr_heap_m = std::unique_ptr<IonAllocator>(new IonAllocator(DDR_HEAP_ID));

    // Using DDR_HEAP_ID, on-chip ION allocation broken
    msmc_heap_m
        = std::unique_ptr<IonAllocator>(new IonAllocator(DDR_HEAP_ID));

}

SharedMemoryProvider::~SharedMemoryProvider() { }

uint64_t SharedMemoryProvider::AllocateMSMC(size_t size)
{
    void *ptr = msmc_heap_m->Allocate(size);
    uint64_t ret = msmc_heap_m->VirtToPhyAddress(ptr);

    ReportTrace("AllocateMSMC (0x%llx, %d)\n", ret, size);
    return ret;
}

void SharedMemoryProvider::FreeMSMC(uint64_t addr)
{
    ReportTrace("FreeMSMC(0x%llx)\n", addr);
    msmc_heap_m->Free_T(addr);
}

uint64_t SharedMemoryProvider::AllocateGlobal(size_t size, bool prefer_32bit)
{
    void *ptr = ddr_heap_m->Allocate(size);
    uint64_t addr = ddr_heap_m->VirtToPhyAddress(ptr);
    ReportTrace("AllocateGlobal (0x%llx, %d, %d)\n", addr, size, prefer_32bit);
    return addr;
}

void SharedMemoryProvider::FreeGlobal(uint64_t addr)
{
    ReportTrace("FreeGlobal(0x%llx)\n", addr);
    ddr_heap_m->Free_T(addr);
}

std::unique_ptr<IonAllocator>&
SharedMemoryProvider::FindAllocator(uint64_t addr)
{
    if (msmc_heap_m->IsAlloced_T(addr))     return msmc_heap_m;
    else if (ddr_heap_m->IsAlloced_T(addr)) return ddr_heap_m;
    else assert(0 && "Cannot find allocator!");
}

std::unique_ptr<IonAllocator>&
SharedMemoryProvider::FindAllocator(void* ptr)
{
    if (msmc_heap_m->IsAlloced(ptr))
        return msmc_heap_m;
    else if (ddr_heap_m->IsAlloced(ptr))
        return ddr_heap_m;
    else
        ReportError(ErrorType::Fatal, ErrorKind::IonPtrNotAlloced, ptr);
}

void SharedMemoryProvider::FreeMSMCorGlobal(uint64_t addr)
{
    ReportTrace("FreeMSMCorGlobal(0x%llx)\n", addr);
    FindAllocator(addr)->Free_T(addr);
}


void* SharedMemoryProvider::clMalloc(size_t size, MemoryRange::Location l)
{
    return ddr_heap_m->Allocate(size);
}

void SharedMemoryProvider::clFree(void* ptr)
{
    FindAllocator(ptr)->Free(ptr);
}

bool SharedMemoryProvider::
clMallocQuery(void* ptr, uint64_t* p_addr, size_t* p_size)
{
    if (msmc_heap_m->IsAlloced(ptr) || ddr_heap_m->IsAlloced(ptr))
    {
        const IonAllocator::Info& info = FindAllocator(ptr)->GetInfo(ptr);
        if (p_addr) *p_addr = info.physical_address;
        if (p_size) *p_size = info.size;
        return true;
    }

    return false;
}

bool SharedMemoryProvider::
CacheInv(uint64_t addr, void *host_addr, size_t sz)
{
    return FindAllocator(host_addr)->CacheInv(host_addr);
}

bool SharedMemoryProvider::
CacheWb(uint64_t addr, void *host_addr, size_t sz)
{
    return FindAllocator(host_addr)->CacheWb(host_addr);
}

bool SharedMemoryProvider::
CacheWbInv(uint64_t addr, void *host_addr, size_t sz)
{
    bool status = CacheWb(addr, host_addr, sz);
    status &= CacheInv(addr, host_addr, sz);

    return status;
}

bool SharedMemoryProvider::CacheWbInvAll()
{
    ReportError(ErrorType::Fatal, ErrorKind::InfoMessage,
                "ION does not support CacheWbInvAll");
    return false;
}

uint64_t SharedMemoryProvider::
HeapBase(MemoryRange::Kind k, MemoryRange::Location l)
{
    if (l == MemoryRange::Location::ONCHIP) return ION_MSMC_MEM_BLOCK_BASE;
    else                                    return ION_DDR_MEM_BLOCK_BASE;
}

uint64_t SharedMemoryProvider::
HeapSize(MemoryRange::Kind k, MemoryRange::Location l)
{
    if (l == MemoryRange::Location::ONCHIP) return ION_MSMC_MEM_BLOCK_SIZE;
    else                                    return ION_DDR_MEM_BLOCK_SIZE;
}

/* This method is used by CL_DEVICE_GLOBAL_MEM_MAX_ALLOC_TI and it's variants
 * This is a TI extension and is not used in any of the examples. It is not
 * documented in the User's Guide either.
 * Since we cannot support a dynamic MaxAllocSize with ION, the best option is
 * to remove the CL_DEVICE_GLOBAL_MEM_MAX_ALLOC_TI queries.
 */
uint64_t SharedMemoryProvider::
HeapMaxAllocSize(MemoryRange::Kind k, MemoryRange::Location l)
{
    return 0;
}

int32_t
SharedMemoryProvider::WriteToShmem(uint64_t dst, uint8_t *src, size_t sz)
{
    void* dst_ptr = FindAllocator(dst)->PhyAddressToVirt_T(dst);

    if (!dst_ptr)
        ReportError(ErrorType::Fatal, ErrorKind::InfoMessage,
              "SharedMemoryProvider::WriteToShmem - invalid physical address");

    std::memcpy(dst_ptr, src, sz);

    // Writeback host cache (before a device read)
    FindAllocator(dst_ptr)->CacheWb(dst_ptr);
    return 0;
}

int32_t
SharedMemoryProvider::ReadFromShmem (uint64_t src, uint8_t *dst, size_t sz)
{
    void* src_ptr = FindAllocator(src)->PhyAddressToVirt_T(src);

    if (!src_ptr)
        ReportError(ErrorType::Fatal, ErrorKind::InfoMessage,
            "SharedMemoryProvider::ReaadFromShmem - invalid physical address");

    // Invalidate host cache, device may have written memory
    FindAllocator(src_ptr)->CacheInv(src_ptr);
    std::memcpy(dst, src_ptr, sz);
    return 0;
}

void*
SharedMemoryProvider::Map(uint64_t addr, size_t sz, bool is_read, bool allow_fail)
{
    void* ptr = FindAllocator(addr)->PhyAddressToVirt_T(addr);
    if (is_read) FindAllocator(ptr)->CacheInv(ptr);

    return ptr;
}

int32_t SharedMemoryProvider::
Unmap(void *host_addr, uint64_t buf_addr, size_t sz, bool is_write)
{
    if (is_write) FindAllocator(host_addr)->CacheWb(host_addr);
    return 0;
}

#if 0
bool SharedMemoryProvider::
CacheInv(uint64_t addr, void *host_addr, size_t sz)
{
    void *ptr;

    if ((ptr = ddr_heap_m->IsAllocedInRange(host_addr)) != nullptr)
        return ddr_heap_m->CacheInv(ptr);

    if ((ptr = msmc_heap_m->IsAllocedInRange(host_addr)) != nullptr)
        return msmc_heap_m->CacheInv(ptr);

    return false;
}


bool SharedMemoryProvider::
CacheWb(uint64_t addr, void *host_addr, size_t sz)
{
    void *ptr;

    if ((ptr = ddr_heap_m->IsAllocedInRange(host_addr)) != nullptr)
        return ddr_heap_m->CacheWb(ptr);

    if ((ptr = msmc_heap_m->IsAllocedInRange(host_addr)) != nullptr)
        return msmc_heap_m->CacheWb(ptr);

    return false;
}
#endif
