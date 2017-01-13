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
#ifndef CMEM_ALLOCATOR_H_
#define CMEM_ALLOCATOR_H_

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

extern "C"
{
#include "ti/cmem.h"
}


/******************************************************************************
* class CmemAllocator 
*    This class will hold underlying CMEM memory allocations for the duration
*    of the lifetime of an object with this type.
*
*    This class is intented to be a singleton, but is not enforced through 
*    implementation due to it's simple usage in the daemon and an obvious 
*    error condition at runtime if the singleton nature is not kept.
******************************************************************************/
class CmemAllocator 
{
public:

    CmemAllocator() :
            ddr_size_(0),
            ddr_alloc_dsp_addr_(0),
            ddr_host_addr_(nullptr),
            msmc_size_(0),
            msmc_alloc_dsp_addr_(0),
            msmc_host_addr_(nullptr)
    {
        int32_t status = CMEM_init();
        assert(status != -1);

        int32_t num_blocks;
        CMEM_getNumBlocks(&num_blocks);
        assert(num_blocks > 0 && num_blocks <= 3);

        CMEM_BlockAttrs pattrs0 = {0, 0};
        CMEM_getBlockAttrs(0, &pattrs0);

        uint64_t ddr_addr  = pattrs0.phys_base;
                 ddr_size_ = pattrs0.size;

        CMEM_AllocParams params = CMEM_DEFAULTPARAMS;
        params.flags            = CMEM_CACHED;
        params.type             = CMEM_POOL;

        ddr_alloc_dsp_addr_ = CMEM_allocPoolPhys2(0, 0, &params);
        assert(ddr_alloc_dsp_addr_ == ddr_addr);

        ddr_host_addr_ = CMEM_map(ddr_alloc_dsp_addr_, ddr_size_);
        assert (nullptr != ddr_host_addr_);

        if  (num_blocks == 1) return;

        CMEM_getBlockAttrs(1, &pattrs0);

        uint64_t msmc_addr  = pattrs0.phys_base;
                 msmc_size_ = pattrs0.size;

        params.type          = CMEM_HEAP;
        msmc_alloc_dsp_addr_ = CMEM_allocPhys2(1, msmc_size_, &params);
        assert(msmc_alloc_dsp_addr_ == msmc_addr);

        msmc_host_addr_  = CMEM_map(msmc_alloc_dsp_addr_, msmc_size_);
        assert (nullptr != msmc_host_addr_);
    }
                
    ~CmemAllocator() 
    {
        if (ddr_host_addr_ != nullptr)
        {
            CMEM_unmap(ddr_host_addr_, ddr_size_);

            CMEM_AllocParams params = CMEM_DEFAULTPARAMS;
            params.flags = CMEM_CACHED;

            CMEM_freePhys(ddr_alloc_dsp_addr_, &params);
        }

        if (msmc_host_addr_ != nullptr)
        {
            CMEM_unmap(msmc_host_addr_, msmc_size_);

            CMEM_AllocParams params = CMEM_DEFAULTPARAMS;
            params.flags = CMEM_CACHED;

            CMEM_freePhys(msmc_alloc_dsp_addr_, &params);
        }

        CMEM_exit();
    }

private:
    uint64_t ddr_size_;
    uint64_t ddr_alloc_dsp_addr_;
    void*    ddr_host_addr_;

    uint64_t msmc_size_;
    uint64_t msmc_alloc_dsp_addr_;
    void*    msmc_host_addr_;

    /*-------------------------------------------------------------------------
    * Prevent copy construction or assignment
    *------------------------------------------------------------------------*/
    CmemAllocator            (const CmemAllocator &) = delete;
    CmemAllocator& operator= (const CmemAllocator &) = delete;
};

#endif // CMEM_ALLOCATOR_H_
