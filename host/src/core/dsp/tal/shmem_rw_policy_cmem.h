#pragma once

#include <cstdint>
#include <cstdlib>
#include "memory_provider_factory.h"

using tiocl::MemoryProviderFactory;


/****************************************************************************
 * Policy class implementing CMEM specific read/write and map/unmap
 * functionality.
 ***************************************************************************/
class ReadWritePolicyCMEM
{
public:
    void    Configure(int32_t device_id, 
                      const MemoryProviderFactory* mpFactory);

    int32_t Write(uint64_t dst, uint8_t *src, size_t sz);
    int32_t Read (uint64_t src, uint8_t *dst, size_t sz);

    void*        Map   (uint64_t addr, size_t sz, bool is_read = false,
                                  bool allow_fail = false);
    int32_t      Unmap (void *host_addr, uint64_t buf_addr,
                                  size_t sz, bool is_write = false);

    bool CacheWbInvAll();

private:
    int32_t                      device_id_;
    const MemoryProviderFactory* mp_factory_;
};

