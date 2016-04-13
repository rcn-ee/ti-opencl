#include "shared_memory_provider.h"
#include "shmem_rw_policy_cmem.h"
#include "shmem_init_policy_cmem.h"

using namespace tiocl;

// Include the file to enable template instantiation
#include "shared_memory_provider.cpp"

// Instantiate the CMEM shared memory provider
template class 
SharedMemoryProvider<InitializationPolicyCMEM, ReadWritePolicyCMEM>;

