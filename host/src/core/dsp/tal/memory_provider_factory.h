#pragma once

#include "memory_provider_interface.h"
#include <vector>

namespace tiocl {

/****************************************************************************
 * Handles creation and destruction of MemoryProvider objects
 ***************************************************************************/
class MemoryProviderFactory
{
public:

    // Create and register a MemoryProvider for a given memory range
    void CreateMemoryProvider(const MemoryRange& r);

    // Given an address, return the appropriate MemoryProvider
    const MemoryProvider* GetMemoryProvider(DSPDevicePtr64 addr) const;


    // Destroy all registered MemoryProvider's
    void DestroyMemoryProviders();

    MemoryProviderFactory() =default;
    ~MemoryProviderFactory();

    MemoryProviderFactory(const MemoryProviderFactory&) =delete;
    MemoryProviderFactory& operator=(const MemoryProviderFactory&) =delete;

private:
    std::vector<const tiocl::MemoryProvider *> mpAreas_;
};

}



