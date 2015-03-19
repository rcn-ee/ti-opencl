/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <CL/cl.h>
#include <stdlib.h>
#include <stdio.h>

#define ERR(code) do {                                          \
    printf("Error at %i: %s\n", __LINE__, errcode_str(code));   \
} while (0);

static const char *errcode_str(cl_int code)
{
    switch (code)
    {
        case 0:
            return "CL_SUCCESS";

        case -1:
            return "CL_DEVICE_NOT_FOUND";

        case -2:
            return "CL_DEVICE_NOT_AVAILABLE";

        case -3:
            return "CL_COMPILER_NOT_AVAILABLE";

        case -4:
            return "CL_MEM_OBJECT_ALLOCATION_FAILURE";

        case -5:
            return "CL_OUT_OF_RESOURCES";

        case -6:
            return "CL_OUT_OF_HOST_MEMORY";

        case -7:
            return "CL_PROFILING_INFO_NOT_AVAILABLE";

        case -8:
            return "CL_MEM_COPY_OVERLAP";

        case -9:
            return "CL_IMAGE_FORMAT_MISMATCH";

        case -10:
            return "CL_IMAGE_FORMAT_NOT_SUPPORTED";

        case -11:
            return "CL_BUILD_PROGRAM_FAILURE";

        case -12:
            return "CL_MAP_FAILURE";

        case -13:
            return "CL_MISALIGNED_SUB_BUFFER_OFFSET";

        case -14:
            return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";

        case -30:
            return "CL_INVALID_VALUE";

        case -31:
            return "CL_INVALID_DEVICE_TYPE";

        case -32:
            return "CL_INVALID_PLATFORM";

        case -33:
            return "CL_INVALID_DEVICE";

        case -34:
            return "CL_INVALID_CONTEXT";

        case -35:
            return "CL_INVALID_QUEUE_PROPERTIES";

        case -36:
            return "CL_INVALID_COMMAND_QUEUE";

        case -37:
            return "CL_INVALID_HOST_PTR";

        case -38:
            return "CL_INVALID_MEM_OBJECT";

        case -39:
            return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";

        case -40:
            return "CL_INVALID_IMAGE_SIZE";

        case -41:
            return "CL_INVALID_SAMPLER";

        case -42:
            return "CL_INVALID_BINARY";

        case -43:
            return "CL_INVALID_BUILD_OPTIONS";

        case -44:
            return "CL_INVALID_PROGRAM";

        case -45:
            return "CL_INVALID_PROGRAM_EXECUTABLE";

        case -46:
            return "CL_INVALID_KERNEL_NAME";

        case -47:
            return "CL_INVALID_KERNEL_DEFINITION";

        case -48:
            return "CL_INVALID_KERNEL";

        case -49:
            return "CL_INVALID_ARG_INDEX";

        case -50:
            return "CL_INVALID_ARG_VALUE";

        case -51:
            return "CL_INVALID_ARG_SIZE";

        case -52:
            return "CL_INVALID_KERNEL_ARGS";

        case -53:
            return "CL_INVALID_WORK_DIMENSION";

        case -54:
            return "CL_INVALID_WORK_GROUP_SIZE";

        case -55:
            return "CL_INVALID_WORK_ITEM_SIZE";

        case -56:
            return "CL_INVALID_GLOBAL_OFFSET";

        case -57:
            return "CL_INVALID_EVENT_WAIT_LIST";

        case -58:
            return "CL_INVALID_EVENT";

        case -59:
            return "CL_INVALID_OPERATION";

        case -60:
            return "CL_INVALID_GL_OBJECT";

        case -61:
            return "CL_INVALID_BUFFER_SIZE";

        case -62:
            return "CL_INVALID_MIP_LEVEL";

        case -63:
            return "CL_INVALID_GLOBAL_WORK_SIZE";

        case -64:
            return "CL_INVALID_PROPERTY";
    }

    return "INVALID ERROR CODE";
}

void platform_info(cl_platform_id platform);
void device_info(cl_device_id device);

int main()
{
    cl_platform_id *platforms;
    cl_int result;
    cl_uint num;
    int i;

    // Get the platforms
    result = clGetPlatformIDs(0, 0, &num);

    if (result != CL_SUCCESS)
    {
        ERR(result);
        return 1;
    }

    platforms = malloc(num * sizeof(cl_platform_id));

    if (!platforms)
    {
        ERR(CL_OUT_OF_HOST_MEMORY);
        return 1;
    }

    result = clGetPlatformIDs(num, platforms, 0);

    if (result != CL_SUCCESS)
    {
        ERR(result);
        return 1;
    }

    for (i=0; i<num; ++i)
    {
        platform_info(platforms[i]);
    }
}

#define PL_PROPERTY_STRING(name, name_str) do {   \
    result = clGetPlatformInfo(platform, name, sizeof(buf), buf, 0);    \
                                                                        \
    if (result != CL_SUCCESS)                                           \
    {                                                                   \
        ERR(result);                                                    \
        return;                                                         \
    }                                                                   \
                                                                        \
    printf("    " name_str ": %s\n", buf);                              \
} while (0);



void platform_info(cl_platform_id platform)
{
    // Display info about the platform
    char buf[512];
    cl_int result;
    int i;

    printf("Platform : {\n");

    PL_PROPERTY_STRING(CL_PLATFORM_PROFILE, "CL_PLATFORM_PROFILE");
    PL_PROPERTY_STRING(CL_PLATFORM_VERSION, "CL_PLATFORM_VERSION");
    PL_PROPERTY_STRING(CL_PLATFORM_NAME, "CL_PLATFORM_NAME");
    PL_PROPERTY_STRING(CL_PLATFORM_VENDOR, "CL_PLATFORM_VENDOR");
    PL_PROPERTY_STRING(CL_PLATFORM_EXTENSIONS, "CL_PLATFORM_EXTENSIONS");

    printf("\n");

    // Display information about the devices
    cl_uint num;
    cl_device_id *devices;

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, 0, &num);

    if (result != CL_SUCCESS)
    {
        ERR(result);
        return;
    }

    devices = malloc(num * sizeof(cl_device_id));

    if (!devices)
    {
        ERR(CL_OUT_OF_HOST_MEMORY);
        return;
    }

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num, devices, 0);

    if (result != CL_SUCCESS)
    {
        ERR(result);
        return;
    }

    for (i=0; i<num; ++i)
    {
        device_info(devices[i]);
    }

    printf("}\n");
}

#define D_PROPERTY_STRING(name, name_str) do {   \
    result = clGetDeviceInfo(device, name, sizeof(buf), buf, 0);        \
                                                                        \
    if (result != CL_SUCCESS)                                           \
    {                                                                   \
        ERR(result);                                                    \
        return;                                                         \
    }                                                                   \
                                                                        \
    printf("        " name_str ": %s\n", buf);                          \
} while (0);

#define D_PROPERTY_SIMPLE(name, name_str, type) do {   \
    result = clGetDeviceInfo(device, name, sizeof(type), &un, 0);       \
                                                                        \
    if (result != CL_SUCCESS)                                           \
    {                                                                   \
        ERR(result);                                                    \
        return;                                                         \
    }                                                                   \
                                                                        \
    type t = un.type##_var;                                             \
                                                                        \
    if (sizeof(type) == 1) {                                            \
        printf("        " name_str ": %hhu\n", t);                      \
    } else if (sizeof(type) == 2) {                                     \
        printf("        " name_str ": %hu\n", t);                       \
    } else if (sizeof(type) == 4) {                                     \
        printf("        " name_str ": %u\n", t);                        \
    } else if (sizeof(type) == 8) {                                     \
        printf("        " name_str ": %llu\n", t);                      \
    }                                                                   \
} while (0);

void device_info(cl_device_id device)
{
    // Display info about the platform
    char buf[512];
    cl_int result;

    union {
        cl_device_type cl_device_type_var;
        cl_uint cl_uint_var;
        size_t size_t_var;
        cl_ulong cl_ulong_var;
        cl_bool cl_bool_var;
        cl_device_fp_config cl_device_fp_config_var;
        cl_device_mem_cache_type cl_device_mem_cache_type_var;
        cl_device_local_mem_type cl_device_local_mem_type_var;
        cl_device_exec_capabilities cl_device_exec_capabilities_var;
        cl_command_queue_properties cl_command_queue_properties_var;
        cl_platform_id cl_platform_id_var;
    } un;

    printf("    Device : {\n");

    D_PROPERTY_SIMPLE(CL_DEVICE_TYPE, "CL_DEVICE_TYPE", cl_device_type);
    D_PROPERTY_SIMPLE(CL_DEVICE_VENDOR_ID, "CL_DEVICE_VENDOR_ID", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_MAX_COMPUTE_UNITS, "CL_DEVICE_MAX_COMPUTE_UNITS", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_MAX_WORK_GROUP_SIZE, "CL_DEVICE_MAX_WORK_GROUP_SIZE", size_t);
    D_PROPERTY_SIMPLE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_MAX_CLOCK_FREQUENCY, "CL_DEVICE_MAX_CLOCK_FREQUENCY", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_ADDRESS_BITS, "CL_DEVICE_ADDRESS_BITS", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_MAX_READ_IMAGE_ARGS, "CL_DEVICE_MAX_READ_IMAGE_ARGS", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_MAX_WRITE_IMAGE_ARGS, "CL_DEVICE_MAX_WRITE_IMAGE_ARGS", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_MAX_MEM_ALLOC_SIZE, "CL_DEVICE_MAX_MEM_ALLOC_SIZE", cl_ulong);
    D_PROPERTY_SIMPLE(CL_DEVICE_IMAGE2D_MAX_WIDTH, "CL_DEVICE_IMAGE2D_MAX_WIDTH", size_t);
    D_PROPERTY_SIMPLE(CL_DEVICE_IMAGE2D_MAX_HEIGHT, "CL_DEVICE_IMAGE2D_MAX_HEIGHT", size_t);
    D_PROPERTY_SIMPLE(CL_DEVICE_IMAGE3D_MAX_WIDTH, "CL_DEVICE_IMAGE3D_MAX_WIDTH", size_t);
    D_PROPERTY_SIMPLE(CL_DEVICE_IMAGE3D_MAX_HEIGHT, "CL_DEVICE_IMAGE3D_MAX_HEIGHT", size_t);
    D_PROPERTY_SIMPLE(CL_DEVICE_IMAGE3D_MAX_DEPTH, "CL_DEVICE_IMAGE3D_MAX_DEPTH", size_t);
    D_PROPERTY_SIMPLE(CL_DEVICE_IMAGE_SUPPORT, "CL_DEVICE_IMAGE_SUPPORT", cl_bool);
    D_PROPERTY_SIMPLE(CL_DEVICE_MAX_PARAMETER_SIZE, "CL_DEVICE_MAX_PARAMETER_SIZE", size_t);
    D_PROPERTY_SIMPLE(CL_DEVICE_MAX_SAMPLERS, "CL_DEVICE_MAX_SAMPLERS", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_MEM_BASE_ADDR_ALIGN, "CL_DEVICE_MEM_BASE_ADDR_ALIGN", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, "CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_SINGLE_FP_CONFIG, "CL_DEVICE_SINGLE_FP_CONFIG", cl_device_fp_config);
    D_PROPERTY_SIMPLE(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, "CL_DEVICE_GLOBAL_MEM_CACHE_TYPE", cl_device_mem_cache_type);
    D_PROPERTY_SIMPLE(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, "CL_DEVICE_GLOBAL_MEM_CACHE_SIZE", cl_ulong);
    D_PROPERTY_SIMPLE(CL_DEVICE_GLOBAL_MEM_SIZE, "CL_DEVICE_GLOBAL_MEM_SIZE", cl_ulong);
    D_PROPERTY_SIMPLE(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE", cl_ulong);
    D_PROPERTY_SIMPLE(CL_DEVICE_MAX_CONSTANT_ARGS, "CL_DEVICE_MAX_CONSTANT_ARGS", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_LOCAL_MEM_TYPE, "CL_DEVICE_LOCAL_MEM_TYPE", cl_device_local_mem_type);
    D_PROPERTY_SIMPLE(CL_DEVICE_LOCAL_MEM_SIZE, "CL_DEVICE_LOCAL_MEM_SIZE", cl_ulong);
    D_PROPERTY_SIMPLE(CL_DEVICE_ERROR_CORRECTION_SUPPORT, "CL_DEVICE_ERROR_CORRECTION_SUPPORT", cl_bool);
    D_PROPERTY_SIMPLE(CL_DEVICE_PROFILING_TIMER_RESOLUTION, "CL_DEVICE_PROFILING_TIMER_RESOLUTION", size_t);
    D_PROPERTY_SIMPLE(CL_DEVICE_ENDIAN_LITTLE, "CL_DEVICE_ENDIAN_LITTLE", cl_bool);
    D_PROPERTY_SIMPLE(CL_DEVICE_AVAILABLE, "CL_DEVICE_AVAILABLE", cl_bool);
    D_PROPERTY_SIMPLE(CL_DEVICE_COMPILER_AVAILABLE, "CL_DEVICE_COMPILER_AVAILABLE", cl_bool);
    D_PROPERTY_SIMPLE(CL_DEVICE_EXECUTION_CAPABILITIES, "CL_DEVICE_EXECUTION_CAPABILITIES", cl_device_exec_capabilities);
    D_PROPERTY_SIMPLE(CL_DEVICE_QUEUE_PROPERTIES, "CL_DEVICE_QUEUE_PROPERTIES", cl_command_queue_properties);
    D_PROPERTY_STRING(CL_DEVICE_NAME, "CL_DEVICE_NAME");
    D_PROPERTY_STRING(CL_DEVICE_VENDOR, "CL_DEVICE_VENDOR");
    D_PROPERTY_STRING(CL_DRIVER_VERSION, "CL_DRIVER_VERSION");
    D_PROPERTY_STRING(CL_DEVICE_PROFILE, "CL_DEVICE_PROFILE");
    D_PROPERTY_STRING(CL_DEVICE_VERSION, "CL_DEVICE_VERSION");
    D_PROPERTY_STRING(CL_DEVICE_EXTENSIONS, "CL_DEVICE_EXTENSIONS");
    D_PROPERTY_SIMPLE(CL_DEVICE_PLATFORM, "CL_DEVICE_PLATFORM", cl_platform_id);
    D_PROPERTY_SIMPLE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_HOST_UNIFIED_MEMORY, "CL_DEVICE_HOST_UNIFIED_MEMORY", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, "CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, "CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, "CL_DEVICE_NATIVE_VECTOR_WIDTH_INT", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, "CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, "CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT", cl_uint);
    D_PROPERTY_SIMPLE(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, "CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE", cl_uint);    D_PROPERTY_SIMPLE(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, "CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF", cl_uint);
    D_PROPERTY_STRING(CL_DEVICE_OPENCL_C_VERSION, "CL_DEVICE_OPENCL_C_VERSION");

    printf("    }\n");
}
