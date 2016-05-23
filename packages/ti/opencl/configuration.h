#pragma once

#include <stdint.h>

uint8_t getNumComputeUnits();
uint32_t ti_opencl_get_OCL_GLOBAL_base();
uint32_t ti_opencl_get_OCL_GLOBAL_len();
uint32_t ti_opencl_get_OCL_HOSTMEM_base();
uint32_t ti_opencl_get_OCL_HOSTMEM_len();
uint32_t ti_opencl_get_OCL_LOCAL_base();
uint32_t ti_opencl_get_OCL_LOCAL_len();
