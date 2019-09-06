/******************************************************************************
 *
 * Copyright (c) 2019, Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 *****************************************************************************/

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/ion.h>
#include <linux/dma-buf.h>
#include <linux/dma_buf_phys.h>

#include "../error_report.h"
#include "ion_allocator.h"

using namespace tiocl;

IonAllocator::IonAllocator(int ion_heap_id) : heap_id_m(ion_heap_id)
{
    ion_fd_m = open("/dev/ion", O_RDONLY | O_CLOEXEC);
    if (ion_fd_m < 0)
        ReportError(ErrorType::Fatal, ErrorKind::IonOpenFailed,
                    heap_id_m, strerror(errno));
}

IonAllocator::~IonAllocator()
{
    if (close(ion_fd_m) < 0)
        ReportError(ErrorType::Fatal, ErrorKind::IonCloseFailed,
                    heap_id_m, strerror(errno));
}

void* IonAllocator::Allocate(size_t size)
{
    Info info = { 0, 0, 0, size};

	struct ion_allocation_data alloc = {
		.len          = size,
		.heap_id_mask = (uint32_t)(1 << heap_id_m),
		.flags        = ION_FLAG_CACHED,
	};

	if (ioctl(ion_fd_m, ION_IOC_ALLOC, &alloc) < 0)
    {
        ReportError(ErrorType::Warning, ErrorKind::IonAllocFailed,
                    heap_id_m, strerror(errno));
        return nullptr;
    }

    info.dmabuf_fd = alloc.fd;

    // Use dma-buf-phys for fd -> physical address

    // Each dmabuf is associated with a unique dma-buf-phys fd
    info.dmabuf_phys_fd = open("/dev/dma-buf-phys", O_RDONLY | O_CLOEXEC);
    if (info.dmabuf_phys_fd < 0)
    {
        // Cannot call ion_free() here because it required a dmabuf_phys_fd
        ReportError(ErrorType::Warning, ErrorKind::IonAllocFailed,
                    heap_id_m, strerror(errno));
        return nullptr;
    }

    // Get the physical address of the underlying memory
    struct dma_buf_phys_data data = {
        .fd = (uint32_t)info.dmabuf_fd,
    };

    if (ioctl(info.dmabuf_phys_fd, DMA_BUF_PHYS_IOC_CONVERT, &data) < 0)
    {
        ReportError(ErrorType::Warning, ErrorKind::IonAllocFailed,
                    heap_id_m, strerror(errno));
        (void)close(info.dmabuf_phys_fd);
        return nullptr;
    }

    info.physical_address = data.phys;

    // Map the dmabuf into the process virtual address space
    void* ptr = mmap(0,                       // Preferred start address
                     size,                    // Length to be mapped
                     PROT_WRITE | PROT_READ,  // Read and write access
                     MAP_SHARED,              // Shared memory
                     info.dmabuf_fd,          // File descriptor
                     0);                      // The byte offset from fd

    if (ptr == MAP_FAILED)
    {
        ReportError(ErrorType::Warning, ErrorKind::IonAllocFailed,
                    heap_id_m, strerror(errno));
        (void)close(info.dmabuf_phys_fd);
        return nullptr;
    }

    info.host_ptr = ptr;

    ReportTrace("ION Allocate(%d) -> %p, dma fd %d, phy addr %llx\n",
                size, ptr, info.dmabuf_fd, info.physical_address);

    mutex_m.lock();
    ptr_info_m.emplace(ptr, info);
    mutex_m.unlock();

    return ptr;
}

const IonAllocator::Info& IonAllocator::GetInfo(void *ptr) const
{
    std::lock_guard<std::mutex> lock(mutex_m);
    PtrInfo::const_iterator it = ptr_info_m.find(ptr);
    if (it == ptr_info_m.end())
        ReportError(ErrorType::Fatal, ErrorKind::IonPtrNotAlloced, ptr);

    return it->second;
}

bool IonAllocator::IsAlloced(void *ptr) const
{
    std::lock_guard<std::mutex> lock(mutex_m);
    if  (ptr_info_m.find(ptr) != ptr_info_m.end())
        return true;

    return false;
}

bool IonAllocator::IsAlloced_T(uint64_t addr) const
{
    void* ptr = PhyAddressToVirt_T(addr);
    if (!ptr) return false;

    return IsAlloced(ptr);
}

uint64_t IonAllocator::VirtToPhyAddress(void *ptr) const
{
    const Info& info = GetInfo(ptr);
    return info.physical_address;
}

void* IonAllocator::PhyAddressToVirt_T(uint64_t addr) const
{
    std::lock_guard<std::mutex> lock(mutex_m);
    for (const auto &i : ptr_info_m)
        if (i.second.physical_address == addr)
            return i.first;

    return nullptr;
}

void IonAllocator::Free(void* ptr)
{
    // Lookup the info stuct using the address
    mutex_m.lock();
    PtrInfo::const_iterator it = ptr_info_m.find(ptr);
    if (it == ptr_info_m.end())
    {
        mutex_m.unlock();
        ReportError(ErrorType::Fatal, ErrorKind::IonFreeFailed,
                    "find", heap_id_m, "pointer not allocated using ION");
    }

    const Info info = it->second;

    // Remove entry from map before ION free. This is to avoid a race where
    // another thread allocates the same pointer and adds it to the map.
    ptr_info_m.erase(it);
    mutex_m.unlock();

    ReportTrace("ION Free(%p, %d)\n", ptr, info.size);

    if (munmap(ptr, info.size) < 0)
        ReportError(ErrorType::Warning, ErrorKind::IonFreeFailed,
                    "munmap", heap_id_m, strerror(errno));

    if (close(info.dmabuf_phys_fd) < 0)
        ReportError(ErrorType::Warning, ErrorKind::IonFreeFailed,
                    "phys fd", heap_id_m, strerror(errno));

    if (close(info.dmabuf_fd) < 0)
        ReportError(ErrorType::Warning, ErrorKind::IonFreeFailed,
                    "dmabuf fd", heap_id_m, strerror(errno));

}

void IonAllocator::Free_T(uint64_t addr)
{
    void* ptr = PhyAddressToVirt_T(addr);
    assert (ptr != nullptr);

    Free(ptr);
}

bool IonAllocator::CacheWb(void* ptr) const
{
    const Info& info = GetInfo(ptr);
    ReportTrace("ION: CacheWb: %p, dma fd %d\n", ptr, info.dmabuf_fd);

    struct dma_buf_sync sync_end = {0};
    sync_end.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;

    if (ioctl(info.dmabuf_fd, DMA_BUF_IOCTL_SYNC, &sync_end) < 0)
    {
        ReportError(ErrorType::Warning, ErrorKind::IonCacheOpFailed,
                    "Wb", info.dmabuf_fd, heap_id_m, strerror(errno));
        return false;
    }

    return true;
}

bool IonAllocator::CacheInv(void* ptr) const
{
    const Info& info = GetInfo(ptr);
    ReportTrace("ION: CacheInv: %p, dma fd %d\n", ptr, info.dmabuf_fd);

    struct dma_buf_sync sync_start = {0};
    sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;

    if (ioctl(info.dmabuf_fd, DMA_BUF_IOCTL_SYNC, &sync_start) < 0)
    {
        ReportError(ErrorType::Warning, ErrorKind::IonCacheOpFailed,
                    "Inv", info.dmabuf_fd, heap_id_m, strerror(errno));
        return false;
    }

    return true;
}

#if 0
// IsAllocedInRange added to support SubBuffers. Only used by the Cache ops
// Note: We will need to stop supporting shared memory SubBuffers after the
// switch to ION. This is because ION does not allow cache operations
// on sub sections of allocated memory regions.
void* IonAllocator::IsAllocedInRange(void* ptr) const
{
    std::lock_guard<std::mutex> lock(mutex_m);
    for (const auto &i : ptr_info_m)
    {
        ptrdiff_t x = (char *)ptr - (char*)i.second.host_ptr;

        if ((ptr >= i.second.host_ptr) && (x < i.second.size))
            return i.first;
    }

    return nullptr;
}
#endif
