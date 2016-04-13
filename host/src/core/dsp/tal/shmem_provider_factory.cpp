#include <cassert>
#include "shared_memory_provider.h"
#include "shmem_rw_policy_cmem.h"
#include "shmem_init_policy_cmem.h"

using namespace tiocl;

SharedMemory* SharedMemoryProviderFactory::CreateSharedMemoryProvider(uint8_t device_id)
{
    // Create a CMEM based shared memory implementation
    SharedMemory* shm =
        new SharedMemoryProvider<InitializationPolicyCMEM, ReadWritePolicyCMEM>
            (device_id);

    assert (shm != nullptr);

    shmProviderMap[device_id] = shm;

    return shm;
}

void SharedMemoryProviderFactory::DestroySharedMemoryProviders()
{
    for (auto const &m : shmProviderMap)
        delete m.second;
}


SharedMemory* SharedMemoryProviderFactory::GetSharedMemoryProvider(uint8_t device_id) const
{
    std::map<uint8_t, SharedMemory*>::const_iterator it = shmProviderMap.find(device_id);

    if (it != shmProviderMap.end())
        return shmProviderMap.at(device_id);

    return nullptr;
}
