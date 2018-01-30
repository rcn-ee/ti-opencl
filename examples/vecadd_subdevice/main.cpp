/******************************************************************************
 * Copyright (c) 2018, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include <CL/cl.hpp>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cstdarg>
#include <unistd.h>
#include <ctime>
#include "ocl_util.h"

using namespace cl;
using namespace std;

/* ======================================================================== */
/* This example demonstrates how multiple sub devices can be created from a */
/* single OpenCL device. Each of these sub devices have all the properties  */
/* of their parent device and can be used in the same context with the same */
/* memory buffers. Using separate command queues for each sub device, this  */
/* example runs a vector addition kernel on each sub device and shows the   */
/* difference in performance across the root device and its sub devices.    */
/* ======================================================================== */
/* ======================================================================== */
/* Global constants                                                         */
/* ======================================================================== */
#define MAX_SOURCE_SIZE        (0x100000)
#define ROOT_DEVICE_IDX        0
#define SUB_DEVICE_IDX_START   (ROOT_DEVICE_IDX+1)

const bool     DEBUG           = true; /* Enable this for some debug output */
const char     fileName[]      = "./kernel.cl";
const cl_uint  NumElements     = 8 * 1024 * 1024; /* 8 Mb */
const cl_uint  NumWorkGroups   = 256;
const cl_uint  VectorElements  = 4;
const cl_uint  NumVecElements  = NumElements / VectorElements;
const cl_uint  WorkGroupSize   = NumVecElements / NumWorkGroups;

/* ======================================================================== */
/* Utility function prototypes                                              */
/* ======================================================================== */
cl_uint GetNumComputeUnits(cl_device_id* device);
cl_uint LoadKernelSource(char* src_str);
void CreateDeviceBuffer(cl_context* context,
                        cl_mem*     buf,
                        cl_uint     size,
                        cl_short*   src);
void InitArrays(cl_short** srcA,
                cl_short** srcB,
                cl_short** Golden,
                cl_short** dst,
                cl_uint    n_elems);
void EnqueueKernelAndWaitForFinish(cl_command_queue* q,
                                   cl_kernel*        kernel,
                                   const cl_uint*    global_work_size,
                                   const cl_uint*    local_work_size);
void SetKernelArguments(cl_kernel* kernel,
                        cl_mem*    bufA,
                        cl_mem*    bufB,
                        cl_mem*    bufDst);
void CreateCommandQueues(cl_context*        context,
                         cl_device_id*      devices,
                         cl_command_queue** Qs,
                         cl_uint            num_Qs);
bool IsCorrect(cl_short* golden,
               cl_short* dst,
               cl_uint   n_elem);
double RunKernelOnQ(cl_command_queue* Q,
                    cl_kernel*        kernel,
                    cl_mem*           bufA,
                    cl_short*         srcA,
                    cl_mem*           bufB,
                    cl_short*         srcB,
                    cl_mem*           bufDst,
                    cl_short*         dst,
                    cl_short*         Golden,
                    cl_uint           n_elems);
void ResetResultArray(cl_short* results,
                      cl_uint   n_elems);
void DebugPrint(const char* fmt, ...);
void CleanUpQs(cl_command_queue* Qs,
               cl_uint numQs);
void CleanUpCLObjects(cl_kernel*  kernel,
                      cl_program* program,
                      cl_context* context);
void CleanUpSubDevices(cl_device_id* subdevices,
                       cl_uint       num);
void CleanUpMemoryObjects(cl_mem*   bufA,
                          cl_mem*   bufB,
                          cl_mem*   bufDst,
                          cl_short* srcA,
                          cl_short* srcB,
                          cl_short* dst,
                          cl_short* Golden);
void ShowTimingData(double*       elapsed_times,
                    cl_device_id* devices,
                    cl_uint       n_devices);
/* ======================================================================== */
/* Timing Setup                                                             */
/* ======================================================================== */
struct timespec t0,t1;
#define tick()  clock_gettime(CLOCK_MONOTONIC, &t0);
#define tock() (clock_gettime(CLOCK_MONOTONIC, &t1), \
                        t1.tv_sec - t0.tv_sec + (t1.tv_nsec - t0.tv_nsec) / 1e9)

/* ======================================================================== */
/*  MAIN                                                                    */
/* ======================================================================== */
int main()
{
    cl_int                       errcode      = CL_SUCCESS;
    cl_uint                      bufsize      = sizeof(cl_short) * NumElements;

    /*-------------------------------------------------------------------------
     * Initialized OpenCL platform, root device and
     * determine number of Sub Devices that can be created
     *------------------------------------------------------------------------*/
    /* Get platform ID */
    cl_platform_id platform;
    cl_uint        n_platforms = 0;
    errcode = clGetPlatformIDs(1,
                               &platform,
                               &n_platforms);
    assert(errcode == CL_SUCCESS);

    /* Get Accelerator Root Device ID */
    cl_device_id root_device;
    cl_uint      n_devices = 0;
    errcode = clGetDeviceIDs(platform,
                             CL_DEVICE_TYPE_ACCELERATOR,
                             1,
                             &root_device,
                             &n_devices);
    assert(errcode == CL_SUCCESS);

    DebugPrint("Initialized %d OpenCL Root Device\n", n_devices);

    /* Get number of compute units on root device */
    cl_uint n_comp_units = GetNumComputeUnits(&root_device);

    DebugPrint("Root Device has %d compute units\n", n_comp_units);

    /* If there is only 1 DSP, then no sub devices will be created. Exit. */
    if (n_comp_units <= 1)
    {
        cout << "No sub devices can be created on this platform." << endl;
        return 0;
    }

    /*-------------------------------------------------------------------------
     * Allocate memory for arrays and initialize them
     *------------------------------------------------------------------------*/
    /* 1 compute unit = 1 sub device
     * Total number of devices = number of compute units + root device */
    n_devices = n_comp_units + 1;
    /* Initialize array to hold all device ids */
    cl_device_id* devices = new cl_device_id[n_devices];
    /* Hold the device and the subdevices in this array with the root device at
     * the start */
    devices[ROOT_DEVICE_IDX] = root_device;
    /* Initialize array to hold all command queues for each device */
    cl_command_queue* Qs = new cl_command_queue[n_devices];
    /* Initialize array to hold elapsed times for each run of Vecadd on
     * different queues */
    double* elapsed_times = new double[n_devices];
    /* Allocate and initialize input and output arrays */
    cl_short  *srcA, *srcB, *dst, *Golden;
    InitArrays(&srcA, &srcB, &Golden, &dst, NumElements);

    /*-------------------------------------------------------------------------
     * Read Vecadd kernel source from file
     *------------------------------------------------------------------------*/
    /* Read kernel source code into string */
    char*   src_str  = new char[MAX_SOURCE_SIZE];
    cl_uint src_size = LoadKernelSource(src_str);

    /*-------------------------------------------------------------------------
     * Create Sub Devices
     *------------------------------------------------------------------------*/
    /* Create sub-device properties, equally with 1 compute unit each. */
    cl_device_partition_property properties[3];
    properties[0] = CL_DEVICE_PARTITION_EQUALLY; /* Divide equally           */
    properties[1] = 1;                           /* 1 compute unit/subdevice */
    properties[2] = 0;                           /* End of the property list */

    /* How many sub devices can be created? */
    cl_uint n_sub_devices = 0;
    errcode = clCreateSubDevices(devices[ROOT_DEVICE_IDX], /* in_device       */
                                 properties,               /* properties      */
                                 0,                        /* num_devices     */
                                 NULL,                     /* out_devices     */
                                 &n_sub_devices);          /* num_devices_ret */
    assert(errcode == CL_SUCCESS);

    /* Create the sub-devices equally */
    errcode = clCreateSubDevices(devices[ROOT_DEVICE_IDX], /* in_device       */
                                 properties,               /* properties      */
                                 n_sub_devices,            /* num_devices     */
                                 &devices[SUB_DEVICE_IDX_START], /*out_devices*/
                                 &n_sub_devices);          /* num_devices_ret */
    assert(errcode == CL_SUCCESS);

    DebugPrint("Using CL_DEVICE_PARTITION_EQUALLY, %d sub-devices created\n",
               n_sub_devices);

    /*-------------------------------------------------------------------------
     * Initialize OpenCL objects
     *------------------------------------------------------------------------*/
    /* Create the context spanning the root device and its subdevices */
    cl_context context = clCreateContext(NULL,         /* properties  */
                                         n_devices,    /* num_devices */
                                         devices,      /* devices     */
                                         NULL,         /* pfn_notify  */
                                         NULL,         /* user_data   */
                                         &errcode);    /* errcode_ret */
    assert(errcode == CL_SUCCESS);

    /* Create separate command queues for each device */
    CreateCommandQueues(&context, devices, &Qs, n_devices);

    /* Create the buffers */
    cl_mem bufA, bufB, bufDst;
    CreateDeviceBuffer(&context, &bufA,  bufsize, srcA);
    CreateDeviceBuffer(&context, &bufB,  bufsize, srcB);
    CreateDeviceBuffer(&context, &bufDst, bufsize, dst);

    /* Create program from source, for all devices in context */
    cl_program program = clCreateProgramWithSource(context,                  /* context */
                                                   1,                        /* count   */
                                                   (const char**)&src_str,   /* strings */
                                                   (const cl_uint*)&src_size,/* lengths */
                                                   &errcode);                /* errcode_ret */
    assert(errcode == CL_SUCCESS);

    /* Build program for all devices associated with program and context */
    errcode = clBuildProgram(program, /* program     */
                             0,       /* num_devices */
                             NULL,    /* device_list */
                             NULL,    /* options     */
                             NULL,    /* pfn_notify  */
                             NULL);   /* user_data   */
    assert(errcode == CL_SUCCESS);

    /* Create kernel */
    cl_kernel kernel = clCreateKernel(program,     /* program */
                                      "VectorAdd", /* kernel_name */
                                      &errcode);   /* errcode_ret */
    assert(errcode == CL_SUCCESS);

    /* Set kernel args */
    SetKernelArguments(&kernel, &bufA, &bufB, &bufDst);

    /*-------------------------------------------------------------------------
     * Run Vecadd on root device and each sub device using different
     * command queues separately
     *------------------------------------------------------------------------*/
    for(auto i=0; i<n_devices; i++)
    {
        elapsed_times[i] = RunKernelOnQ(&Qs[i], &kernel,
                                        &bufA,  srcA,
                                        &bufB,  srcB,
                                        &bufDst, dst,
                                        Golden, NumElements);
        ResetResultArray(dst, NumElements);
    }

    /* Display timing data */
    ShowTimingData(elapsed_times, devices, n_devices);

    /*-------------------------------------------------------------------------
     * Clean Up
     *------------------------------------------------------------------------*/
    CleanUpQs(Qs, n_devices);
    CleanUpCLObjects(&kernel, &program, &context);
    CleanUpSubDevices(&devices[ROOT_DEVICE_IDX+1], n_sub_devices);
    CleanUpMemoryObjects(&bufA, &bufB, &bufDst,
                         srcA, srcB, dst, Golden);
    delete[] devices;
    delete[] elapsed_times;
    delete[] src_str;

    cout << "PASS!" << endl;
    return 0;
}

/* ======================================================================== */
/* Utility function definitions                                             */
/* ======================================================================== */

/*-------------------------------------------------------------------------
 * Get number of compute units in the given device
 *------------------------------------------------------------------------*/
cl_uint GetNumComputeUnits(cl_device_id* device)
{
    int errcode;
    cl_uint n_units;
    errcode = clGetDeviceInfo(*device,                    /* device              */
                              CL_DEVICE_MAX_COMPUTE_UNITS,/* param_name          */
                              sizeof(cl_uint),            /* param_value_size    */
                              &n_units,                   /* param_value         */
                              NULL);                      /* param_value_size_ret*/
    assert(errcode == CL_SUCCESS);
    return n_units;
}

/*-------------------------------------------------------------------------
 * Load the source code containing the kernel
 *------------------------------------------------------------------------*/
cl_uint LoadKernelSource(char* src_str)
{
    FILE* fp;
    cl_uint size;

    fp = fopen(fileName, "r");

    if (!fp)
    {
        cerr << "Failed to load kernel." << endl;
        exit(1);
    }

    size = fread(src_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);
    return size;
}

/*-------------------------------------------------------------------------
 * Create device buffer from given host buffer
 *------------------------------------------------------------------------*/
void CreateDeviceBuffer(cl_context* context,
                        cl_mem*     buf,
                        cl_uint     size,
                        cl_short*   src)
{
    int errcode;
    *buf = clCreateBuffer(*context,                              /* context     */
                          CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,/* flags       */
                          size,                                  /* size        */
                          src,                                   /* host_ptr    */
                          &errcode);                             /* errcode_ret */
    assert(errcode == CL_SUCCESS);
}

/*-------------------------------------------------------------------------
 * Allocate input and output arrays
 * Initialize input arrays with random numbers
 * Initialize output array to 0
 * Calculate correct results in array Golden for error checking
 *------------------------------------------------------------------------*/
void InitArrays(cl_short** srcA,
                cl_short** srcB,
                cl_short** Golden,
                cl_short** dst,
                cl_uint    n_elems)
{
    cl_uint bufsize = sizeof(cl_short) * n_elems;

    *srcA   = (cl_short*)__malloc_ddr(bufsize);
    *srcB   = (cl_short*)__malloc_ddr(bufsize);
    *dst    = (cl_short*)__malloc_ddr(bufsize);
    *Golden = (cl_short*)__malloc_ddr(bufsize);

    assert(*srcA   != nullptr &&
           *srcB   != nullptr &&
           *dst    != nullptr &&
           *Golden != nullptr);

    srand(time(NULL));
    for (auto i = 0; i < n_elems; ++i)
    {
        (*srcA)[i] = rand() % 100 + 1;
        (*srcB)[i] = rand() % 100 + 1;
        (*Golden)[i] = (*srcB)[i] + (*srcA)[i];
        (*dst)[i]    = 0;
    }
}

/*-------------------------------------------------------------------------
 * Enqueue NDRange Kernel and wait for it to finish on given queue
 *------------------------------------------------------------------------*/
void EnqueueKernelAndWaitForFinish(cl_command_queue* q,
                                   cl_kernel*        kernel,
                                   const cl_uint*    global_work_size,
                                   const cl_uint*    local_work_size)
{
    int errcode;
    cl_event ev = nullptr;
    errcode = clEnqueueNDRangeKernel(*q, 	          /* command_queue          */
                                     *kernel, 		  /* kernel                 */
                                     1, 			  /* work_dim               */
                                     nullptr, 		  /* global_work_offset     */
                                     global_work_size,/* global_work_size       */
                                     local_work_size, /* local_work_size        */
                                     0, 			  /* num_events_in_wait_list*/
                                     nullptr, 		  /* event_wait_list        */
                                     &ev);		      /* event                  */
    assert(errcode == CL_SUCCESS);
    assert(ev != nullptr);

    /* Wait for kernel to finish */
    errcode = clWaitForEvents(1, &ev);
    assert(errcode == CL_SUCCESS);
}

/*-------------------------------------------------------------------------
 * Set arguments on kernel
 *------------------------------------------------------------------------*/
void SetKernelArguments(cl_kernel* kernel,
                        cl_mem*    bufA,
                        cl_mem*    bufB,
                        cl_mem*    bufDst)
{
    int errcode;
    errcode = clSetKernelArg(*kernel,       /* kernel    */
                             0,             /* arg_index */
                             sizeof(cl_mem),/* arg_size  */
                             (void*)bufA);  /* arg_value */
    assert(errcode == CL_SUCCESS);
    errcode = clSetKernelArg(*kernel,       /* kernel    */
                             1,             /* arg_index */
                             sizeof(cl_mem),/* arg_size  */
                             (void*)bufB);  /* arg_value */
    assert(errcode == CL_SUCCESS);
    errcode = clSetKernelArg(*kernel,       /* kernel    */
                             2,             /* arg_index */
                             sizeof(cl_mem),/* arg_size  */
                             (void*)bufDst);/* arg_value */
    assert(errcode == CL_SUCCESS);
}

/*-------------------------------------------------------------------------
 * Create one command queue for each device in given list
 *------------------------------------------------------------------------*/
void CreateCommandQueues(cl_context*        context,
                         cl_device_id*      devices,
                         cl_command_queue** Qs,
                         cl_uint            num_Qs)
{
    int errcode;
    for (auto i = 0; i < num_Qs; i++)
    {
        (*Qs)[i] = clCreateCommandQueue(*context,  /* context     */
                                        devices[i],/* device      */
                                        0,         /* properties  */
                                        &errcode); /* errcode_ret */
        assert(errcode == CL_SUCCESS);
    }
}

/*-------------------------------------------------------------------------
 * Check if the results are correct compared to the golden array
 *------------------------------------------------------------------------*/
bool IsCorrect(cl_short* golden,
               cl_short* dst,
               cl_uint   n_elem)
{
    for (auto i = 0; i < n_elem; ++i)
    {
        if (golden[i] != dst[i])
        {
            cout << "Failed at Element " << i << ": "
                 << golden[i] << " != " << dst[i] << endl;
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------
 * Run kernel on given command queue
 *------------------------------------------------------------------------*/
double RunKernelOnQ(cl_command_queue* Q,
                    cl_kernel*        kernel,
                    cl_mem*           bufA,
                    cl_short*         srcA,
                    cl_mem*           bufB,
                    cl_short*         srcB,
                    cl_mem*           bufDst,
                    cl_short*         dst,
                    cl_short*         Golden,
                    cl_uint           n_elems)
{
    cl_uint bufsize = sizeof(cl_short) * n_elems;
    double secs = 0;
    tick();

    /* Enqueue NDRange Kernel and wait for it to finish */
    EnqueueKernelAndWaitForFinish(Q, kernel, &NumVecElements, &WorkGroupSize);

    secs = tock();

    /* Check correctness */
    if (!IsCorrect(Golden, dst, n_elems)) exit(1);

    return secs;
}

/*-------------------------------------------------------------------------
 * Reset the result array values to 0
 *------------------------------------------------------------------------*/
void ResetResultArray(cl_short* results, cl_uint n_elems)
{
    for (auto i = 0; i < n_elems; i++) results[i] = 0;
}

/*-------------------------------------------------------------------------
 * Print debug messages
 *------------------------------------------------------------------------*/
void DebugPrint(const char* fmt, ...)
{
    if (!DEBUG) return;
    std::string debug_fmt = "DEBUG: ";
    debug_fmt += fmt;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, debug_fmt.c_str(), ap);
    va_end(ap);
    std::fflush(stdout);
}

/*-------------------------------------------------------------------------
 * Finalize command queues and deallocate memory
 *------------------------------------------------------------------------*/
void CleanUpQs(cl_command_queue* Qs, cl_uint numQs)
{
    int errcode;
    for(auto i=0; i<numQs; i++)
    {
        errcode = clFlush(Qs[i]);
        assert(errcode == CL_SUCCESS);
        errcode = clFinish(Qs[i]);
        assert(errcode == CL_SUCCESS);
        errcode = clReleaseCommandQueue(Qs[i]);
        assert(errcode == CL_SUCCESS);
    }
    delete[] Qs;
}

/*-------------------------------------------------------------------------
 * Release OpenCL objects
 *------------------------------------------------------------------------*/
void CleanUpCLObjects(cl_kernel*  kernel,
                      cl_program* program,
                      cl_context* context)
{
    int errcode;
    errcode = clReleaseKernel(*kernel);
    assert(errcode == CL_SUCCESS);
    errcode = clReleaseProgram(*program);
    assert(errcode == CL_SUCCESS);
    errcode = clReleaseContext(*context);
    assert(errcode == CL_SUCCESS);
}

/*-------------------------------------------------------------------------
 * Release Sub Devices
 *------------------------------------------------------------------------*/
void CleanUpSubDevices(cl_device_id* subdevices, cl_uint num)
{
    int errcode;
    for (auto i = 0; i < num; i++)
    {
        errcode = clReleaseDevice(subdevices[i]);
        assert(errcode == CL_SUCCESS);
    }
}

/*-------------------------------------------------------------------------
 * Release and Deallocate buffers
 *------------------------------------------------------------------------*/
void CleanUpMemoryObjects(cl_mem*   bufA,
                          cl_mem*   bufB,
                          cl_mem*   bufDst,
                          cl_short* srcA,
                          cl_short* srcB,
                          cl_short* dst,
                          cl_short* Golden)
{
    int errcode;
    errcode = clReleaseMemObject(*bufA);
    assert(errcode == CL_SUCCESS);
    errcode = clReleaseMemObject(*bufB);
    assert(errcode == CL_SUCCESS);
    errcode = clReleaseMemObject(*bufDst);
    assert(errcode == CL_SUCCESS);
    __free_ddr(srcA);
    __free_ddr(srcB);
    __free_ddr(dst);
    __free_ddr(Golden);
}

/*-------------------------------------------------------------------------
 * Display timing data
 *------------------------------------------------------------------------*/
void ShowTimingData(double*       elapsed_times,
                    cl_device_id* devices,
                    cl_uint       n_devices)
{
    cout << "--------------------------------------------" << endl;
    cout << "Device [Num Compute Units]: Computation Time" << endl;
    cout << "--------------------------------------------" << endl;

    cout << "Root Device ["
         << GetNumComputeUnits(&devices[ROOT_DEVICE_IDX])
         << "]: "
         << elapsed_times[ROOT_DEVICE_IDX]
         << "s"
         << endl;

    for (auto i = SUB_DEVICE_IDX_START; i < n_devices; i++)
    {
        cout << "Sub Device"
             << i
             << " ["
             << GetNumComputeUnits(&devices[i])
             << "]: "
             << elapsed_times[i]
             << "s"
             << endl;
    }
    cout << "--------------------------------------------" << endl;
}
