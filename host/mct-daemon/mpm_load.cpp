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
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>

#include <string>
#include <set>

extern "C"
{
   #include "mpmclient.h"
};


/******************************************************************************
* get_ocl_dsp - retun the directory of the opencl DSP monitor binaries
******************************************************************************/
static std::string get_ocl_dsp()
{
    std::string stdpath("/usr/share/ti/opencl");

    const char *ocl_install    = getenv("TI_OCL_INSTALL");
    const char *target_rootdir = getenv("TARGET_ROOTDIR");

    if (ocl_install)         stdpath = ocl_install    + stdpath;
    else if (target_rootdir) stdpath = target_rootdir + stdpath;

    #if defined (DSPC868X)
    // DSPC868x requires TI_OCL_INSTALL to be specified
    if (ocl_install == nullptr)
    {
        syslog(LOG_ERR, "Set TI_OCL_INSTALL environment variable to location "
                        " of OpenCL installation. Refer User Guide for details");
        abort();
    }
    #endif

    struct stat st;
    stat(stdpath.c_str(), &st);
    if (S_ISDIR(st.st_mode))
        return stdpath;

    syslog(LOG_ERR, 
           "Internal Error: Path to DSP monitor binary (%s) does not exist",
           stdpath.c_str());
    abort();

    // Will not get here
    return "";
}

/******************************************************************************
* reset_dsps - reset all DSP's using MPM
******************************************************************************/
void reset_dsps(std::set<uint8_t>& dsps)
{
    syslog(LOG_INFO, "Resetting %d DSPs", dsps.size());

    /*-------------------------------------------------------------------------
    * On K2x devices, sleep to ensure mpm daemon has completed setup. systemd
    * unit dependencies between mpmd and ti-mctd do not work as expected
    * because the mpmd daemon forks before completing its setup.
    * mpm_ping() returns 0 if the mpm server is functional. If it does not
    * succeed after 8 attempts, abort.
    *------------------------------------------------------------------------*/
    int count = 0;
    while (mpm_ping() != 0 && count < 8)
    {
        usleep(5000000); // 0.5 seconds
        count++;
    }

    if (mpm_ping() != 0)
    {
        syslog(LOG_ERR, "Internal Error: Cannot ping mpm server");
        abort();
    }

    for (uint8_t core : dsps)
    {
        std::string curr_core("dsp" + std::to_string(core));
        int error_code = 0;
        int ret = mpm_reset(const_cast<char*>(curr_core.c_str()), 
                            &error_code);

        if (ret < 0)
        {
            syslog(LOG_ERR, 
                   "Internal Error: Failed to reset device, DSP core %d, error code %d",
                   core, error_code);
            abort();
        }
    }
}

/******************************************************************************
* load_dsps - load monitor binaries on all DSP's using MPM
******************************************************************************/
void load_dsps(std::set<uint8_t>& dsps)
{
    syslog(LOG_INFO, "Loading %d DSPs", dsps.size());
    std::string monitor_dir(get_ocl_dsp() + "/");
    for (uint8_t core : dsps)
    {
        std::string curr_core("dsp" + std::to_string(core));
        std::string dspbin   (monitor_dir + curr_core + ".out");

        int error_code = 0;
        int ret = mpm_load(const_cast<char*>(curr_core.c_str()), 
                           const_cast<char*>(dspbin.c_str()),
                           &error_code);
        if (ret < 0)
        {
            syslog(LOG_ERR, 
                   "Internal Error: Failed to load monitor %s on DSP core %d, error code %d",
                   dspbin.c_str(), core, error_code);
            abort();
        }
    }
}

/******************************************************************************
* run_dsps - start all DSP's running
******************************************************************************/
void run_dsps(std::set<uint8_t>& dsps)
{
    syslog(LOG_INFO, "Running %d DSPs", dsps.size());
    for (uint8_t core : dsps)
    {
        std::string curr_core  ("dsp" + std::to_string(core));

        int error_code = 0;
        int ret = mpm_run(const_cast<char*>(curr_core.c_str()), &error_code);
        if (ret < 0)
        {
            syslog(LOG_ERR, 
                   "Internal Error: Failed to run monitor on DSP core %d, error code %d",
                   core, error_code);
            abort();
        }
    }
}
