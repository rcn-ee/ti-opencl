/******************************************************************************
 * Copyright (c) 2012-2015, Texas Instruments Incorporated - http://www.ti.com/
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
#include <list>
#include <iostream>
#include <stdlib.h>

#include "CL/cl.h"
#include "CL/cl_ext.h"
#include "platform.h"
#include "propertylist.h"
#include "object.h"
#include "cpu/device.h"
#include "dsp/device.h"
#include "dsp/driver.h"

/*-----------------------------------------------------------------------------
* For the lock file
*----------------------------------------------------------------------------*/
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

using namespace Coal;

// Ensure that Class Platform remains mutable to the ICD "POD" C structure, as
// expected by the ICD loader
static_assert(std::is_standard_layout<Platform>::value,
              "Class Platform must be of C++ standard layout type.");

/******************************************************************************
* begin_file_lock_crit_section
******************************************************************************/
static int begin_file_lock_crit_section(const char* fname)
{
    /*---------------------------------------------------------------------
    * Create a lock, so only 1 OpenCL program can progress at a time.
    * I'm not sure about the appropriateness of putting this in the ctor.
    * We may look at delayed ctor of platform with this in it.
    *--------------------------------------------------------------------*/
    int lock_fd = open(fname, O_CREAT, 
                     S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

    std::string str_fname(fname);

    if (lock_fd < 0) 
    {
        std::cout << "Can not open lock file " << str_fname << ", Aborting !" << std::endl;
        exit(-1);
    }

    int res = flock(lock_fd, LOCK_EX|LOCK_NB);
    if (res == -1)
    {
       if (errno == EWOULDBLOCK)
       {
           std::cout << "Waiting on lock " << str_fname << " ..." << std::endl;
           res = flock(lock_fd, LOCK_EX);
           if (res == -1)
           {
               std::cout << "Error Locking file " << str_fname << ", Aborting !" << std::endl;
               exit(-1);
           }
           else std::cout << "Acquired lock " << str_fname << ", Proceeding!" << std::endl;
       }
       else
       {
           std::cout << "Error Locking file " << str_fname << ", Aborting !" << std::endl;
           exit(-1);
       }
    }

    return lock_fd;

}
 
namespace Coal
{
    Platform::Platform() : dispatch(&dispatch_table)
    {
        p_lock_fd = begin_file_lock_crit_section("/var/lock/opencl");

	// For now, don't add the CPU device on K2X platforms unless it is
	// asserted that we want to enable it (eg. the ooo example)
	if (getenv("TI_OCL_CPU_DEVICE_ENABLE") != NULL)
        {
	    Coal::DeviceInterface * device = new Coal::CPUDevice;
            p_devices.push_back(desc(device));
        }

        for (int i = 0; i < Driver::instance()->num_dsps(); i++)
        {
	    Coal::DeviceInterface * device = new Coal::DSPDevice(i);
            p_devices.push_back(desc(device));
        }

        signal(SIGINT,  exit);
        signal(SIGABRT, exit);
        signal(SIGTERM, exit);
    }

    Platform::~Platform()
    {
        flock(p_lock_fd, LOCK_UN);
        close(p_lock_fd);

        for (int i = 0; i < p_devices.size(); i++)
	    delete pobj(p_devices[i]);
    }

    cl_uint Platform::getDevices(cl_device_type device_type, 
                                 cl_uint num_entries, cl_device_id * devices)
    {
        cl_uint device_number = 0;

        if (device_type == CL_DEVICE_TYPE_DEFAULT)
            device_type = CL_DEVICE_TYPE_ACCELERATOR;

        for (int d = 0; d < p_devices.size(); d++)
        {
            cl_device_type type;
            auto device = pobj(p_devices[d]);
            device->info(CL_DEVICE_TYPE, sizeof(cl_device_type), &type,0);

            if (type & device_type)
            {
                if (devices && device_number < num_entries)
                     devices[device_number++] = p_devices[d];
                else device_number++;
            }
        }

        return device_number;
    }

    cl_int Platform::info(cl_mem_info param_name,
                       size_t param_value_size,
                       void *param_value,
                       size_t *param_value_size_ret) const
    {
        void  *value = 0;
        size_t value_length = 0;

        switch (param_name)
        {
            case CL_PLATFORM_PROFILE:
                STRING_ASSIGN("FULL_PROFILE");
                break;

            #define STRINGIZE(x) #x
            #define STRINGIZE2(x) STRINGIZE(x)
            case CL_PLATFORM_VERSION:
                STRING_ASSIGN("OpenCL 1.1 TI product version " 
                              STRINGIZE2(_PRODUCT_VERSION)
                              " (" __DATE__ " " __TIME__ ")");
                break;

            case CL_PLATFORM_NAME:
#if defined(DEVICE_K2X)
                STRING_ASSIGN("TI KeyStone II");
#elif defined(DEVICE_AM57)
                STRING_ASSIGN("TI AM57x");
#else
                STRING_ASSIGN("TI KeyStone I");
#endif
                break;

            case CL_PLATFORM_VENDOR:
                STRING_ASSIGN("Texas Instruments, Inc.");
                break;

            case CL_PLATFORM_EXTENSIONS:
                // TODO add cl_khr_icd  when it works
                if (getenv("TI_OCL_ENABLE_FP64"))
                    STRING_ASSIGN("cl_khr_byte_addressable_store cl_khr_fp64 cl_ti_msmc_buffers cl_ti_clmalloc")
                else
                    STRING_ASSIGN("cl_khr_byte_addressable_store cl_ti_msmc_buffers cl_ti_clmalloc cl_khr_icd")
                break;

            case CL_PLATFORM_ICD_SUFFIX_KHR:
                STRING_ASSIGN("TI");
                break;

            default:
                return CL_INVALID_VALUE;
        }

        if (param_value && param_value_size < value_length)
            return CL_INVALID_VALUE;

        if (param_value_size_ret)
            *param_value_size_ret = value_length;

        if (param_value)
            std::memcpy(param_value, value, value_length);

        return CL_SUCCESS;
    }
};

