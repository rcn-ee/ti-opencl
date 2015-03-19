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
#include "driver.h"
#include <deque>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <string>
#include <bfd.h>

extern "C"
{
   #include "mpmclient.h"
};


#define ERR(status, msg) if (status) { printf("ERROR: %s\n", msg); exit(-1); }
#define BOOT_ENTRY_LOCATION_ADDR   0x87FFFC
#define BOOT_MAGIC_ADDR(core) (0x10000000 | (core << 24) | 0x87FFFC)

Driver* Driver::pInstance = 0;

/******************************************************************************
* Thread safe instance function for singleton behavior
******************************************************************************/
Driver* Driver::instance () 
{
    static Mutex Driver_instance_mutex;
    Driver* tmp = pInstance;

    __sync_synchronize();

    if (tmp == 0) 
    {
        ScopedLock lck(Driver_instance_mutex);

        tmp = pInstance;
        if (tmp == 0) 
        {
            tmp = new Driver;
            __sync_synchronize();
            pInstance = tmp;
        }
    }
    return tmp;
}

/******************************************************************************
* Convert pci data into a recognizable board name for a device
******************************************************************************/
const char *get_board(unsigned switch_device)
{
    switch (switch_device)
    {
        case 0x8624: return "dspc8681";
        case 0x8748: return "dspc8682";
        default    : ERR(1, "Unsupported device"); return "unknown";
    }
}

#define TOTAL_NUM_CORES_PER_CHIP 8

/******************************************************************************
* wait_for_ready
******************************************************************************/
bool Driver::wait_for_ready(int chip) { return true; }

static void report_core_state(const char *curr_core)
{
#if 0
    char state[50];
    int ret;
    mpm_slave_state_e core_state;

    ret = mpm_state(curr_core, &core_state);
    if ( ret < 0) 
        printf("state query failed, %s\n", curr_core);

    switch (core_state)
    {
        case mpm_slave_state_idle:    sprintf(state, "idle");      break;
        case mpm_slave_state_loaded:  sprintf(state, "loaded");    break;
        case mpm_slave_state_running: sprintf(state, "running");   break;
        case mpm_slave_state_crashed: sprintf(state, "crashed");   break;
        case mpm_slave_state_error:   sprintf(state, "in error");  break;

        default:                sprintf(state, "in undefined state"); break;
    }

    printf("DSP core state: %s is %s\n", curr_core, state);
#endif
}

void *Driver::reset_and_load(int chip)
{
    int ret;
    int error_code = 0;
    int error_code_msg[50];
    char curr_core[10];

    std::string get_ocl_dsp();
    std::string monitor = get_ocl_dsp() + "/dsp.out";

    for (int core=0; core< TOTAL_NUM_CORES_PER_CHIP; core++)
    {
        snprintf(curr_core, 5, "dsp%d", core);

        ret = mpm_reset(curr_core, &error_code);
        if ( ret < 0) 
            printf("reset failed, core %d (retval: %d, error: %d)\n",
                   core, ret, error_code);
// JKN Update ERR to handle error_code
        ERR (ret, "DSP out of reset failed");  

        report_core_state(curr_core);
    }

    /*-------------------------------------------------------------------------
    * Load monitor on the devices
    *------------------------------------------------------------------------*/
    for (int core=0; core< TOTAL_NUM_CORES_PER_CHIP; core++)
    {
        snprintf(curr_core, 5,"dsp%d", core);
        ret = mpm_load(curr_core, const_cast<char*>(monitor.c_str()), 
                       &error_code);
        if ( ret < 0) 
            printf("load failed, core %d (retval: %d, error: %d)\n",
                   core, ret, error_code);
        ERR(ret, "Download image failed");

       report_core_state(curr_core);
    }

    /*-------------------------------------------------------------------------
    * Run monitor on the devices
    *------------------------------------------------------------------------*/
    for (int core=0; core< TOTAL_NUM_CORES_PER_CHIP; core++)
    {
        snprintf(curr_core, 5,"dsp%d", core);
        ret = mpm_run(curr_core, &error_code);
        if ( ret < 0) 
            printf("run failed, core %d (retval: %d, error: %d)\n",
                   core, ret, error_code);
        ERR(ret, "DSP run failed");

        report_core_state(curr_core);
    }

    bfd *dsp_bfd = bfd_openr(monitor.c_str(), NULL);
    char** matching;
    char *ptr;

    if(dsp_bfd == NULL) 
    {
      printf("\nERROR:driver: %s Error Open image %s\n",  
             bfd_errmsg(bfd_get_error()), monitor.c_str()); 
      exit(-1); 
    }
    /* Check format with matching */	  
    if (!bfd_check_format_matches (dsp_bfd, bfd_object, &matching)) 
    {
        fprintf(stderr, "\nERROR:driver  %s: %s\n", monitor.c_str(), 
        bfd_errmsg(bfd_get_error()));  
        if (bfd_get_error () == bfd_error_file_ambiguously_recognized) 
        {
            for (ptr = *matching; ptr != NULL; ptr++) 
            {
                printf("%s: \n", ptr); 
                exit(-1); 
            }
            free (matching);
        }
    }

    return (void *)dsp_bfd;
}

/******************************************************************************
* Driver::open
******************************************************************************/
int32_t Driver::open()         
{ 
    Lock lock(this); 

    pNum_dsps = 1;

    return 0;
}

/******************************************************************************
* Driver::close()         
******************************************************************************/
int32_t Driver::close()         
{
    Lock lock(this); 

    while (!pShmem_areas.empty()) delete pShmem_areas.back(), pShmem_areas.pop_back();

    cmem_exit();
    return 0;
}

void Driver::cmem_init(DSPDevicePtr64 *addr1, uint64_t *size1,
                       DSPDevicePtr   *addr2, uint32_t *size2,
                       DSPDevicePtr64 *addr3, uint64_t *size3)
{
    shmem_cmem::cmem_init(addr1, size1, addr2, size2, addr3, size3);
}

void Driver::cmem_exit()
{
    shmem_cmem::cmem_exit();
}

DSPDevicePtr64 Driver::cmem_ondemand_malloc(uint64_t size)
{
    return shmem_cmem_ondemand::cmem_malloc(size);
}

void Driver::cmem_ondemand_free(DSPDevicePtr64 addr)
{
    shmem_cmem_ondemand::cmem_free(addr);
}

/******************************************************************************
* Driver::split_ddr_heap: partition DDR to persistent mapping part (heap1)
*                                       and on demand mapping part (heap2)
******************************************************************************/
void Driver::split_ddr_memory(DSPDevicePtr64  addr,  uint64_t  size,
                              DSPDevicePtr64& addr1, uint64_t& size1,
                              DSPDevicePtr64& addr2, uint64_t& size2,
                                                     uint64_t& size3)
{
    addr1 = addr;
    size1 = size;
    addr2 = 0;
    size2 = 0;


    // split ddr memory 1 into two chunks
    if (getenv("TI_OCL_DSP_NOMAP") != NULL)
    {
        size3 = 0;
    }
    else if (addr + size > ALL_PERSISTENT_MAX_DSP_ADDR ||
             (size3 > 0 && addr + size > MPAX_USER_MAPPED_DSP_ADDR))
    {
        size2 = addr + size - MPAX_USER_MAPPED_DSP_ADDR;
        size1 = size - size2;
        addr2 = addr + size1;
    }

    // translate first chunk to using 32-bit aliased physical addresses
    if (addr > DSP_36BIT_ADDR)
    {
        addr1 = addr + 0xA0000000 - 0x820000000ULL;
        /*---------------------------------------------------------------------
        * if the ddr size is greater than we can currently support, limit it
        *--------------------------------------------------------------------*/
        //const int ddr_size_limit = (1.5 * 1024*1024*1024) - (48 *1024*1024);
        const uint64_t ddr_size_limit = ALL_PERSISTENT_MAX_DSP_ADDR - addr;
        if (size1 > ddr_size_limit)
            size1 = ddr_size_limit;
    }
}

void Driver::shmem_configure(DSPDevicePtr64 addr, uint64_t size, int cmem_block)
{
    if (size <= 0) return;

    shmem *area;
    if (addr >= MPAX_USER_MAPPED_DSP_ADDR)
        area = new shmem_cmem_ondemand();
    else if (cmem_block >= 0)
        area = new shmem_cmem_persistent(cmem_block);
    else
        area = new shmem_persistent();

    area->configure(addr, size);
    pShmem_areas.push_back(area);
}

/******************************************************************************
* Driver::get_memory_region
******************************************************************************/
shmem* Driver::get_memory_region(DSPDevicePtr64 addr)
{

    for (int i = 0; i < pShmem_areas.size(); ++i)
    {
        uint64_t end_exclusive = (uint64_t)pShmem_areas[i]->start() +
                                           pShmem_areas[i]->size();

        if (addr >= pShmem_areas[i]->start() && addr <  end_exclusive)
            return pShmem_areas[i];
    }

    printf("Illegal memory region: addr = 0x%llx\n", addr);
    exit(-1);
}


/******************************************************************************
* Driver::write
******************************************************************************/
int32_t Driver::write(int32_t dsp_id, DSPDevicePtr64 addr, uint8_t *buf,
                      uint32_t size)
{ 
    int core;
    /*-------------------------------------------------------------------------
    * if the write is to L2, then write for each core
    *------------------------------------------------------------------------*/
    if ((addr >> 20) == 0x008)
        for (core=0; core< TOTAL_NUM_CORES_PER_CHIP; core++)
            write_core(dsp_id, ((0x10 + core) << 24) + addr, buf, size);
    else write_core(dsp_id, addr, buf, size);
}

/******************************************************************************
* Driver::write_core
******************************************************************************/
int32_t Driver::write_core(int32_t dsp_id, DSPDevicePtr64 addr, uint8_t *buf,
                      uint32_t size)
{ 
    Lock lock(this); 

    shmem* region = get_memory_region(addr);
    void* dst_host_addr = region->map(addr, size, false);
    if (dst_host_addr) memcpy((char*)dst_host_addr, buf, size);
    else ERR(1, "Unable to map dsp addr for write");
    region->unmap(dst_host_addr, size, true);

    return 0;
}

void*   Driver::map(Coal::DSPDevice *device, DSPDevicePtr64 addr, uint32_t sz,
                    bool is_read, bool allow_fail)
{
    Lock lock(this);
    shmem* region = get_memory_region(addr);
    void* host_addr = region->map(addr, sz, is_read);
    if (host_addr == NULL && !allow_fail) ERR(1, "Unable to map a dsp address");
    return host_addr;
}

int32_t Driver::unmap(Coal::DSPDevice *device, void *host_addr,
                      DSPDevicePtr64 buf_addr, uint32_t sz, bool is_write)
{
    Lock lock(this);
    shmem* region = get_memory_region(buf_addr);
    region->unmap(host_addr, sz, is_write);
    return 0;
}

bool Driver::cacheInv(DSPDevicePtr64 addr, void *host_addr, uint32_t sz)
{
    Lock lock(this);
    shmem* region = get_memory_region(addr);
    return region->cacheInv(host_addr, sz);
}

bool Driver::cacheWb(DSPDevicePtr64 addr, void *host_addr, uint32_t sz)
{
    Lock lock(this);
    shmem* region = get_memory_region(addr);
    return region->cacheWb(host_addr, sz);
}

bool Driver::cacheWbInv(DSPDevicePtr64 addr, void *host_addr, uint32_t sz)
{
    Lock lock(this);
    shmem* region = get_memory_region(addr);
    return region->cacheWbInv(host_addr, sz);
}

bool Driver::cacheWbInvAll()
{
    Lock lock(this);
    return shmem_cmem::cacheWbInvAll();
}


/******************************************************************************
* Driver::read
******************************************************************************/
int32_t Driver::read(int32_t dsp_id, DSPDevicePtr64 addr, uint8_t *buf, 
                     uint32_t size)
{ 
    Lock lock(this); 

    shmem* region = get_memory_region(addr);
    void* dst_host_addr = region->map(addr, size, true);
    if (dst_host_addr) memcpy(buf, (char*)dst_host_addr, size);
    else ERR(1, "Unable to map dsp addr for read");
    region->unmap(dst_host_addr, size, false);

    return 0;
}

/******************************************************************************
* Driver::free_image_handle
******************************************************************************/
void Driver::free_image_handle(void *handle) 
{ 
    bfd_close((bfd*)handle); 
}

/******************************************************************************
* Driver::get_symbol
******************************************************************************/
DSPDevicePtr Driver::get_symbol(void* image_handle, const char *name)
{
    DSPDevicePtr addr;
    bfd*         dsp_bfd;
    uint32_t     nsyms, nsize;
    asymbol **   symtab;
    symbol_info  syminfo;
    int          i;

    if (!image_handle)
    {
       std::cout << "ERROR: Failed to get image handle"  << std::endl;
       exit(-1);
    }

    dsp_bfd = (bfd *)image_handle;

    /*-------------------------------------------------------------------------
    * Find boot address and address of mpi_rank.
    *------------------------------------------------------------------------*/
    nsize = bfd_get_symtab_upper_bound (dsp_bfd);
    if ((symtab = (asymbol**)malloc(nsize)) == NULL) 
    {
       std::cout << "ERROR: Failed to malloc memory in get_symbol"  << std::endl;
       exit(-1);
    }

    nsyms = bfd_canonicalize_symtab(dsp_bfd, symtab);

    for (i = 0; i < nsyms; i++)
        if (strcmp(symtab[i]->name, name) == 0)
        {
            bfd_symbol_info(symtab[i], &syminfo);
            DSPDevicePtr addr = syminfo.value;
            free(symtab);

            return addr;
        }

    free(symtab);
    std::cout << "ERROR: Get symbol failed" << std::endl;
    exit(-1); 
}
