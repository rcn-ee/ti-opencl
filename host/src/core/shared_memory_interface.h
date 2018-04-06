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
#include <cstdlib>
#include <cstdint>
#include <map>
#include "memory_range.h"

#pragma once

namespace tiocl {

/***************************************************************************
 * Interface to manage shared memory between the host and device
 * Includes methods to:
 * - Manage dynamic memory allocation in the shared memory
 * - Cache operations
 * - Read/Write methods
 * - Map/Unmap methods to map shared memory into host virtual address space
 *
 * A Coal::Device object has a pointer to a shared memory interface. It's
 * possible for more than one device to use the same SharedMemory interface
 * - in this scenario, the 2 devices will share the same heap.
 * Implementations of the SharedMemory interface are created by the
 * SharedMemoryProviderFactory.
 ****************************************************************************/
class SharedMemory
{
public:
    virtual ~SharedMemory() {};

    // Allocate/Free on chip shared memory
    virtual uint64_t  AllocateMSMC(size_t size) =0;
    virtual void      FreeMSMC(uint64_t addr) =0;

    // Allocate/Free off chip shared memory
    virtual uint64_t  AllocateGlobal(size_t size, bool prefer_32bit) =0;
    virtual void      FreeGlobal(uint64_t addr) =0;

    // Determine which memory the pointer was allocated from and free it
    virtual void      FreeMSMCorGlobal(uint64_t addr)  =0;

    // Allocate/Free functions with ability to query whether an address
    // has been allocated by the clMalloc method
    virtual void* clMalloc     (size_t size, MemoryRange::Location l) =0;
    virtual void  clFree       (void* ptr) =0;
    virtual bool  clMallocQuery(void* ptr, uint64_t* p_addr, size_t* p_size) =0;

    // Host side cache operations
    virtual bool CacheInv(uint64_t addr, void *host_addr, size_t sz) =0;
    virtual bool CacheWb(uint64_t addr, void *host_addr, size_t sz) =0;
    virtual bool CacheWbInv(uint64_t addr, void *host_addr, size_t sz) =0;
    virtual bool CacheWbInvAll() = 0;

    // Methods to query heap base, heap size and maximum allocation size
    virtual uint64_t HeapBase(MemoryRange::Kind k, MemoryRange::Location l) =0;
    virtual uint64_t HeapSize(MemoryRange::Kind k, MemoryRange::Location l) =0;
    virtual uint64_t HeapMaxAllocSize(MemoryRange::Kind k, MemoryRange::Location l)=0;

    // Read and Write shared memory
    virtual int32_t ReadFromShmem (uint64_t src, uint8_t *dst, size_t sz) = 0;
    virtual int32_t WriteToShmem(uint64_t dst, uint8_t *src, size_t sz) = 0;

    // Map/Unmap to/from virtual address space
    virtual void*   Map(uint64_t addr, size_t sz, bool is_read = false,
                        bool allow_fail = false) = 0;
    virtual int32_t Unmap(void *host_addr, uint64_t buf_addr,
                          size_t sz, bool is_write = false) = 0;
};

/****************************************************************************
 * Creates the appropriate shared memory provider for a given device
 * and OS combination.
 * Note: The device_id is currently used to select the appropriate
 * device on the PCIe bus for the read/write methods (e.g. one of the C6678s
 * in the quad-C6678 PCIe card.
 ****************************************************************************/
class SharedMemoryProviderFactory
{
public:
    SharedMemory* CreateSharedMemoryProvider(uint8_t device_id);
    void DestroySharedMemoryProviders();

    SharedMemory* GetSharedMemoryProvider(uint8_t device_id) const;

    SharedMemoryProviderFactory() =default;
    ~SharedMemoryProviderFactory() =default;

    SharedMemoryProviderFactory(const SharedMemoryProviderFactory&) =delete;
    SharedMemoryProviderFactory& operator=(const SharedMemoryProviderFactory&) =delete;

private:
    std::map<uint8_t, SharedMemory*> shmProviderMap;
};

}
