/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2012-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include <core/platform.h>
#include <core/deviceinterface.h>
#include <core/dsp/subdevice.h>
#include <core/eve/device.h>
#include <core/cpu/device.h>
#include <set>
#include <vector>
#include <numeric>

cl_int
clGetDeviceIDs(cl_platform_id   platform,
               cl_device_type   device_type,
               cl_uint          num_entries,
               cl_device_id *   devices,
               cl_uint *        num_devices)
{
    /*-------------------------------------------------------------------------
    * We currently implement only one platform
    *------------------------------------------------------------------------*/
    if (!platform) platform = (cl_platform_id)&(the_platform::Instance());

    if (platform != &(the_platform::Instance())) return CL_INVALID_PLATFORM;
    if (num_entries == 0 && devices != 0)        return CL_INVALID_VALUE;
    if (num_devices == 0 && devices == 0)        return CL_INVALID_VALUE;

    int device_number = platform->getDevices(device_type,
                                         num_entries, devices);

    if (num_devices) *num_devices = device_number;

    if (device_number == 0)
        return CL_DEVICE_NOT_FOUND;

    return CL_SUCCESS;
}

cl_int
clGetDeviceInfo(cl_device_id    d_device,
                cl_device_info  param_name,
                size_t          param_value_size,
                void *          param_value,
                size_t *        param_value_size_ret)
{
    auto device = pobj(d_device);

    if (!device->isA(Coal::Object::T_Device))
        return CL_INVALID_DEVICE;

    Coal::DeviceInterface *iface = (Coal::DeviceInterface *)device;
    return iface->info(param_name, param_value_size, param_value,
                       param_value_size_ret);

}

/* Helper function to calculate the required partition unit sizes
 * */
cl_int
GetPartitionUnitSizes(std::vector<int>& pu_sizes,
                      const cl_device_partition_property* properties,
                      const cl_uint num_devices = 0)
{
    switch (properties[0])
    {
        case CL_DEVICE_PARTITION_EQUALLY:
            {
                int partition_unit_size = properties[1];
                for (int i = 0; i < num_devices; i++)
                { pu_sizes.push_back(partition_unit_size); }
                break;
            }
        case CL_DEVICE_PARTITION_BY_COUNTS:
            {
                int it = 1;
                while (properties[it] != CL_DEVICE_PARTITION_BY_COUNTS_LIST_END)
                {
                    int unit_value = static_cast<int>(properties[it]);
                    if (unit_value < 0) return CL_INVALID_DEVICE_PARTITION_COUNT;
                    pu_sizes.push_back(unit_value);
                    ++it;
                }
                break;
            }
        default:
            break;
    }
    return CL_SUCCESS;
}

/* Helper function to create the Coal::DSPDevice objects for the
 * CL_DEVICE_PARTITION_BY_COUNTS partitioning policy
 * */
cl_int
CreateSubDevicesByCounts(const std::vector<int>& partition_unit_sizes,
                         Coal::DSPDevice* parent_device,
                         const cl_device_partition_property* properties,
                         cl_device_id* out_devices)
{
    /* Create a local copy of the compute unit set */
    DSPCoreSet compute_units_on_parent_device(parent_device->GetComputeUnits());
    for (int i = 0; i < partition_unit_sizes.size(); i++)
    {
        /* Create the compute unit list */
        DSPCoreSet compute_units_for_sub_device;
        for (int j = 0; j < partition_unit_sizes[i]; j++)
        {
            /* The compute units are in sorted order. Take the first unit
             * off the root set and add it to the sub device set */
            compute_units_for_sub_device.insert(*compute_units_on_parent_device.begin());
            compute_units_on_parent_device.erase(compute_units_on_parent_device.begin());
        }
        Coal::DeviceInterface* sub_device = nullptr;
        sub_device = new Coal::DSPSubDevice(parent_device,
                                            compute_units_for_sub_device,
                                            properties);
        if (sub_device)     out_devices[i] = desc(sub_device);
        else                return CL_DEVICE_PARTITION_FAILED;
    }
    return CL_SUCCESS;
}

cl_int
clCreateSubDevices(cl_device_id                         d_device,
                   const cl_device_partition_property*  properties,
                   cl_uint                              num_devices,
                   cl_device_id*                        out_devices,
                   cl_uint*                             num_devices_ret)
{
    auto parent_device = pobj(d_device);

    if (!parent_device->isA(Coal::Object::T_Device))            return CL_INVALID_DEVICE;
    if (properties == nullptr)                                  return CL_INVALID_VALUE;
    if (out_devices == nullptr && num_devices_ret == nullptr)   return CL_INVALID_VALUE;
    if (out_devices != nullptr && !num_devices)                 return CL_INVALID_VALUE;

#if defined(DEVICE_AM57) && !defined(_SYS_BIOS)
    /* Check if device is EVE, then return error */
    if (dynamic_cast<Coal::EVEDevice*>(parent_device))
    { return CL_INVALID_DEVICE; }
    /* Check if device is CPU, then return error */
    if (dynamic_cast<Coal::CPUDevice*>(parent_device))
    { return CL_INVALID_DEVICE; }
#endif

    /* Create a local copy of the compute unit set */
    DSPCoreSet compute_units_on_parent_device(dynamic_cast<Coal::DSPDevice*>(parent_device)->GetComputeUnits());

    /* Get the platform instance */
    cl_platform_id platform = (cl_platform_id) & (the_platform::Instance());

    cl_int ret = CL_SUCCESS;
    std::vector<int> partition_unit_sizes;

    /* Calculate how many sub devices to create based on properties */
    switch (properties[0])
    {
        case CL_DEVICE_PARTITION_EQUALLY:
            {
                cl_uint partition_unit_size = properties[1];
                /* Set the number of sub devices that the device may be partitioned into according
                 * to the partitioning scheme specified in properties
                 * 1 DSP = 1 Device */
                if (num_devices_ret != nullptr)
                {
                    if (compute_units_on_parent_device.size() % partition_unit_size != 0 ||
                        compute_units_on_parent_device.size() <= partition_unit_size     ||
                        partition_unit_size == 0)
                        return CL_INVALID_VALUE;

                    *num_devices_ret = compute_units_on_parent_device.size() / partition_unit_size;
                }

                /* If out_devices is NULL, no devices need to be created.*/
                if (out_devices == nullptr) return CL_SUCCESS;

                /* If requesting more compute units than this device has, return error */
                if ((partition_unit_size * num_devices) > compute_units_on_parent_device.size())
                { return CL_INVALID_VALUE; }

                /* If requesting only one device with the size of the root device, return errror
                 * Cant have sub devices with the same number of compute units as the root device
                 * */
                if (partition_unit_size == compute_units_on_parent_device.size() && num_devices == 1)
                { return CL_INVALID_VALUE; }

                ret = GetPartitionUnitSizes(partition_unit_sizes, properties, num_devices);
                if (ret != CL_SUCCESS) return ret;

                /* Create sub devices */
                ret = CreateSubDevicesByCounts(partition_unit_sizes,
                                               dynamic_cast<Coal::DSPDevice*>(parent_device),
                                               properties, out_devices);
                break;
            }
        case CL_DEVICE_PARTITION_BY_COUNTS:
            {
                ret = GetPartitionUnitSizes(partition_unit_sizes, properties);
                if (ret != CL_SUCCESS) return ret;

                /* Set number of sub devices that the device may be partitioned
                 * into according to the partitioning scheme specified in properties
                 * */
                if (num_devices_ret != nullptr)
                {
                    *num_devices_ret = 0;
                    int total_compute_units = compute_units_on_parent_device.size();
                    for (auto & unit_size : partition_unit_sizes)
                    {
                        if (unit_size <= total_compute_units)
                        {
                            total_compute_units -= unit_size;
                            *num_devices_ret += 1;
                        }
                        else
                        {
                            /* If the partition unit size specified is greater
                             * than the number of compute units on the root
                             * device, return error * */
                            *num_devices_ret = 0;
                            return CL_INVALID_DEVICE_PARTITION_COUNT;
                        }

                    }

                    /* If only one partition is requested with size equal to
                     * the root Cant have sub devices with the same number of
                     * compute units as the root device device size, return
                     * error */
                    if (*num_devices_ret == 1 &&
                        partition_unit_sizes[0] == compute_units_on_parent_device.size())
                    {
                        *num_devices_ret = 0;
                        return CL_INVALID_DEVICE_PARTITION_COUNT;
                    }
                }

                /* If out_devices is NULL, no devices need to be created.*/
                if (out_devices == nullptr) return CL_SUCCESS;

                /* Create sub devices */
                ret = CreateSubDevicesByCounts(partition_unit_sizes,
                                               dynamic_cast<Coal::DSPDevice*>(parent_device),
                                               properties, out_devices);
                break;
            }
        case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
            {
                /* No devices can be created using this type */
                if (num_devices_ret != nullptr)
                {
                    *num_devices_ret = 0;
                    return CL_INVALID_VALUE;
                }
            }
        default:
            {
                /* Incorrect partition type provided */
                return CL_INVALID_VALUE;
            }
    }

    return ret;
}

/* Helper function to check if a given device is a valid object */
template <class D>
bool IsValidDevice(cl_device_id d_device)
{
    auto device = pobj(d_device);

    if (!device->isA(Coal::Object::T_Device)) return false;
    if (dynamic_cast<D*>(device))             return true;

    return false;
}

cl_int
clRetainDevice(cl_device_id d_device)
{
    /* On page 53, the OpenCL 1.2 specification says: "clRetainDevice returns
     * CL_SUCCESS if the function is executed successfully or the device is a
     * root-level device."
     * */
#if !defined(_SYS_BIOS)
    if (IsValidDevice<Coal::EVEDevice>(d_device) ||
        IsValidDevice<Coal::CPUDevice>(d_device))     return CL_SUCCESS;
#endif
    if (IsValidDevice<Coal::DSPRootDevice>(d_device)) return CL_SUCCESS;
    if (!IsValidDevice<Coal::DSPSubDevice>(d_device)) return CL_INVALID_DEVICE;

    auto device = pobj(d_device);
    device->reference();
    return CL_SUCCESS;
}

cl_int
clReleaseDevice(cl_device_id d_device)
{
    /* On page 53, the OpenCL 1.2 specification says: "If device is a root level
     * device i.e. a cl_device_id returned by clGetDeviceIDs, the device
     * reference count remains unchanged.
     * */
#if !defined(_SYS_BIOS)
    if (IsValidDevice<Coal::EVEDevice>(d_device) ||
        IsValidDevice<Coal::CPUDevice>(d_device))     return CL_SUCCESS;
#endif
    if (IsValidDevice<Coal::DSPRootDevice>(d_device)) return CL_SUCCESS;
    if (!IsValidDevice<Coal::DSPSubDevice>(d_device)) return CL_INVALID_DEVICE;

    auto device = pobj(d_device);
    if (device->dereference()) delete device;
    return CL_SUCCESS;
}


