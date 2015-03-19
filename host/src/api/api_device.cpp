/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file api_device.cpp
 * \brief Devices
 */

#include "CL/cl.h"
#include "api_platform.h"
#include <core/cpu/device.h>
#include <core/dsp/device.h>

extern Coal::CPUDevice cpudevice;
extern Coal::DSPDevice dsp0device;
extern Coal::DSPDevice dsp1device;
extern Coal::DSPDevice dsp2device;
extern Coal::DSPDevice dsp3device;

cl_int
clGetDeviceIDs(cl_platform_id   platform,
               cl_device_type   device_type,
               cl_uint          num_entries,
               cl_device_id *   devices,
               cl_uint *        num_devices)
{
    int device_number = 0;

    /*-------------------------------------------------------------------------
    * We currently implement only one platform
    *------------------------------------------------------------------------*/
    if (platform != PLATFORM_ID)          return CL_INVALID_PLATFORM;
    if (num_entries == 0 && devices != 0) return CL_INVALID_VALUE;
    if (num_devices == 0 && devices == 0) return CL_INVALID_VALUE;

    if (device_type & (CL_DEVICE_TYPE_DEFAULT | CL_DEVICE_TYPE_CPU))
    {
        //cpudevice.init();
        
        if (devices)
            devices[device_number] = (cl_device_id)(&cpudevice);

        device_number++;
    }

    if (device_type & CL_DEVICE_TYPE_ACCELERATOR)
    {
        //dsp0device.init();
        //dsp1device.init();
        //dsp2device.init();
        //dsp3device.init();
        
        if (devices)
        {
            devices[device_number    ] = (cl_device_id)(&dsp0device);
            devices[device_number + 1] = (cl_device_id)(&dsp1device);
            devices[device_number + 2] = (cl_device_id)(&dsp2device);
            devices[device_number + 3] = (cl_device_id)(&dsp3device);
        }

        device_number += 4;
    }

    if (num_devices) *num_devices = device_number;

    if (device_number == 0)
        return CL_DEVICE_NOT_FOUND;

    return CL_SUCCESS;
}

cl_int
clGetDeviceInfo(cl_device_id    device,
                cl_device_info  param_name,
                size_t          param_value_size,
                void *          param_value,
                size_t *        param_value_size_ret)
{
    if (!device->isA(Coal::Object::T_Device))
        return CL_INVALID_DEVICE;

    Coal::DeviceInterface *iface = (Coal::DeviceInterface *)device;
    return iface->info(param_name, param_value_size, param_value,
                       param_value_size_ret);
}
