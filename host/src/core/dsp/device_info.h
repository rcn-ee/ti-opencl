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
#ifndef _DRIVER_H
#define _DRIVER_H
#include <cstdint>
#include <string>

#include "../tiocl_types.h"

#include "symbol_address_interface.h"

namespace tiocl {

// Singleton containing device information
class DeviceInfo {
public:
    DeviceInfo();
    virtual ~DeviceInfo();

    // Disable copy constuction and assignment
    DeviceInfo(const DeviceInfo&)            =delete;
    DeviceInfo& operator=(const DeviceInfo&) =delete;

    uint8_t      GetNumDevices() const { return num_devices_; } // was num_dsps
    uint8_t      GetNumEVEDevices() const { return num_eve_devices_; }
    int32_t      GetCmemBlockOffChip() const { return cmem_block_offchip_; }
    int32_t      GetCmemBlockOnChip()  const { return cmem_block_onchip_; }
    std::string  FullyQualifiedPathToDspMonitor() const;
    uint8_t      GetComputeUnitsPerDevice(int device) const; // was cores_per_dsp(int dsp);
    DSPDevicePtr GetSymbolAddress(const std::string &name) const;

    const DSPCoreSet& GetComputeUnits() const { return available_compute_units_; }

    static const DeviceInfo& Instance();

private:

    void ComputeUnits_CmemBlocks_Available();
    void EVEDevicesAvailable();

    uint8_t num_devices_;
    uint8_t num_eve_devices_;
    uint8_t num_compute_units_;
    int32_t cmem_block_offchip_;
    int32_t cmem_block_onchip_;
    DSPCoreSet available_compute_units_;
    const SymbolAddressLookup* symbol_lookup_;
};

}

#endif // _DRIVER_H
