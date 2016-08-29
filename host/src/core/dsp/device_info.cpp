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
#include <cstring>
#include <cstdlib>
#ifndef _SYS_BIOS
#include <dirent.h>
#endif
#include <string>
#include <iostream>
#include <sstream>

#include "../tiocl_thread.h"
#ifdef _SYS_BIOS
#include <Singleton.h>
#else
#define  LOKI_PTHREAD_H
#include <loki/Singleton.h>
#endif

#include "../error_report.h"
#include "device_info.h"

using namespace tiocl;

#ifndef _SYS_BIOS
typedef Loki::SingletonHolder <tiocl::DeviceInfo, Loki::CreateUsingNew,
Loki::DefaultLifetime, Loki::ClassLevelLockable> SingleDeviceInfo;
#else
typedef Loki::SingletonHolder <tiocl::DeviceInfo, Loki::CreateUsingNew,
Loki::DefaultLifetime, Loki::SingleThreaded> SingleDeviceInfo;
#endif

/******************************************************************************
* Thread safe instance function for singleton behavior
******************************************************************************/
const DeviceInfo& DeviceInfo::Instance ()
{
    return SingleDeviceInfo::Instance();
}

DeviceInfo::DeviceInfo()
:symbol_lookup_(nullptr)
{
    num_devices_ = 1;

    #if !defined(_SYS_BIOS)
    symbol_lookup_ = CreateSymbolAddressLookup(FullyQualifiedPathToDspMonitor());
    #endif

    ComputeUnitsAvailable();

}

DeviceInfo::~DeviceInfo()
{
    delete symbol_lookup_;
}

extern "C" {
const char* ti_opencl_getComputeUnitList();
}

// Device and OS specific approach to determining the number of compute units
// available to the OpenCL runtime.
void DeviceInfo::ComputeUnitsAvailable()
{
    const char* comp_unit = nullptr;

    #if !defined (_SYS_BIOS)
    // TI_OCL_COMPUTE_UNIT_LIST is set to a comma separated list of
    // compute units available. E.g. "0,1" => cores 0 and 1 are available
    comp_unit = getenv("TI_OCL_COMPUTE_UNIT_LIST");
    #else
    // OpenCL over RTOS, get compute unit list from RTSC configuration file
    comp_unit = ti_opencl_getComputeUnitList();
    #endif

    if (!comp_unit)
    {
        #if defined(_SYS_BIOS)
            num_compute_units_ = 0;
            ReportError(ErrorType::Fatal, ErrorKind::NumComputeUnitDetectionFailed);
        #elif defined (DSPC868X)
            num_compute_units_ = 8;
        #elif defined (DEVICE_AM57)
            num_compute_units_ = 0;
            DIR *dir = opendir("/proc/device-tree/ocp");
            if (!dir)
                ReportError(ErrorType::Fatal, ErrorKind::NumComputeUnitDetectionFailed);

            while (dirent * entry = readdir(dir))
            {
                if (entry->d_name[0] && entry->d_name[0] == 'd' &&
                    entry->d_name[1] && entry->d_name[1] == 's' &&
                    entry->d_name[2] && entry->d_name[2] == 'p' &&
                    entry->d_name[3] && entry->d_name[3] == '@')
                    ++num_compute_units_;
            }

            closedir(dir);
        #else
            num_compute_units_ = 0;
            DIR *dir = opendir("/dev");
            if (!dir)
                ReportError(ErrorType::Fatal, ErrorKind::NumComputeUnitDetectionFailed);

            while (dirent * entry = readdir(dir))
            {
                if (entry->d_name[0] && entry->d_name[0] == 'd' &&
                    entry->d_name[1] && entry->d_name[1] == 's' &&
                    entry->d_name[2] && entry->d_name[2] == 'p' &&
                    entry->d_name[3] && isdigit(entry->d_name[3]))
                    ++num_compute_units_;
            }

            closedir(dir);
        #endif

        for (int x = 0; x < num_compute_units_; x++)
            available_compute_units_.insert(x);
    }
    else
    {
        // Parse the comma separated string to determine the compute units available
        const std::string cu = comp_unit;
        std::stringstream ss(cu);
        num_compute_units_ = 0;

        int i;
        while (ss >> i)
        {
            available_compute_units_.insert(i);
            num_compute_units_++;

            if (ss.peek() == ',')
                ss.ignore();
        }
    }
}

#include <sys/stat.h>
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
        ReportError(ErrorType::Fatal, ErrorKind::TiOclInstallNotSpecified);
    #endif

    struct stat st;
    stat(stdpath.c_str(), &st);
    if (S_ISDIR(st.st_mode))
        return stdpath;

    ReportError(ErrorType::Fatal, ErrorKind::DSPMonitorPathNonExistent,
                stdpath.c_str());

    // Will not get here, ReportError will abort
    return "";
}

std::string DeviceInfo::FullyQualifiedPathToDspMonitor() const
{
    return get_ocl_dsp() + "/dsp.out";
}

uint8_t DeviceInfo::GetComputeUnitsPerDevice(int device) const
{
    return num_compute_units_;
}



/******************************************************************************
* Driver::get_symbol
******************************************************************************/
DSPDevicePtr DeviceInfo::GetSymbolAddress(const std::string &name) const
{
    return symbol_lookup_->GetAddress(name);
}
