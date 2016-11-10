#include "shared_memory_provider.h"
#include "shmem_rw_policy_pcie.h"
#include "shmem_init_policy_pcie.h"
#include "heaps_policy_thread.h"

using namespace tiocl;

// Include the file to enable template instantiation
#include "shared_memory_provider.cpp"

// Instantiate the PCIe shared memory provider
template class 
SharedMemoryProvider<InitializationPolicyPCIe, ReadWritePolicyPCIe, 
                     HeapsMultiThreadedPolicy>;

