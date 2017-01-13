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

#include <cassert>
#include "shared_memory_provider.h"
#include "shmem_rw_policy_cmem.h"
#include "shmem_init_policy_cmem.h"
#include "heaps_policy_process.h"

using namespace tiocl;

SharedMemory* SharedMemoryProviderFactory::CreateSharedMemoryProvider
           (uint8_t device_id)
{
    // Create a CMEM based shared memory implementation
    SharedMemory* shm =
        new SharedMemoryProvider<InitializationPolicyCMEM, ReadWritePolicyCMEM,
            HeapsMultiProcessPolicy> (device_id);

    assert (shm != nullptr);

    shmProviderMap[device_id] = shm;

    return shm;
}

void SharedMemoryProviderFactory::DestroySharedMemoryProviders()
{
    for (auto const &m : shmProviderMap)
        delete m.second;
}


SharedMemory* SharedMemoryProviderFactory::GetSharedMemoryProvider
      (uint8_t device_id) const
{
    std::map<uint8_t, SharedMemory*>::const_iterator it = 
             shmProviderMap.find(device_id);

    if (it != shmProviderMap.end())
        return shmProviderMap.at(device_id);

    return nullptr;
}
