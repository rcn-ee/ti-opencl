#include <cassert>
#include "dspmem.h"
#include "memory_provider_factory.h"
#include "../error_report.h"


#if defined (DEVICE_K2X) || defined (DEVICE_K2G)
    #define DEVICE_USES_DEVMEM_MPM      (1)
    #define DEVICE_USES_DEVMEM_MMAP     (!(DEVICE_USES_DEVMEM_MPM))

    #include "memory_provider_devmem_mpm.h"
    #include "memory_provider_cmem.h"
#elif defined (DEVICE_AM57)
  #if !defined(_SYS_BIOS)
    #define DEVICE_USES_DEVMEM_MPM      (0)
    #define DEVICE_USES_DEVMEM_MMAP     (!(DEVICE_USES_DEVMEM_MPM))

    #include "memory_provider_devmem_mmap.h"
    #include "memory_provider_cmem.h"
  #else
    #include "memory_provider_rtos.h"
  #endif
#elif defined (DSPC868X)
    #define DEVICE_USES_DEVMEM_MPM      (0)
    #define DEVICE_USES_DEVMEM_MMAP     (0)
    #include "memory_provider_pcie.h"
#else
    #error "Device not supported"
#endif

using namespace tiocl;

void
MemoryProviderFactory::CreateMemoryProvider(const MemoryRange &r)
{
    MemoryProvider *mp = nullptr;

    #if !defined(DSPC868X)
    switch (r.GetKind())
    {
        #if !defined(_SYS_BIOS)
        case MemoryRange::Kind::CMEM_ONDEMAND:
        {
            mp = new CMEMOnDemand(r);
            break;
        }
        case MemoryRange::Kind::CMEM_PERSISTENT:
        {
            mp = new CMEMPersistent(r);
            break;
        }
        case MemoryRange::Kind::DEVMEM:
        {
            #if DEVICE_USES_DEVMEM_MPM
            mp = new DevMemMPM(r);
            #else
            mp = new DevMemMmap(r);
            #endif
            break;
        }
        #else
        case MemoryRange::Kind::RTOS_SHMEM:
        case MemoryRange::Kind::RTOS_HOSTMEM:
        {
            mp = new RTOSMemPersistent(r);
            break;
        }
        #endif
        default:
            assert(0);
            break;
    }
    #else
    assert (r.GetKind() == MemoryRange::Kind::CMEM_PERSISTENT);
    mp = new MemoryProviderPCIe(r);
    #endif

    assert (mp != nullptr);

    mpAreas_.push_back(mp);
}

const MemoryProvider*
MemoryProviderFactory:: GetMemoryProvider(DSPDevicePtr64 addr) const
{
    for (const auto mp: mpAreas_)
    {
        if (mp->IsAddressInRange(addr))
            return mp;
    }

    ReportError(ErrorType::Fatal, ErrorKind::IllegalMemoryRegion,
                addr);

    return nullptr;
}

MemoryProviderFactory::~MemoryProviderFactory()
{
    DestroyMemoryProviders();
}

// Added a destroy function to ensure CMEM_freePhys calls in
// MemoryProvider destructor are invoked before the call to
// CMEM_exit. Refer ~SharedMemoryProvider()
void MemoryProviderFactory::DestroyMemoryProviders()
{
    while (!mpAreas_.empty())
    {
        delete mpAreas_.back();
        mpAreas_.pop_back();
    }
}
