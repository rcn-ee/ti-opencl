/******************************************************************************
 * Copyright (c) 2013-2018, Texas Instruments Incorporated - http://www.ti.com/
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
#include "subdevice.h"

using namespace Coal;

/******************************************************************************
* DSPSubDevice::DSPSubDevice()
******************************************************************************/
DSPSubDevice::DSPSubDevice(DSPDevice*                           parent_device,
                           const DSPCoreSet&                    compute_units,
                           const cl_device_partition_property*  partition_properties)
    : DSPDevice(parent_device->GetSHMHandler())
{
    /*-------------------------------------------------------------------------
    * Set compute unit list
    *------------------------------------------------------------------------*/
    p_compute_units = compute_units;

    /*-------------------------------------------------------------------------
    * Set parent
    *------------------------------------------------------------------------*/
    p_parent = parent_device;

    /*-------------------------------------------------------------------------
    * Set possible partition types
    *------------------------------------------------------------------------*/
    if (p_compute_units.size() > 1)
    {
        p_partitions_supported[0] = CL_DEVICE_PARTITION_EQUALLY;
        p_partitions_supported[1] = CL_DEVICE_PARTITION_BY_COUNTS;
    }

    /*-------------------------------------------------------------------------
    * Copy partition properties into local structure
    *------------------------------------------------------------------------*/
    int data_size = 1; /* Start at 1 to account for the 0 after LIST_END */
    int it = 0;

    while (partition_properties[it++] != CL_DEVICE_PARTITION_BY_COUNTS_LIST_END) data_size += 1;
    std::memcpy(p_partition_type, partition_properties,
                sizeof(cl_device_partition_property)*data_size);
}
