#include <cassert>
#include "dspmem.h"
#include "memory_provider_factory.h"
#include "memory_provider_cmem.h"
#include "../error_report.h"


#if defined (DEVICE_K2X) || defined (DEVICE_K2G)
#define DEVICE_USES_DEVMEM_MPM      (1)
#define DEVICE_USES_DEVMEM_MMAP     (!(DEVICE_USES_DEVMEM_MPM))

#include "memory_provider_devmem_mpm.h"
#elif defined (DEVICE_AM57)
#define DEVICE_USES_DEVMEM_MPM      (0)
#define DEVICE_USES_DEVMEM_MMAP     (!(DEVICE_USES_DEVMEM_MPM))

#include "memory_provider_devmem_mmap.h"
#endif

using namespace tiocl;

void
MemoryProviderFactory::CreateMemoryProvider(const MemoryRange &r)
{
    MemoryProvider *mp = nullptr;
    switch (r.GetKind())
    {
        case MemoryRange::Kind::CMEM_ONDEMAND:
            mp = new CMEMOnDemand(r);
            break;
        case MemoryRange::Kind::CMEM_PERSISTENT:
            mp = new CMEMPersistent(r);
            break;
        case MemoryRange::Kind::DEVMEM:
        {
            #if DEVICE_USES_DEVMEM_MPM
            mp = new DevMemMPM(r);
            #else
            mp = new DevMemMmap(r);
            #endif
            break;
        }
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
