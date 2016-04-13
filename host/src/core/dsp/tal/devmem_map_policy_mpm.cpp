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
#include <stdlib.h>
#include <cstring>
#include <cassert>

#include "memory_provider_devmem.h"
#include "devmem_map_policy_mpm.h"
#include "../error_report.h"

using namespace tiocl;


DevMemMapPolicyMPM::DevMemMapPolicyMPM()
 : dsp_addr_(0), size_(0), host_addr_(0), mpm_transport_handle_(nullptr),
   xlate_dsp_to_host_offset_(nullptr)
{ }

DevMemMapPolicyMPM::~DevMemMapPolicyMPM()
{
    if (host_addr_)
        mpm_transport_munmap(mpm_transport_handle_, host_addr_, size_);

    ReportTrace("DevMem mpm_transport_unmap  %p, %lld bytes\n",
                host_addr_, size_);
}

void DevMemMapPolicyMPM::Configure(DSPDevicePtr64 dsp_addr,  uint64_t size)
{
    // MSMC and DDR addresses go through /dev/dsp0
    const char* devname = "dsp0";

    mpm_transport_open_t mpm_transport_open_cfg;
    mpm_transport_open_cfg.open_mode = (O_SYNC|O_RDWR);
    mpm_transport_handle_ = mpm_transport_open(const_cast<char *>(devname),
                                               &mpm_transport_open_cfg);

    if (mpm_transport_handle_ == NULL)
        ReportError(ErrorType::Fatal, ErrorKind::FailedToOpenFileName, devname);

    dsp_addr_ = dsp_addr;

    mpm_transport_mmap_t mpm_transport_mmap_cfg;
    mpm_transport_mmap_cfg.mmap_prot = (PROT_READ|PROT_WRITE);
    mpm_transport_mmap_cfg.mmap_flags = MAP_SHARED;

    host_addr_ = (void *)mpm_transport_mmap(mpm_transport_handle_,
                                                dsp_addr, size,
                                                &mpm_transport_mmap_cfg);
    assert (host_addr_ != 0);

    size_     = size;

    xlate_dsp_to_host_offset_ = (void*)((int64_t)host_addr_ - dsp_addr);

    ReportTrace("DevMem mpm_transport_open 0x%llx -> %p, %lld bytes\n",
                dsp_addr, host_addr_, size_);
}

void* DevMemMapPolicyMPM::Map (DSPDevicePtr64 dsp_addr,  uint32_t size) const
{
    if (!host_addr_) return 0;

    if (dsp_addr >= dsp_addr_ && dsp_addr + size <= dsp_addr_ + size_)
         return dsp_addr + (char*)xlate_dsp_to_host_offset_;
    else
        ReportError(ErrorType::Fatal,
                    ErrorKind::TranslateAddressOutsideMappedAddressRange);
}

void DevMemMapPolicyMPM::Unmap(void* host_addr, uint32_t size) const
{}

template class DevMem<DevMemMapPolicyMPM>;
