#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "CL/opencl.h"
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

// Host buffers for demo
// *********************************************************************
void *srcA, *srcB, *dst;        // Host buffers for OpenCL test
void* Golden;                   // Host buffer for host golden processing cross check

// OpenCL Vars
cl_context cxGPUContext;        // OpenCL context
cl_command_queue cqCommandQueue;// OpenCL command que
cl_platform_id cpPlatform;      // OpenCL platform
cl_device_id cdDevice;          // OpenCL device
cl_program cpProgram;           // OpenCL program
cl_kernel ckKernel;             // OpenCL kernel
cl_mem cmDevSrcA;               // OpenCL device source buffer A
cl_mem cmDevSrcB;               // OpenCL device source buffer B 
cl_mem cmDevDst;                // OpenCL device destination buffer 
size_t szGlobalWorkSize;        // 1D var for Total # of work items
size_t szLocalWorkSize;		    // 1D var for # of work items in the work group	
size_t szParmDataBytes;			// Byte size of context information
size_t szKernelLength;			// Byte size of kernel code
cl_int ciErr1, ciErr2;			// Error code var
const char* cExecutableName = NULL;
const char* cSourceCL =
    "__kernel void VectorAdd(__global const float* a, "
    "                        __global const float* b, "
    "                        __global float* c, "
    "                        int iNumElements)"
    "{"
    "    int id = get_global_id(0);"
    "    if (id >= iNumElements) return; "
    "    c[id] = a[id] + b[id];"
    "}";

// demo config vars
int iNumElements = 11444777;	// Length of float arrays to process (odd # for illustration)

// Forward Declarations
// *********************************************************************
void shrFillArray(float* src, int iNumElements);
bool shrComparefet(const float* Golden, const float* dst, int iNumElements, float epsilon);
int shrRoundUp(int size, int val);
void VectorAddHost(const float* pfData1, const float* pfData2, float* pfResult, int iNumElements);
void Cleanup (int argc, char **argv, int iExitCode);

// Main function 
// *********************************************************************
int main(int argc, char **argv)
{
    // start logs 
	cExecutableName = argv[0];
    printf("%s Starting...\n\n# of float elements per Array \t= %i\n", argv[0], iNumElements); 

    // set and log Global and Local work size dimensions
    szLocalWorkSize = 256;
    szGlobalWorkSize = shrRoundUp((int)szLocalWorkSize, iNumElements);  // rounded up to the nearest multiple of the LocalWorkSize
    printf("Global Work Size \t\t= %u\nLocal Work Size \t\t= %u\n# of Work Groups \t\t= %u\n\n", 
           szGlobalWorkSize, szLocalWorkSize, (szGlobalWorkSize % szLocalWorkSize + szGlobalWorkSize/szLocalWorkSize)); 

    // Allocate and initialize host arrays 
    printf( "Allocate and Init Host Mem...\n"); 
    srcA = (void *)malloc(sizeof(cl_float) * szGlobalWorkSize);
    srcB = (void *)malloc(sizeof(cl_float) * szGlobalWorkSize);
    dst = (void *)malloc(sizeof(cl_float) * szGlobalWorkSize);
    Golden = (void *)malloc(sizeof(cl_float) * iNumElements);
    shrFillArray((float*)srcA, iNumElements);
    shrFillArray((float*)srcB, iNumElements);

    //Get an OpenCL platform
    ciErr1 = clGetPlatformIDs(1, &cpPlatform, NULL);

    printf("clGetPlatformID...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clGetPlatformID, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(argc, argv, EXIT_FAILURE);
    }

    //Get the devices
    ciErr1 = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_CPU, 1, &cdDevice, NULL);
    printf("clGetDeviceIDs...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clGetDeviceIDs, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(argc, argv, EXIT_FAILURE);
    }

    //Create the context
    cxGPUContext = clCreateContext(0, 1, &cdDevice, NULL, NULL, &ciErr1);
    printf("clCreateContext...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clCreateContext, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Create a command-queue
    cqCommandQueue = clCreateCommandQueue(cxGPUContext, cdDevice, 0, &ciErr1);
    printf("clCreateCommandQueue...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clCreateCommandQueue, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Allocate the OpenCL buffer memory objects for source and result on the device GMEM
    cmDevSrcA = clCreateBuffer(cxGPUContext, CL_MEM_READ_ONLY, sizeof(cl_float) * szGlobalWorkSize, NULL, &ciErr1);
    cmDevSrcB = clCreateBuffer(cxGPUContext, CL_MEM_READ_ONLY, sizeof(cl_float) * szGlobalWorkSize, NULL, &ciErr2);
    ciErr1 |= ciErr2;
    cmDevDst = clCreateBuffer(cxGPUContext, CL_MEM_WRITE_ONLY, sizeof(cl_float) * szGlobalWorkSize, NULL, &ciErr2);
    ciErr1 |= ciErr2;
    printf("clCreateBuffer...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clCreateBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(argc, argv, EXIT_FAILURE);
    }
    
    // Create the program
    cpProgram = clCreateProgramWithSource(cxGPUContext, 1, (const char **)&cSourceCL, &szKernelLength, &ciErr1);
    printf("clCreateProgramWithSource...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clCreateProgramWithSource, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(argc, argv, EXIT_FAILURE);
    }

    ciErr1 = clBuildProgram(cpProgram, 1, &cdDevice, NULL, NULL, NULL);
    // ASW FIX ciErr1 = clBuildProgram(cpProgram, 0, NULL, NULL, NULL, NULL);
    printf("clBuildProgram...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clBuildProgram, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Create the kernel
    ckKernel = clCreateKernel(cpProgram, "VectorAdd", &ciErr1);
    printf("clCreateKernel (VectorAdd)...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clCreateKernel, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Set the Argument values
    ciErr1 = clSetKernelArg(ckKernel, 0, sizeof(cl_mem), (void*)&cmDevSrcA);
    ciErr1 |= clSetKernelArg(ckKernel, 1, sizeof(cl_mem), (void*)&cmDevSrcB);
    ciErr1 |= clSetKernelArg(ckKernel, 2, sizeof(cl_mem), (void*)&cmDevDst);
    ciErr1 |= clSetKernelArg(ckKernel, 3, sizeof(cl_int), (void*)&iNumElements);
    printf("clSetKernelArg 0 - 3...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clSetKernelArg, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(argc, argv, EXIT_FAILURE);
    }

    // --------------------------------------------------------
    // Start Core sequence... copy input data to GPU, compute, copy results back

    // Asynchronous write of data to GPU device
    ciErr1 = clEnqueueWriteBuffer(cqCommandQueue, cmDevSrcA, CL_FALSE, 0, sizeof(cl_float) * szGlobalWorkSize, srcA, 0, NULL, NULL);
    ciErr1 |= clEnqueueWriteBuffer(cqCommandQueue, cmDevSrcB, CL_FALSE, 0, sizeof(cl_float) * szGlobalWorkSize, srcB, 0, NULL, NULL);
    printf("clEnqueueWriteBuffer (SrcA and SrcB)...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Launch kernel
    ciErr1 = clEnqueueNDRangeKernel(cqCommandQueue, ckKernel, 1, NULL, &szGlobalWorkSize, &szLocalWorkSize, 0, NULL, NULL);
    printf("clEnqueueNDRangeKernel (VectorAdd)...\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clEnqueueNDRangeKernel, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(argc, argv, EXIT_FAILURE);
    }

    // Synchronous/blocking read of results, and check accumulated errors
    ciErr1 = clEnqueueReadBuffer(cqCommandQueue, cmDevDst, CL_TRUE, 0, sizeof(cl_float) * szGlobalWorkSize, dst, 0, NULL, NULL);

    printf("clEnqueueReadBuffer (Dst)...\n\n"); 
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clEnqueueReadBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(argc, argv, EXIT_FAILURE);
    }
    //--------------------------------------------------------

    // Compute and compare results for golden-host and report errors and pass/fail
    printf("Comparing against Host/C++ computation...\n\n"); 
    VectorAddHost ((const float*)srcA, (const float*)srcB, (float*)Golden, iNumElements);
    bool bMatch = shrComparefet((const float*)Golden, (const float*)dst, iNumElements, 0.0f);

    if (bMatch)
        printf("Passed!\n");
    else
        printf("Failed!\n");

    // Cleanup and leave
    Cleanup (argc, argv, (bMatch == true) ? EXIT_SUCCESS : EXIT_FAILURE);
}

void Cleanup (int argc, char **argv, int iExitCode)
{
    // Cleanup allocated objects
    printf("Starting Cleanup...\n\n");

    if (ckKernel)      clReleaseKernel       (ckKernel);  
    if (cpProgram)     clReleaseProgram      (cpProgram);
    if (cqCommandQueue)clReleaseCommandQueue (cqCommandQueue);
    if (cmDevSrcA)     clReleaseMemObject    (cmDevSrcA);
    if (cmDevSrcB)     clReleaseMemObject    (cmDevSrcB);
    if (cmDevDst)      clReleaseMemObject    (cmDevDst);
    if (cxGPUContext)  clReleaseContext      (cxGPUContext);

    // Free host memory
    free (srcA); 
    free (srcB);
    free (dst);
    free (Golden);
    printf("Leaving Cleanup!\n");
}

// "Golden" Host processing vector addition function for comparison purposes
// *********************************************************************
void VectorAddHost(const float* pfData1, const float* pfData2, float* pfResult, int iNumElements)
{
    int i;
    for (i = 0; i < iNumElements; i++) 
    {
        pfResult[i] = pfData1[i] + pfData2[i]; 
    }
}

bool shrComparefet(const float* Golden, const float* dst, int iNumElements, float epsilon)
{
    int i;
    for (i=0; i< iNumElements; ++i)
        if (Golden[i] - dst[i] > epsilon) return false;
    return true;
}

void shrFillArray(float* src, int iNumElements)
{
    int i;
    for (i=0; i< iNumElements; ++i)
        src[i] = i;
}

int shrRoundUp(int size, int val)
{
    return (((val + size - 1) / size) * size);
}
