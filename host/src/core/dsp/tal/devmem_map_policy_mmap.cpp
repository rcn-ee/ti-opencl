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
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <cassert>

#include "memory_provider_devmem.h"
#include "devmem_map_policy_mmap.h"
#include "../error_report.h"

using namespace tiocl;


DevMemMapPolicyMmap::DevMemMapPolicyMmap()
 : host_addr_(nullptr), dsp_addr_(0), size_(0), xlate_dsp_to_host_offset_(nullptr), mmap_fd_(-1)
{ }


DevMemMapPolicyMmap::~DevMemMapPolicyMmap()
{
    if (host_addr_) munmap(host_addr_, size_);

    if (mmap_fd_ != -1) close(mmap_fd_);

    ReportTrace("DevMem munmap  %p, %lld bytes\n", host_addr_, size_);
}

void DevMemMapPolicyMmap::Configure(DSPDevicePtr64 dsp_addr,  uint64_t size)
{
    mmap_fd_ = open("/dev/mem", (O_RDWR | O_SYNC));
    if (mmap_fd_ == -1)
        ReportError(ErrorType::Fatal, ErrorKind::FailedToOpenFileName,
                    "/dev/mem");

    dsp_addr_ = dsp_addr;
    size_     = size;

    host_addr_ = mmap(0, size, (PROT_READ|PROT_WRITE), MAP_SHARED, mmap_fd_,
                         (off_t)dsp_addr);

    assert (host_addr_ != MAP_FAILED);

    xlate_dsp_to_host_offset_ = (void*)((int64_t)host_addr_ - dsp_addr);

    ReportTrace("DevMem mmap 0x%llx -> %p, %lld bytes\n", dsp_addr, host_addr_, size_);
}

void* DevMemMapPolicyMmap::Map (DSPDevicePtr64 dsp_addr,  size_t size) const
{
    if (!host_addr_) return 0;

    if (dsp_addr >= dsp_addr_ && dsp_addr + size <= dsp_addr_ + size_)
         return dsp_addr + (char*)xlate_dsp_to_host_offset_;
    else
        ReportError(ErrorType::Fatal,
                    ErrorKind::TranslateAddressOutsideMappedAddressRange);
    return 0;
}

void DevMemMapPolicyMmap::Unmap(void* host_addr, size_t size) const
{}

template class DevMem<DevMemMapPolicyMmap>;
