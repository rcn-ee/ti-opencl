#include <cstddef>
#include <algorithm>
#include "shared_memory_provider.h"
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

using namespace tiocl;

template <class MIPolicy, class RWPolicy, class HeapPolicy>
SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
SharedMemoryProvider(uint8_t device_id) : 
    device_id_(device_id)
{
    // Discover available shared memory memory_ranges_
    MIPolicy::DiscoverMemoryRanges(memory_ranges_);

    for (const auto &r : memory_ranges_)
    {
        // Create a memory provider for that range
        mp_factory_.CreateMemoryProvider(r);

        // Configure a heap for the range
        if (r.GetLocation() == MemoryRange::Location::ONCHIP)
        {
            HeapPolicy::msmc_heap_->configure(r.GetBase(), r.GetSize(), 
                                              MIN_BLOCK_SIZE);
            ulm_put_mem(ULM_MEM_IN_DATA_ONLY, r.GetSize() >> 16, 1.0f);
        }
        else if (r.GetLocation() == MemoryRange::Location::OFFCHIP)
        {
            if (r.GetKind() == MemoryRange::Kind::CMEM_PERSISTENT ||
                r.GetKind() == MemoryRange::Kind::RTOS_SHMEM)
            {
                HeapPolicy::ddr_heap1_->configure(r.GetBase(), r.GetSize(), 
                                                  MIN_BLOCK_SIZE);
                ulm_put_mem(ULM_MEM_EX_CODE_AND_DATA, r.GetSize() >> 16, 1.0f);
            }
            else if (r.GetKind() == MemoryRange::Kind::CMEM_ONDEMAND)
            {
                HeapPolicy::ddr_heap2_->configure(r.GetBase(), r.GetSize(), 
                                                  MIN_CMEM_ONDEMAND_BLOCK_SIZE);
                HeapPolicy::ddr_heap2_->set_additional_alignment_requirements
                                        (MAX_CMEM_MAP_ALIGN);

                ulm_put_mem(ULM_MEM_EX_DATA_ONLY, r.GetSize() >> 16, 1.0f);
            }
            else
            {
                // DO NOT manage this memory range in heaps, e.g. RTOS_HOSTMEM
            }
        }
    }

    RWPolicy::Configure(device_id_, &mp_factory_);
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
~SharedMemoryProvider()
{
    // Free any allocated memory_ranges_
    mp_factory_.DestroyMemoryProviders();

    // Perform any cleanup required
    MIPolicy::Destroy();
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
uint64_t SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
AllocateMSMC(size_t size)
{
    uint64_t ret = HeapPolicy::msmc_heap_->malloc(size,true);
    if (ret) dsptop_msmc();
    ReportTrace("AllocateMSMC (0x%llx, %d)\n", ret, size);
    return ret;
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
void SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
FreeMSMC(uint64_t addr)
{
    ReportTrace("FreeMSMC(0x%llx)\n", addr);
    HeapPolicy::msmc_heap_->free(addr);
    dsptop_msmc();
}

// TODO: examine the flag, the logic, etc
#define FRACTION_PERSISTENT_FOR_BUFFER  8
template <class MIPolicy, class RWPolicy, class HeapPolicy>
uint64_t SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
AllocateGlobal(size_t size, bool prefer_32bit)
{
    if (prefer_32bit)
    {
        uint64_t ret = HeapPolicy::ddr_heap1_->malloc(size, true);
        if (ret) dsptop_ddr_fixed();
        ReportTrace("AllocateGlobal (0x%llx, %d, %d)\n", ret, size, prefer_32bit);
        return ret;
    }

    uint64_t addr = 0;
    uint64_t size64 = 0;
    uint32_t block_size = 0;

    HeapPolicy::ddr_heap1_->max_block_size(size64, block_size);

    if (size64 / size > FRACTION_PERSISTENT_FOR_BUFFER)
    {
        addr = HeapPolicy::ddr_heap1_->malloc(size, true);
        if (addr) dsptop_ddr_fixed();
    }
    if (!addr)
    {
        addr = HeapPolicy::ddr_heap2_->malloc(size, true);
        if (addr) dsptop_ddr_extended();
    }
    if (!addr)
    {
        addr = HeapPolicy::ddr_heap1_->malloc(size, true); // another chance
        if (addr) dsptop_ddr_fixed();
    }

    ReportTrace("AllocateGlobal (0x%llx, %d, %d)\n", addr, size, prefer_32bit);
    return addr;
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
void SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
FreeGlobal(uint64_t addr)
{
    ReportTrace("FreeGlobal(0x%llx)\n", addr);
    if (HeapPolicy::ddr_heap1_->contains(addr))
    {
        HeapPolicy::ddr_heap1_->free(addr);
        dsptop_ddr_fixed();
    }
    else if (HeapPolicy::ddr_heap2_->contains(addr))
    {
        HeapPolicy::ddr_heap2_->free(addr);
        dsptop_ddr_extended();
    }
    else
        ReportError(ErrorType::Fatal, ErrorKind::InvalidPointerToFree,
                    addr, "FreeGlobal");
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
void SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
FreeMSMCorGlobal(uint64_t addr)
{
    ReportTrace("FreeMSMCorGlobal(0x%llx)\n", addr);
    if (HeapPolicy::msmc_heap_->contains(addr))
        FreeMSMC(addr);
    else if (HeapPolicy::ddr_heap1_->contains(addr) ||
             HeapPolicy::ddr_heap2_->contains(addr) )
        FreeGlobal(addr);
    else
        ReportError(ErrorType::Fatal, ErrorKind::InvalidPointerToFree,
                    addr, "FreeMSMCorGlobal");
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
void SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
dsptop_msmc()
{
    uint64_t k64block_size = HeapPolicy::msmc_heap_->size() >> 16;
    float    pctfree  =  HeapPolicy::msmc_heap_->available();
             pctfree /=  HeapPolicy::msmc_heap_->size();

    ulm_put_mem(ULM_MEM_IN_DATA_ONLY, k64block_size, pctfree);
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
void SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
dsptop_ddr_fixed()
{
    uint64_t k64block_size = HeapPolicy::ddr_heap1_->size() >> 16;
    float    pctfree  =  HeapPolicy::ddr_heap1_->available();
             pctfree /=  HeapPolicy::ddr_heap1_->size();

    ulm_put_mem(ULM_MEM_EX_CODE_AND_DATA, k64block_size, pctfree);
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
void SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
dsptop_ddr_extended()
{
    uint64_t ext_size = HeapPolicy::ddr_heap2_->size();
    uint64_t k64block_size = ext_size >> 16;
    float    pctfree  =  HeapPolicy::ddr_heap2_->available();

    if (ext_size != 0) pctfree /= ext_size;
    else               pctfree  = 0.0f;

    ulm_put_mem(ULM_MEM_EX_DATA_ONLY, k64block_size, pctfree);
}


template <class MIPolicy, class RWPolicy, class HeapPolicy>
void * SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
clMalloc(size_t size, MemoryRange::Location l)
{
    uint64_t phys_addr = 0;
    void          *host_addr = NULL;
    bool           use_msmc  = (l == MemoryRange::Location::ONCHIP);

    if (use_msmc)  phys_addr = AllocateMSMC(size);
    else           phys_addr = AllocateGlobal(size, false);

    if (phys_addr != 0)
    {
        host_addr = RWPolicy::Map(phys_addr, size, false, true);
        if (host_addr)
        {
            Lock lock(this);
            clMalloc_mapping_[host_addr] = PhysAddrSizeFlagsTriple(
                           PhysAddrSizePair(phys_addr, size), l);
        }
        else
        {
            if (use_msmc) FreeMSMC(phys_addr);
            else          FreeGlobal(phys_addr);
        }
    }
    return host_addr;
}

typedef clMallocMapping::iterator clMallocMapping_iter;

template <class MIPolicy, class RWPolicy, class HeapPolicy>
void SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
clFree(void* ptr)
{
    Lock lock(this);
    clMallocMapping::iterator it = clMalloc_mapping_.find(ptr);
    if (it != clMalloc_mapping_.end())
    {
        PhysAddrSizeFlagsTriple phys_a_s_f = (*it).second;
        uint64_t phys_addr = phys_a_s_f.first.first;
        size_t         size      = phys_a_s_f.first.second;
        MemoryRange::Location flags     = phys_a_s_f.second;
        clMalloc_mapping_.erase(it);
        RWPolicy::Unmap(ptr, phys_addr, size, false);
        if (flags == MemoryRange::Location::ONCHIP)  FreeMSMC(phys_addr);
        else                                         FreeGlobal(phys_addr);
    }
    else
        ReportError(ErrorType::Warning, ErrorKind::InvalidPointerToClFree, ptr);
}

// Support query of host ptr in the middle of a clMalloced region
template <class MIPolicy, class RWPolicy, class HeapPolicy>
bool SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
clMallocQuery(void* ptr, uint64_t* p_addr, size_t* p_size)
{
    Lock lock(this);
    if (clMalloc_mapping_.empty())  return false;

    clMallocMapping::iterator it = clMalloc_mapping_.upper_bound(ptr);
    if (it == clMalloc_mapping_.begin())  return false;

    // map has bidirectional iterator, so --it is defined, even at end()
    PhysAddrSizeFlagsTriple phys_a_s_f = (*--it).second;
    // (ptr >= (*it).first) must hold because of upper_bound() call
    if (ptr >= (char*)(*it).first + phys_a_s_f.first.second)  return false;

    ptrdiff_t offset = ((char *) ptr) - ((char *) (*it).first);
    if (p_addr)  *p_addr = phys_a_s_f.first.first + offset;
    if (p_size)  *p_size = phys_a_s_f.first.second - offset;
    return true;
}


template <class MIPolicy, class RWPolicy, class HeapPolicy>
bool SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
CacheInv(uint64_t addr, void *host_addr, size_t sz)
{
    const MemoryProvider* region = mp_factory_.GetMemoryProvider(addr);
    return region->CacheInv(host_addr, sz);
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
bool SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
CacheWb(uint64_t addr, void *host_addr, size_t sz)
{
    const MemoryProvider* region = mp_factory_.GetMemoryProvider(addr);
    return region->CacheWb(host_addr, sz);
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
bool SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
CacheWbInv(uint64_t addr, void *host_addr, size_t sz)
{
    const MemoryProvider* region = mp_factory_.GetMemoryProvider(addr);
    return region->CacheWbInv(host_addr, sz);
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
bool SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
CacheWbInvAll()
{
    return RWPolicy::CacheWbInvAll();
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
uint64_t SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
HeapBase(MemoryRange::Kind k, MemoryRange::Location l)
{
    switch (k)
    {
        case MemoryRange::Kind::CMEM_PERSISTENT:
        {
            if (l == MemoryRange::Location::ONCHIP)
                return HeapPolicy::msmc_heap_->base();
            else
                return HeapPolicy::ddr_heap1_->base();
        }

        case MemoryRange::Kind::CMEM_ONDEMAND:
            return HeapPolicy::ddr_heap2_->base();

        default:
            return 0;
    }
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
uint64_t SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
HeapSize(MemoryRange::Kind k, MemoryRange::Location l)
{
    switch (k)
    {
        case MemoryRange::Kind::CMEM_PERSISTENT:
        {
            if (l == MemoryRange::Location::ONCHIP)
                return HeapPolicy::msmc_heap_->size();
            else
                return HeapPolicy::ddr_heap1_->size();
        }

        case MemoryRange::Kind::CMEM_ONDEMAND:
            return HeapPolicy::ddr_heap2_->size();

        default:
            return 0;
    }
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
uint64_t SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
HeapMaxAllocSize(MemoryRange::Kind k, MemoryRange::Location l)
{
    uint64_t size = 0;
    uint32_t not_used = 0;

    switch (k)
    {
        case MemoryRange::Kind::CMEM_PERSISTENT:
        {
            if (l == MemoryRange::Location::ONCHIP)
                HeapPolicy::msmc_heap_->max_block_size(size, not_used);
            else
                HeapPolicy::ddr_heap1_->max_block_size(size, not_used);

            return size;
        }

        case MemoryRange::Kind::CMEM_ONDEMAND:
        {
            HeapPolicy::ddr_heap2_->max_block_size(size, not_used);
            return size;
        }

        default:
            return 0;
    }
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
int32_t SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
WriteToShmem(uint64_t dst, uint8_t *src, size_t sz)
{
    return RWPolicy::Write(dst, src, sz);
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
int32_t SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
ReadFromShmem (uint64_t src, uint8_t *dst, size_t sz)
{
    return RWPolicy::Read(src, dst, sz);
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
void * SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
Map(uint64_t addr, size_t sz, bool is_read, bool allow_fail)
{
    return RWPolicy::Map(addr, sz, is_read, allow_fail);
}

template <class MIPolicy, class RWPolicy, class HeapPolicy>
int32_t SharedMemoryProvider<MIPolicy, RWPolicy, HeapPolicy>::
Unmap(void *host_addr, uint64_t buf_addr, size_t sz, bool is_write)
{
    return RWPolicy::Unmap(host_addr, buf_addr, sz, is_write);
}

