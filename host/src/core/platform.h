/******************************************************************************
 * Copyright (c) 2012-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <CL/TI/cl.h>
#include <vector>
#include <cstring>
#include <memory>
#include "icd.h"
#include "shared_memory_interface.h"

#include "tiocl_thread.h"
#ifdef _SYS_BIOS
#include <Singleton.h>
#else
#define  LOKI_PTHREAD_H
#include <loki/Singleton.h>
#endif

namespace Coal
{

class Platform
{
    public:
        Platform();
        ~Platform();

        cl_uint getDevices(cl_device_type device_type,
                           cl_uint num_entries, cl_device_id * devices);

        cl_int info(cl_platform_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const;

        const tiocl::SharedMemoryProviderFactory* GetSharedMemoryProviderFactory() const
        { return p_shmFactory.get(); }

    private:
        KHRicdVendorDispatch *dispatch;
        std::vector <cl_device_id> p_devices;
        /* MCT-820 Using pointer to allow Platform to be a standard layout class */
        std::unique_ptr<tiocl::SharedMemoryProviderFactory> p_shmFactory;

    public:
        static bool constructed;
};

}

struct _cl_platform_id : public Coal::Platform 
{};

#ifndef _SYS_BIOS
// Fix for MCT-499: Delete the singleton explicitly via a gcc "destructor" function
// Loki DefaultLifetime registers destructor using atexit. This could result in
// platform being deleted before file scope OpenCL objects. gcc destructor functions
// are invoked after atexit processing.
typedef Loki::SingletonHolder<Coal::Platform, Loki::CreateUsingNew, 
                               Loki::NoDestroy, Loki::ClassLevelLockable> the_platform;
#else
typedef Loki::SingletonHolder<Coal::Platform, Loki::CreateUsingNew,
                               Loki::DefaultLifetime, Loki::SingleThreaded> the_platform;
#endif

#endif
