/******************************************************************************
 *
 * Copyright (c) 2019, Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 *****************************************************************************/

#pragma once

#include <mutex>
#include <map>

class IonAllocator
{
    public:

        struct Info
        {
            uint64_t physical_address;
            int      dmabuf_fd;
            int      dmabuf_phys_fd;
            size_t   size;
            void*    host_ptr;
        };

        IonAllocator(int ion_heap_id);
        ~IonAllocator();

        void* Allocate(size_t size);
        void  Free(void *ptr);

        // Returns true if ptr was Allocate'd via ION and has not been Free'd
        bool IsAlloced(void* ptr) const;

        // Returns the information associated with the pointer. Will report an
        // error if the ptr was not Allocate'd via ION
        const Info& GetInfo(void *ptr) const;

        bool CacheWb(void *ptr) const;
        bool CacheInv(void *ptr) const;

        uint64_t VirtToPhyAddress(void *ptr) const;

        // These methods are temporarily added to ease the CMEM->ION transition
        // The OpenCL runtime must minimize use and tracking of physical
        // addresses and switch to ION fds.
        void  Free_T(uint64_t addr);
        bool  IsAlloced_T(uint64_t addr) const;
        void* PhyAddressToVirt_T(uint64_t addr) const;

    private:

        typedef std::map<void*, Info> PtrInfo;

        PtrInfo    ptr_info_m;
        int        heap_id_m;
        int        ion_fd_m;
        mutable std::mutex mutex_m;
};
