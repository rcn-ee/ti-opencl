#include "configuration.h"

#include <xdc/std.h>
#include "package/internal/OpenCL.xdc.h"

uint8_t getNumComputeUnits()
{
    return OpenCL_numDspCores;
}

