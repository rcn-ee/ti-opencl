/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include "shmem.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <ti/cmem.h>

#define REPORT(x) printf(x "\n")
#define ERR(status, msg) if (status) { printf("ERROR: %s\n", msg); exit(-1); }

/******************************************************************************
* shmem::shmem
******************************************************************************/
shmem::shmem()
 : p_dsp_addr(0), p_size(0), p_page_size(sysconf(_SC_PAGE_SIZE)), p_mmap_fd(-1)
   , p_mpm_transport_handle(NULL), p_threshold(32<<20) 

{ }

/******************************************************************************
* shmem::~shmem
******************************************************************************/
shmem::~shmem()
{
    if (p_mmap_fd != -1) close(p_mmap_fd);
}

/******************************************************************************
* shmem::configure
******************************************************************************/
void shmem::configure_base(DSPDevicePtr64 dsp_addr,  uint64_t size)
{
    /*-------------------------------------------------------------------------
    * If the sysconf for the page size failed
    *------------------------------------------------------------------------*/
    if (p_page_size <= 0) { REPORT("Failed to get PAGE_SIZE"); return; }

    // p_mmap_fd = open("/dev/mem", (O_RDWR | O_SYNC));
    // Now we use mpm_transport_{open, mmap, munmap, close}
    /*-------------------------------------------------------------------------
    * core1-core7's l2 go through /dev/dsp{1-7}
    * everything else (core0's l2, msmc, global addr) go through /dev/dsp0
    *------------------------------------------------------------------------*/
    char devname[16];
    strcpy(devname, "dsp0");
    if (0x11800000 <= dsp_addr & dsp_addr < 0x17900000)
        devname[3] = ((dsp_addr >> 24) - 0x10) + '0';
    mpm_transport_open_t mpm_transport_open_cfg;
    mpm_transport_open_cfg.open_mode = (O_SYNC|O_RDWR);
    p_mpm_transport_handle = mpm_transport_open(devname,
                                                &mpm_transport_open_cfg);

    /*-------------------------------------------------------------------------
    * If the open failed
    *------------------------------------------------------------------------*/
    // if (p_mmap_fd == -1) { REPORT("Failed to open /dev/mem"); return; }
    if (p_mpm_transport_handle == NULL)
    { 
        printf("Failed to open /dev/%s", devname);
        return;
    }

    p_dsp_addr = dsp_addr;
    p_size     = size;
}


/******************************************************************************
* shmem_persistent::shmem
******************************************************************************/
#define MULTIPLE_OF_POW2(x, y) (((x) & ((y)-1)) != 0 ? false : true)

shmem_persistent::shmem_persistent()
     : p_host_addr(0), p_xlate_dsp_to_host_offset(0)
{ }

/******************************************************************************
* shmem_persistent::configure
******************************************************************************/
void shmem_persistent::configure(DSPDevicePtr64 dsp_addr, uint64_t size) 
{
    configure_base(dsp_addr, size);

    /*-------------------------------------------------------------------------
    * if base class failed to construct, because /dev/mem could not be opened
    *------------------------------------------------------------------------*/
    // if (p_mmap_fd == -1) return;
    if (p_mpm_transport_handle == NULL) return;

    if (!MULTIPLE_OF_POW2(dsp_addr, p_page_size)) 
    {
        REPORT("Mapped region addr is not a multiple of page size");
        return;
    }

    if (!MULTIPLE_OF_POW2(size,     p_page_size)) 
    {
        REPORT("Mapped region size is not a multiple of page size");
        return;
    }

    //p_host_addr = mmap(0, size, (PROT_READ|PROT_WRITE), MAP_SHARED, p_mmap_fd,
    //                     (off_t)dsp_addr);
    mpm_transport_mmap_t mpm_transport_mmap_cfg;
    mpm_transport_mmap_cfg.mmap_prot = (PROT_READ|PROT_WRITE);
    mpm_transport_mmap_cfg.mmap_flags = MAP_SHARED;

    p_host_addr = (void *)mpm_transport_mmap(p_mpm_transport_handle,
                                                dsp_addr, size,
                                                &mpm_transport_mmap_cfg);

    // if (p_host_addr == MAP_FAILED) 
    if (p_host_addr == (void *) -1)
    { 
        REPORT("Failed to mmap");
        p_host_addr = 0; 
        return; 
    }

    p_xlate_dsp_to_host_offset = (void*)((int64_t)p_host_addr - dsp_addr);
}

/******************************************************************************
* shmem_persistent::~shmem_persistent
******************************************************************************/
shmem_persistent::~shmem_persistent()
{
    // if (p_host_addr) munmap(p_host_addr, p_size);
    if (p_host_addr)
        mpm_transport_munmap(p_mpm_transport_handle, p_host_addr, p_size);
}

/******************************************************************************
* shmem_persistent::map
******************************************************************************/
void *shmem_persistent::map(DSPDevicePtr64 dsp_addr, uint32_t size, bool is_read)
{
    if (!p_host_addr) return 0;

    if (dsp_addr >= p_dsp_addr && dsp_addr + size <= p_dsp_addr + p_size)
         return dsp_addr + (char*)p_xlate_dsp_to_host_offset;
    else 
    {
        REPORT("Attempting to map a region outside a defined area");
        return 0;
    }
}

/******************************************************************************
* shmem_persistent::unmap
******************************************************************************/
void  shmem_persistent::unmap(void* host_addr, uint32_t size, bool is_write)
{ 
    // if (host_addr) msync(host_addr, size, MS_SYNC);
}



/******************************************************************************
* shmem_ondemand::shmem_ondemap
******************************************************************************/
shmem_ondemand::shmem_ondemand()
{ }

/******************************************************************************
* shmem::~shmem
******************************************************************************/
shmem_ondemand::~shmem_ondemand()
{
}

/******************************************************************************
* shmem_ondemand::configure
******************************************************************************/
void shmem_ondemand::configure(DSPDevicePtr64 dsp_addr, uint64_t size) 
{ 
    configure_base(dsp_addr, size);
}


/******************************************************************************
* shmem_ondemand::map
******************************************************************************/
void *shmem_ondemand::map(DSPDevicePtr64 dsp_addr, uint32_t size, bool is_read)
{
    if (!MULTIPLE_OF_POW2(dsp_addr, p_page_size))
    {
        REPORT("Mapped region addr is not a multiple of page size");
        return 0;
    }

    if (!MULTIPLE_OF_POW2(size,     p_page_size))
    {
        REPORT("Mapped region addr is not a multiple of page size");
        return 0;
    }
    
    if (dsp_addr < p_dsp_addr || dsp_addr + size > p_dsp_addr + p_size)
    {
        REPORT("Attempting to map a region outside a defined area");
        return 0;
    }

    //void *host_addr = mmap(0, size, (PROT_READ|PROT_WRITE), MAP_SHARED, 
    //                          p_mmap_fd, (off_t)dsp_addr);
    mpm_transport_mmap_t mpm_transport_mmap_cfg;
    mpm_transport_mmap_cfg.mmap_prot = (PROT_READ|PROT_WRITE);
    mpm_transport_mmap_cfg.mmap_flags = MAP_SHARED;

    void * host_addr = mpm_transport_mmap(p_mpm_transport_handle,
                                          dsp_addr, size,
                                          &mpm_transport_mmap_cfg);

    // if (host_addr == MAP_FAILED)
    if (host_addr == (void *) -1)
    { 
        REPORT("Failed to mmap");
        return 0; 
    }

    return host_addr;
}

/******************************************************************************
* shmem_ondemand::unmap
******************************************************************************/
void  shmem_ondemand::unmap(void* host_addr, uint32_t size, bool is_write)
{ 
    // if (host_addr) munmap(host_addr, size);
}


/******************************************************************************
* shmem_cmem:: cmem cache operations
******************************************************************************/
#define CMEM_SUCCESS 0
bool shmem_cmem::cacheInv(void *host_addr, uint32_t size)
{
#if (CMEM_VERSION > 0x04000000)
    if (size >= p_threshold)
        return CMEM_cacheWbInvAll() == CMEM_SUCCESS;
    else
#endif
        return CMEM_cacheInv(host_addr, size) == CMEM_SUCCESS;
}

bool shmem_cmem::cacheWb(void *host_addr, uint32_t size)
{
#if (CMEM_VERSION > 0x04000000)
    if (size >= p_threshold)
        return CMEM_cacheWbInvAll() == CMEM_SUCCESS;
    else
#endif
        return CMEM_cacheWb(host_addr, size) == CMEM_SUCCESS;
}

bool shmem_cmem::cacheWbInv(void *host_addr, uint32_t size)
{
#if (CMEM_VERSION > 0x04000000)
    if (size >= p_threshold)
        return CMEM_cacheWbInvAll() == CMEM_SUCCESS;
    else
#endif
        return CMEM_cacheWbInv(host_addr, size) == CMEM_SUCCESS;
}

bool shmem_cmem::cacheWbInvAll()
{
#if (CMEM_VERSION > 0x04000000)
    return CMEM_cacheWbInvAll() == CMEM_SUCCESS;
#endif
    return false;
}

/******************************************************************************
* shmem_cmem::cmem_init
* TODO: remove addr3, size3 once uboot is updated, so that we don't have
*       have fragemented CMEM blocks for DDR
******************************************************************************/
void shmem_cmem::cmem_init(DSPDevicePtr64 *addr1, uint64_t *size1,
                           DSPDevicePtr   *addr2, uint32_t *size2,
                           DSPDevicePtr64 *addr3, uint64_t *size3)
{
#if defined(DEVICE_AM57)
    const char *cmem_command = "For available CMEM DDR block size: ~512MB:\n"
        "modprobe cmemk "
        "phys_start=0xa0000000 phys_end=0xc0000000 pools=1x536870912 "
        "allowOverlap=1";
    const char *cmem_command2 = "";
#define CMEM_MIN_BLOCKS (1)
#else
    const char *cmem_command = "For available CMEM DDR block size: ~1.5GB:\n"
        "modprobe cmemk_1-5_GB";
    const char *cmem_command2 = "For available CMEM DDR block size: ~6.5GB:\n"
        "modprobe cmemk_6-5_GB";
#define CMEM_MIN_BLOCKS (2)
#endif

    /*-------------------------------------------------------------------------
    * First initialize the CMEM module
    *------------------------------------------------------------------------*/
    if (CMEM_init() == -1) 
    {
        printf("\nThe cmemk kernel module does not appear to installed.\n\n"
               "Commands such as the following run as root would "
               "install cmemk and allow OpenCL to proceed properly.\n\n");
        printf("%s\n\n", cmem_command);
        printf("%s\n\n", cmem_command2);
        exit(-1);
    }

    /*-------------------------------------------------------------------------
    * Debug to see in cmem init was correct
    * Valid CMEM configurations: last block for MPI (hyperlink/SRIO) buffers
    *     DDR1(OCL), MSMC2(OCL), DDR3(MPI)
    * or  DDR1(OCL), MSMC2(OCL), DDR3(OCL), DDR4(MPI)
    *------------------------------------------------------------------------*/
    int num_Blocks = 0;
    CMEM_getNumBlocks(&num_Blocks);
    if (num_Blocks < 2)
    {
        printf("\nOpenCL needs at least two CMEM blocks to operate properly.\n"
               "One for DDR, the other for MSMC.  Example commands:\n");
        printf("%s\n\n", cmem_command);
        printf("%s\n\n", cmem_command2);
        exit(-1);
    }

    CMEM_BlockAttrs pattrs0 = {0, 0}; 
    CMEM_BlockAttrs pattrs1 = {0, 0};
    CMEM_BlockAttrs pattrs2 = {0, 0};

    CMEM_getBlockAttrs(0, &pattrs0);
    CMEM_getBlockAttrs(1, &pattrs1);

    /*-------------------------------------------------------------------------
    * Return 36-bit addr, and up to 7.5G memory size
    *------------------------------------------------------------------------*/
    *addr1 = (DSPDevicePtr64) pattrs0.phys_base;
    *size1 = (uint64_t) pattrs0.size;

    // Persistent CMEM should start within 0x8:2200_0000 - 0x8:4000_0000
    if (*addr1 >= MPAX_USER_MAPPED_DSP_ADDR)
    { 
        printf("Unable to allocate OCL persistent CMem from 0x%llx\n",
               pattrs0.phys_base);
        exit(EXIT_FAILURE);
    }

    *addr2 = pattrs1.phys_base;
    *size2 = pattrs1.size;
    if (*addr2 < MSMC_OCL_START_ADDR || *addr2 >= MSMC_OCL_END_ADDR)
    { 
        printf("Unable to allocate OCL MSMC memory from 0x%llx\n",
               pattrs1.phys_base);
        exit(EXIT_FAILURE);
    }

    /*-------------------------------------------------------------------------
    * Grab all available CMEM physical address, to be managed by OCL
    *------------------------------------------------------------------------*/
    DSPDevicePtr64 alloc_dsp_addr = 0;
    CMEM_AllocParams params = CMEM_DEFAULTPARAMS;
    params.flags            = CMEM_CACHED;
    params.type             = CMEM_POOL;
    alloc_dsp_addr          = CMEM_allocPoolPhys2(0, 0, &params);
    if (!alloc_dsp_addr || alloc_dsp_addr != *addr1)
    { 
        printf("Failed to allocate 0x%llx from CMem 0, allocated=0x%llx\n",
               *size1, alloc_dsp_addr);
        exit(EXIT_FAILURE);
    }

    params.type    = CMEM_HEAP;
    alloc_dsp_addr = CMEM_allocPhys2(1, *size2, &params);
    if (!alloc_dsp_addr || alloc_dsp_addr != *addr2)
    { 
        printf("Failed to allocate 0x%x from CMem 1, allocated=0x%llx\n",
               *size2, alloc_dsp_addr);
        exit(EXIT_FAILURE);
    }

    if (num_Blocks > 3)  // DDR1(OCL), MSMC2(OCL), DDR3(OCL), DDR4(MPI)
    {
        CMEM_getBlockAttrs(2, &pattrs2);
        *addr3 = pattrs2.phys_base;
        *size3 = pattrs2.size;
        params.type    = CMEM_POOL;
        alloc_dsp_addr = CMEM_allocPoolPhys2(2, 0, &params);
        if (!alloc_dsp_addr || alloc_dsp_addr != *addr3)
        { 
            printf("Failed to allocate 0x%llx from CMem 2, allocated=0x%llx\n",
                   *size3, alloc_dsp_addr);
            exit(EXIT_FAILURE);
        }
    }
    else                 // DDR1(OCL), MSMC2(OCL), DDR3(MPI)
    {
        *addr3 = 0;
        *size3 = 0;
    }
}

/******************************************************************************
* shmem_cmem::cmem_exit
******************************************************************************/
void shmem_cmem::cmem_exit()
{
    /* Finalize the CMEM module */
    if (CMEM_exit() == -1) ERR(1, "Failed to finalize CMEM");
}


/******************************************************************************
* shmem_cmem_persistent::shmem
******************************************************************************/
shmem_cmem_persistent::shmem_cmem_persistent(int cmem_block)
     : p_host_addr(0), p_xlate_dsp_to_host_offset(0), p_cmem_block(cmem_block)
{ }

/******************************************************************************
* shmem_cmem_persistent::configure
******************************************************************************/
void shmem_cmem_persistent::configure(DSPDevicePtr64 dsp_addr, uint64_t size) 
{
    p_dsp_addr = dsp_addr;
    p_size     = size;
    DSPDevicePtr64 cmem_addr = p_dsp_addr;
    if (p_dsp_addr >= 0xA0000000 && p_dsp_addr < 0xFFFFFFFF)
        cmem_addr = p_dsp_addr - 0xA0000000 + 0x820000000ULL;
    p_host_addr = CMEM_map(cmem_addr, size);
    if (! p_host_addr) 
        ERR(1, "Cannot map CMEM physical memory into the Host virtual address space.\n"
               "       This is typically due to Linux system memory being near capacity.");
    p_xlate_dsp_to_host_offset = (int64_t)p_host_addr - dsp_addr;
}

/******************************************************************************
* shmem_cmem_persistent::~shmem_cmem_persistent
******************************************************************************/
shmem_cmem_persistent::~shmem_cmem_persistent()
{
    if (p_dsp_addr == 0) return;

    if (p_host_addr != NULL) CMEM_unmap(p_host_addr, p_size);
    CMEM_AllocParams params = CMEM_DEFAULTPARAMS;
    params.flags = CMEM_CACHED;
    DSPDevicePtr64 cmem_addr = p_dsp_addr;
    if (p_dsp_addr > 0xA0000000 && p_dsp_addr < 0xFFFFFFFF)
        cmem_addr = p_dsp_addr - 0xA0000000 + 0x820000000ULL;
    CMEM_freePhys(cmem_addr, &params);
}

/******************************************************************************
* shmem_cmem_persistent::map: dsp_addr (phys) -> host_addr (virt)
******************************************************************************/
void *shmem_cmem_persistent::map(DSPDevicePtr64 dsp_addr, uint32_t size, bool is_read)
{
    if (!p_host_addr || 
        dsp_addr < p_dsp_addr || dsp_addr + size > p_dsp_addr + p_size)
    {
        ERR(1, "Attempting to cmem_map a region outside a defined area");
        return NULL;
    }

    void *host_addr = dsp_addr + (char*)p_xlate_dsp_to_host_offset;
    if (is_read)  cacheInv(host_addr, size);
    return host_addr;
}

/******************************************************************************
* shmem_cmem_persistent::unmap: flush host side writes
******************************************************************************/
void  shmem_cmem_persistent::unmap(void* host_addr, uint32_t size, bool is_write)
{ 
    if (host_addr && is_write)  cacheWb(host_addr, size);
}


/******************************************************************************
* shmem_cmem_ondeman::configure
******************************************************************************/
void shmem_cmem_ondemand::configure(DSPDevicePtr64 dsp_addr, uint64_t size) 
{
    p_dsp_addr = dsp_addr;
    p_size = size;
}

/******************************************************************************
* shmem_cmem_ondemand::map: dsp_addr (phys) -> host_addr (virt)
******************************************************************************/
void *shmem_cmem_ondemand::map(DSPDevicePtr64 dsp_addr, uint32_t size, bool is_read)
{
    int          align_offset = ((int) dsp_addr) & (MIN_CMEM_MAP_ALIGN - 1);
    DSPDevicePtr64 align_addr = dsp_addr - align_offset;
    uint32_t       align_size = size + align_offset;
    void     *align_host_addr = CMEM_map(align_addr, align_size);
    if (! align_host_addr)  return NULL;
    void           *host_addr = (char *) align_host_addr + align_offset;

    if (is_read)  cacheInv(host_addr, size);
    return host_addr;
}

/******************************************************************************
* shmem_cmem_persistent::unmap: flush host side writes
******************************************************************************/
void  shmem_cmem_ondemand::unmap(void* host_addr, uint32_t size, bool is_write)
{ 
    if (host_addr && is_write)  cacheWb(host_addr, size);

    if (host_addr)
    {
        int      align_offset = ((int) host_addr) & (MIN_CMEM_MAP_ALIGN-1);
        void *align_host_addr = (char *) host_addr - align_offset;
        uint32_t   align_size = size + align_offset;
        CMEM_unmap(align_host_addr, align_size);
    }
}

/******************************************************************************
* shmem_cmem_ondemand::malloc: allocate CMEM physical address
* 64-bit size: could be allocating a buffer, then accessing smaller subbuffers
******************************************************************************/
DSPDevicePtr64 shmem_cmem_ondemand::cmem_malloc(uint64_t size)
{
    CMEM_AllocParams params = CMEM_DEFAULTPARAMS;
    params.flags = CMEM_CACHED;
    params.type = CMEM_HEAP;
    DSPDevicePtr64 addr = CMEM_allocPhys2(0, size, &params);
    if (!addr)
    { 
        printf("Failed to allocate space 0x%llx from CMem\n", size);
        exit(EXIT_FAILURE);
    }
    return addr;
}

/******************************************************************************
* shmem_cmem_ondemand::free: free allocated CMEM physical address
******************************************************************************/
void shmem_cmem_ondemand::cmem_free(DSPDevicePtr64 addr)
{
    CMEM_AllocParams params = CMEM_DEFAULTPARAMS;
    params.flags = CMEM_CACHED;
    params.type = CMEM_HEAP;
    CMEM_freePhys(addr, &params);
}

