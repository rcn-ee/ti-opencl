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
#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <bfd.h>

#if !defined (DEVICE_AM57)
extern "C"
{
   #include "mpmclient.h"
};
#endif

#include "../error_report.h"

using namespace tiocl;

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

std::string Driver::dsp_monitor(int dsp)
{
    std::string get_ocl_dsp();
    return get_ocl_dsp() + "/dsp.out";
}

int Driver::cores_per_dsp(int dsp)
{
#if defined(DEVICE_AM57)
    return 2;
#else
    static int n = 0;

    if(n == 0)
    {
        DIR *dir = opendir("/dev");
        if(!dir)
            ERR(1, "failed to open /dev\n");

        while(dirent *entry = readdir(dir))
        {
            if(entry->d_name[0] && entry->d_name[0] == 'd' &&
               entry->d_name[1] && entry->d_name[1] == 's' &&
               entry->d_name[2] && entry->d_name[2] == 'p' &&
               entry->d_name[3] && isdigit(entry->d_name[3]))
                ++n;
        }

        closedir(dir);
    }

    return n;
#endif
}

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

void Driver::reset_and_load(int chip)
{
#if !defined (DEVICE_AM57)
    int ret;
    int error_code = 0;
    int error_code_msg[50];
    char curr_core[10];

    std::string monitor = dsp_monitor(chip);
    int n_cores = cores_per_dsp(chip);

    for (int core=0; core < n_cores; core++)
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
    for (int core=0; core < n_cores; core++)
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
    for (int core=0; core < n_cores; core++)
    {
        snprintf(curr_core, 5,"dsp%d", core);
        ret = mpm_run(curr_core, &error_code);
        if ( ret < 0)
            printf("run failed, core %d (retval: %d, error: %d)\n",
                   core, ret, error_code);
        ERR(ret, "DSP run failed");

        report_core_state(curr_core);
    }

#endif
    return;
}

/******************************************************************************
* Driver::open
******************************************************************************/
int32_t Driver::open()
{
    pNum_dsps = 1;
    return 0;
}

/******************************************************************************
* Driver::close()
******************************************************************************/
int32_t Driver::close()
{
    return 0;
}



/******************************************************************************
* Driver::create_image_handle
******************************************************************************/
void* Driver::create_image_handle(int chip)
{
    std::string monitor = dsp_monitor(chip);

    bfd *dsp_bfd = bfd_openr(monitor.c_str(), NULL);

    if(dsp_bfd == NULL)
    {
      printf("\nERROR:driver: %s Error Open image %s\n",
             bfd_errmsg(bfd_get_error()), monitor.c_str());
      exit(-1);
    }

    char** matching;
    char *ptr;
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
