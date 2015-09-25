*********************
Float compute example
*********************

This example computes ``y[i] = M[i] * x[i] + C`` on single precision floating point arrays with 2 million elements. It uses OpenCL to accelerate computation by dispatching an OpenCL NDRange kernel across the compute units (C66x cores) in the compute device. Refer :doc:`../intro` for details on the number of compute units in a compute device for various TI SoCs. The example illustrates the following OpenCL features:

-  Using \_\_malloc\_ddr, a TI OpenCL extension, to reduce the overhead
   of dispatch
-  Using local memory to improve performance of OpenCL C code
-  Using an OpenCL vector type, float2, to further improve performance

This example consists of 2 source files:

-  main.cpp with the OpenCL host code. The host code is written using
   the OpenCL C++ Wrapper API. Code written using the C++ API is easier
   to read because it uses try-catch exception handling to detect and
   report errors (vs. having to check the return code on every API call)
-  dsp\_compute.cl with OpenCL-C kernel code

The code in main.cpp consists of the following sections:

#. OpenCL setup - this is boiler-plate code that is used to initialize
   OpenCL data structures such as contexts, devices and command queues
#. Allocate arrays for input and output
#. Initialize inputs
#. Perform computation on ARM to get timing information on ARM and
   generate a "golden" array of output values
#. Perform computation on OpenCL compute device (DSP)
#. Check results by comparing DSP output against the ARM output
#. Report performance data on ARM and DSP

.. note::
   Steps 3 through 6 are repeated 5 times and the ARM and DSP times reported is the average across the runs

The code in dsp\_compute.cl consists of the following sections:

#. Copy input arrays from global memory to local memory
#. Perform the computation and store results to local memory
#. Copy output array from local memory to global memory

Host Code (main.cpp)
--------------------

OpenCL Setup
~~~~~~~~~~~~

-  Create an OpenCL context with device type
   CL\_DEVICE\_TYPE\_ACCELERATOR and query the context for devices. On
   the TI OpenCL implementation, CL\_DEVICE\_TYPE\_ACCELERATOR is a
   device consisting of one or more C66x DSP cores.

::

    Context context(CL_DEVICE_TYPE_ACCELERATOR);
    std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

-  Create an OpenCL program from the DSP binary

The DSP binary is built by the Makefile, refer ../make.inc for relevant
rules. ocl\_read\_binary is a utility function provided by TI's OpenCL
implementation. Refer :doc:`../compilation` for details.

::

    char *bin;
    int bin_length = ocl_read_binary("dsp_compute.out", bin);

    Program::Binaries   binary(1, std::make_pair(bin, bin_length));
    Program             program = Program(context, devices, binary);
    program.build(devices);

    delete [] bin;

Allocate arrays
~~~~~~~~~~~~~~~

This section allocates space for the input and output arrays.
\_\_malloc\_ddr is a TI OpenCL extension. Any memory region allocated
via \_\_malloc\_ddr is accessible to both the ARM and the DSP and the
OpenCL runtime performs the required cache operations to ensure
consistency.

::

    cl_float *M = (cl_float *)__malloc_ddr(sizeof(float) * NumElements);
    cl_float *x = (cl_float *)__malloc_ddr(sizeof(float) * NumElements);
    cl_float *y = (cl_float *)__malloc_ddr(sizeof(float) * NumElements);

Data synchronization between the host and compute device can be a
significant source of overhead. This overhead has implications for the
amount of computation that needs to be performed by OpenCL offload to
outweigh the data synchronization overhead. Host variables can span
multiple non-contiguous pages in Linux virtual memory whereas the OpenCL
device operates on contiguous physical memory. When mapping variables
from the Linux process space, the variables must be copied into
contiguous memory for device operation. This copy is inefficient,
especially for large variables. To eliminate the copy, TI's OpenCL
implementation provides a special purpose dynamic memory allocation API,
\_\_malloc\_ddr() and \_\_malloc\_ msmc(). The physical memory
associated with this heap is contiguous and is mapped to a contiguous
chunk of virtual memory on the host. If any host variables allocated via
this API are accessed on the device, the OpenCL runtime generates cache
management operations on the host, significantly reducing the overhead.
\_\_malloc\_msmc is available only on 66AK2x.

Compute on ARM
~~~~~~~~~~~~~~

The computation on ARM is parallelized across 2 threads using the OpenMP
``'parallel for'`` pragma.

::

    #pragma omp parallel for num_threads(2)
    for (int i=0; i < count; ++i)
        out[i] = in1[i] * in2[i] + C;

Compute on OpenCL device
~~~~~~~~~~~~~~~~~~~~~~~~

-  Create OpenCL buffers to pass as arguments to the kernel. The
   CL\_MEM\_USE\_HOST\_PTR flag indicates that the backing storage for
   the buffer is provided by the host pointer (specified as the last
   argument).

::

    const int BufSize = sizeof(float) * NumElements;
    Buffer bufM(context,CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,  BufSize, M);
    Buffer bufx(context,CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,  BufSize, x);
    Buffer bufy(context,CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR, BufSize, y);

-  Create a kernel object and set its arguments. The string
   "dsp\_compute" corresponds to the name of the kernel function defined
   in dsp\_compute.cl The kernel has been optimized to operate out of
   local memory. The required local buffers are passed as arguments to
   the kernel, one for each of the three arrays involved in the
   computation. The size of the local buffer is proportional to the
   number of work items in the work-group, specified by WorkGroupSize.

::

    Kernel kernel(program, "dsp_compute");
    kernel.setArg(0, bufM);
    kernel.setArg(1, bufx);
    kernel.setArg(2, C);
    kernel.setArg(3, bufy);
    kernel.setArg(4, __local(sizeof(cl_float2)*WorkGroupSize));
    kernel.setArg(5, __local(sizeof(cl_float2)*WorkGroupSize));
    kernel.setArg(6, __local(sizeof(cl_float2)*WorkGroupSize));

-  Dispatch the kernel and wait for it to complete

::

    Event ev1;
    Q.enqueueNDRangeKernel(kernel, NullRange, NDRange(NumVecElements),
                           NDRange(WorkGroupSize), NULL, &ev1);

    ev1.wait();


.. note::
    Since the kernel operates on float2 vectors, the global NDRange is the number of vector elements in the input.

OpenCL C kernel code (dsp\_compute.cl)
--------------------------------------

The kernel arguments are float2 vs. float because the C66x DSP core can perform single precision floating point multiply, add efficiently on float2 vectors.

Copy input arrays from global memory to local memory
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each work-group copies its chunk of input data from global memory to
local memory. The offset of the chunk in global memory is determined by
the work group id and the number of work items in the work group. This
copy is performed before any of the work items in the work group are
executed. The waits ensure the copies are complete before the local
arrays are used.

::

    int grp_id    = get_group_id(0);
    int num_elems = get_local_size(0);

    // Initiate copy of input arrays from global to local memory
    event_t ev1 = async_work_group_copy(lM, M+grp_id*num_elems, num_elems, 0);
    event_t ev2 = async_work_group_copy(lx, x+grp_id*num_elems, num_elems, 0);

    // Wait for copies to complete
    wait_group_events(1, &ev1);
    wait_group_events(1, &ev2);


.. note::
    The use of async\_work\_group\_copy is an optimization to take advantage of the lower latency and higher bandwidth of local memories.

Perform computation out of local memory
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note that local ids are used to index the arrays in local memory.

.. code:: c

    int lid    = get_local_id(0);
    ly[lid] = lx[lid] * lM[lid] + C;

Copy output array from local memory to global memory
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The last step is to copy the result array from local memory to the
appropriate chunk of global memory and wait for the copy to complete.

.. code:: c

    event_t ev3 = async_work_group_copy(y+grp_id*num_elems, ly, num_elems, 0);
    wait_group_events(1, &ev3);

Sample Output
-------------

::

    ./float_compute 

    This example computes y[i] = M[i] * x[i] + C on a single precision float point 
    array of size 2097152
     Computation on the ARM is parallelized across the 2 A15s using OpenMP.
     Computation on the DSP is performed by dispatching an OpenCL NDRange kernel 
       across the 2 compute units (C66x cores) in the compute device.
    .....

    Average across 5 runs: 
    ARM (2 OpenMP threads)         : 0.016421 secs
    DSP (OpenCL NDRange kernel)    : 0.009904 secs
    OpenCL-DSP speedup             : 1.657894


    For more information on:
      * TI's OpenCL product, http://processors.wiki.ti.com/index.php/OpenCL
      * This and other OpenCL examples, http://processors.wiki.ti.com/index.php/OpenCL_Examples

