#include "shared_memory_provider.h"
#include "shmem_rw_policy_rtos.h"
#include "shmem_init_policy_rtos.h"

using namespace tiocl;

// Include the file to enable template instantiation
#include "shared_memory_provider.cpp"

// Instantiate the CMEM shared memory provider
template class 
SharedMemoryProvider<InitializationPolicyRTOS, ReadWritePolicyRTOS>;

