#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <iostream>
#include <iomanip>

extern "C"
{
#include "ti/cmem.h"
}

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

    uint64_t ddr_size_;
    uint64_t ddr_alloc_dsp_addr_;
    void*    ddr_host_addr_;

    uint64_t msmc_size_;
    uint64_t msmc_alloc_dsp_addr_;
    void*    msmc_host_addr_;
};


int main()
{
    CmemAllocator CT;

    daemon(0,0);

    while (1);

    return 0;
}
