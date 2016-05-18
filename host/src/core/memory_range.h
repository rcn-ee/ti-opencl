#include "tiocl_types.h"
#include <cstdint>

#pragma once

namespace tiocl {

// Represents a memory range with a start address and size
// Both are 64bits to handle devices which support >4GB of memory
class MemoryRange
{
public:
    enum class Kind { CMEM_PERSISTENT, CMEM_ONDEMAND, DEVMEM, RTOS_SHMEM,
                      RTOS_HOSTMEM };
    enum class Location {ONCHIP, OFFCHIP};

    MemoryRange(DSPDevicePtr64 a, uint64_t sz, Kind k, Location l):
                start(a), size(sz), kind(k), loc(l)
    {}

    DSPDevicePtr64 GetBase() const { return start; }
    uint64_t       GetSize() const { return size; }
    Kind           GetKind() const { return kind; }
    Location       GetLocation() const { return loc; }

    bool IsAddressInRange(DSPDevicePtr64 addr) const {
        uint64_t end_exclusive = (uint64_t)start + size;
        if (addr >= start && addr <  end_exclusive)
            return true;
        return false;
    }

private:
    DSPDevicePtr64 start;
    uint64_t       size;
    Kind           kind;
    Location       loc;

};

}
