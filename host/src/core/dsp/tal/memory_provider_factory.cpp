#include <cassert>
#include "dspmem.h"
#include "memory_provider_factory.h"
#include "../error_report.h"


#if defined (DEVICE_K2X) || defined (DEVICE_K2G)
    #include "memory_provider_cmem.h"
#elif defined (DEVICE_AM57)
  #if !defined(_SYS_BIOS)
    #include "memory_provider_cmem.h"
  #else
    #include "memory_provider_rtos.h"
  #endif
#else
    #error "Device not supported"
#endif

using namespace tiocl;

void
MemoryProviderFactory::CreateMemoryProvider(const MemoryRange &r)
{
    MemoryProvider *mp = nullptr;

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
