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

extern "C"
{
    #include "pciedrv.h"
    #include "dnldmgr.h"
    #include "cmem_drv.h"
    #include "bufmgr.h"
}

#include "core/error_report.h"
#include "core/tiocl_types.h"

#include "pcie_cmem.h"
#include "shmem_rw_policy_pcie.h"

using namespace tiocl;

void
ReadWritePolicyPCIe::Configure(int32_t device_id,
                               const tiocl::MemoryProviderFactory* mpf)
{
    device_id_ = device_id;
    mp_factory_ = mpf;
}


int32_t 
ReadWritePolicyPCIe::Write(uint64_t dst, uint8_t *src, size_t size)
{
    DSPDevicePtr addr = (DSPDevicePtr) dst;
    /*-------------------------------------------------------------------------
    * Regular writes under 24k are faster than DMA writes (may change)
    *------------------------------------------------------------------------*/
    if (size < 24 * 1024)
    {
        int status = pciedrv_dsp_write(device_id_, addr, src, size);
        ReportError(ErrorType::Fatal, ErrorKind::PCIeDriverError);
        return 0;
    }

    Cmem::instance()->dma_write(device_id_, addr, src, size);
    return 0;
}

int32_t 
ReadWritePolicyPCIe::Read(uint64_t src, uint8_t *dst, size_t size)
{
    DSPDevicePtr addr = (DSPDevicePtr) src;
    Cmem::instance()->dma_read(device_id_, addr, dst, size);

    return 0;
}

void*   
ReadWritePolicyPCIe::Map(uint64_t addr, size_t sz, bool is_read, bool allow_fail)
{
    void *host_addr = malloc(sz);
    if (host_addr == NULL && !allow_fail)
        ReportError(ErrorType::Fatal,
                    ErrorKind::UnableToMapDSPAddress, "");
    if (host_addr && is_read)
        Read(addr, (uint8_t *)host_addr, sz);

    return host_addr;
}

int32_t 
ReadWritePolicyPCIe::Unmap(void *host_addr, uint64_t buf_addr, size_t sz, bool is_write)
{
    if (host_addr && is_write)
        Write(buf_addr, (uint8_t *)host_addr, sz);
    if (host_addr)  free(host_addr);

    return 0;
}

bool ReadWritePolicyPCIe::CacheWbInvAll()
{
    return true;
}
