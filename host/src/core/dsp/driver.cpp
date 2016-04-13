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
#ifdef DSPC868X
#include "driver_c6678.cpp"
#include "cmem.cpp"
#else
#include "driver_hawking.cpp"
#endif


#include "../platform.h"

extern "C" {

int32_t __device_write(int32_t dsp, DSPDevicePtr64 addr,
                       uint8_t *buf, uint32_t sz)
{
    const SharedMemoryProviderFactory &shmFactory =
        the_platform::Instance().GetSharedMemoryProviderFactory();
    SharedMemory* shm = shmFactory.GetSharedMemoryProvider(dsp);
    assert (shm != nullptr);

    return shm->WriteToShmem(addr, buf, sz);
}

int32_t __device_read (int32_t dsp, DSPDevicePtr64 addr,
                       uint8_t *buf, uint32_t sz)
{
    const SharedMemoryProviderFactory &shmFactory =
        the_platform::Instance().GetSharedMemoryProviderFactory();
    SharedMemory* shm = shmFactory.GetSharedMemoryProvider(dsp);
    assert (shm != nullptr);

    return shm->ReadFromShmem(addr, buf, sz);
}

}
