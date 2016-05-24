#include "configuration.h"

#include <xdc/std.h>
#include "package/internal/OpenCL.xdc.h"

uint8_t getNumComputeUnits()
{
    return OpenCL_numDspCores;
}

uint32_t ti_opencl_get_OCL_GLOBAL_base()
{
    return OpenCL_OCL_GLOBAL_base;
}

uint32_t ti_opencl_get_OCL_GLOBAL_len()
{
    return OpenCL_OCL_GLOBAL_len;
}

uint32_t ti_opencl_get_OCL_HOSTMEM_base()
{
    return OpenCL_OCL_HOSTPROG_base;
}

uint32_t ti_opencl_get_OCL_HOSTMEM_len()
{
    return OpenCL_OCL_HOSTPROG_len;
}

uint32_t ti_opencl_get_OCL_LOCAL_base()
{
    return OpenCL_OCL_LOCAL_base;
}

uint32_t ti_opencl_get_OCL_LOCAL_len()
{
    return OpenCL_OCL_LOCAL_len;
}

