******************************************************
Understanding OpenCL Memory Usage
******************************************************

Device Memory 
=====================================================
The following device components are relevant to the memory discussion for
OpenCL.

+----------------------------------+-----------+----------------+
| Attribute                        | 66AK2H    | AM57           |
+==================================+===========+================+
| ARM A15 CPU cores                | 4         | 2              |
+----------------------------------+-----------+----------------+
| C66 DSP cores                    | 8         | 2              |
+----------------------------------+-----------+----------------+
| L1P per C66 core                 | 32KB      | 32KB           |
+----------------------------------+-----------+----------------+
| L1D per C66 core                 | 32KB      | 32KB           |
+----------------------------------+-----------+----------------+
| L2 cache shared across ARM cores | 4MB       | 2MB            |
+----------------------------------+-----------+----------------+
| L2 memory per C66 core           | 1MB       | 288KB          |
+----------------------------------+-----------+----------------+
| DDR3 available                   | up to 8GB | 2GB            |
+----------------------------------+-----------+----------------+
| On-chip shared memory            | 6MB       | None Available |
+----------------------------------+-----------+----------------+

The L1 and L2 memory areas in the C66 cores can be configured as all cache, all
scratchpad or partitioned with both. For OpenCL applications, this partition is 
fixed as follows for each C66 core:

+---------------------------------------+--------+-------+
| Attribute                             | 66AK2H | AM57  |
+=======================================+========+=======+
| L1P cache                             | 32KB   | 32KB  |
+---------------------------------------+--------+-------+
| L1D cache                             | 32KB   | 32KB  |
+---------------------------------------+--------+-------+
| L2 cache                              | 128KB  | 128KB |
+---------------------------------------+--------+-------+
| L2 reserved                           | 128KB  | 32KB  |
+---------------------------------------+--------+-------+
| L2 available for OpenCL local buffers | 768KB  | 128KB |
+---------------------------------------+--------+-------+

.. Note::
    The amount of Local, MSMC and DDR that is available for OpenCL use
    may change from release to release.  The amount available for OpenCL use
    can be queried from the OpenCL runtime.  This is illustrated in the
    platform example shipped with the product:
    ``/usr/share/ti/examples/opencl/platforms``.

How DDR3 is Partitioned for Linux System and OpenCL
=====================================================

The DDR3 is partitioned into three components:

1. Linux system memory,
2. CMEM contiguous memory, and
3. Reserved memory.

The Linux system memory is the underlying memory store for the Linux virtual
memory system and would contain standard items like:

- A15 stacks,
- A15 heaps,
- A15 application code,
- A15 application variables, etc.

The CMEM contiguous memory is controlled by a Linux kernel module that
guarantees contiguous virtual addresses within a range are mapped to
contiguous physical addresses within the range. This is required for
buffer communication between the A15 and C66 cores, because the C66 cores
do not access memory through a shared MMU with the A15 CPUs and thus require
that buffers be allocated in contiguous physical memory. The CMEM memory
areas will be managed by OpenCL for allocation to OpenCL buffers and OpenCL C
programs.

The reserved memory is a very small portion of the DDR3 memory that is used in
the OpenCL implementation and is exposed to neither CMEM nor Linux.

66AK2H
--------------------------------------------------

The 8GB of attached DDR3 memory is accessible to the K2H device through a
64-bit bus. The 8GB of DDR3 is populated in the K2H 36-bit address space at
locations 8:0000:0000 through 9:FFFF:FFFF.

The first 2GB of DDR3 are fixed in usage to the following:

========================== ===================
Memory Range               Usage
========================== ===================
8:0000:0000 - 8:1FFF:FFFF  512M Linux System
8:2000:0000 - 8:22FF:FFFF  48M Reserved
8:2300:0000 - 8:7FFF:FFFF  1488M CMEM
========================== ===================

The remaining 6GB of DDR3 can be split between Linux and CMEM using boot time
variables. The default partition of the remaining 6GB would be:

========================== ===================
Memory Range               Usage
========================== ===================
8:8000:0000 - 8:BFFF:FFFF  1GB Linux System
8:C000:0000 - 9:FFFF:FFFF  5GB CMEM
========================== ===================

You can verify the partition in your system by viewing the ``/proc/iomem``
system file. The bottom of this file will contain the external DDR memory map,
for example::

    800000000-81fffffff : System RAM
      800008000-8006b5277 : Kernel code
      8006fa000-8007ace53 : Kernel data
    823000000-87fffffff : CMEM
    8 80000000-8bfffffff : System RAM
    8c0000000-9ffffffff : CMEM

The default partition of 1.5GB Linux system memory and 6.48GB CMEM provides a
minimum suggested Linux system memory size and a larger area for OpenCL buffer
and program space.<br>

.. Note::
    The m800 K2H system ships with 8GB of DDR3. The K2H EVM ships with
    2GB of DDR3 and can be upgraded to 8GB by replacing the DIMM

AM57
--------------------------------------------------
The 2GB of attached DDR3 memory is accessible to the AM57 device through a
32-bit bus. The 2GB of DDR3 is populated in the 32-bit address space at
locations 8000:0000 through FFFF:FFFF. The default partition is 1.5GB for Linux
system memory and 512MB of CMEM. ::

    80000000-9fffffff : System RAM
      80008000-808470b3 : Kernel code
      808a0000-80945a8b : Kernel data
    a0000000-bfffffff : CMEM
    c0000000-ffdfffff : System RAM
    fff00000-ffffefff : System RAM

The holes in System RAM ranges (e.g. FFE0:0000 to FFEF:FFFF, 1MB and FFFF:F000
to FFFF:FFFF, 4KB) are reserved memory ranges.

The OpenCL Memory Model
=====================================================

The OpenCL 1.1 specification
http://www.khronos.org/registry/cl/specs/opencl-1.1.pdf available from Khronos
defines a memory model in Section 3.3.  Please refer to the specification for
details on these memory regions and how they relate to work-items, work-groups,
and kernels. This document will focus on the mapping of the OpenCL memory model
to TI devices. There are four virtual memory regions defined.

Global Memory 
  This memory region contains global buffers and is the primary conduit for
  data transfers from the host A15 CPUs to/from the C66 DSPs. This region will
  also contain OpenCL C program code that will be executed on the C66 DSPs.
  For this OpenCL implementation, global memory by default maps to the portion
  of DDR3 partitioned as CMEM contiguous memory.  

  On K2H devices, MSMC memory is also available as global memory and buffers
  can be defined to reside in this memory instead of DDR3 through an OpenCL API
  extension specfic to TI. This mechanism will be described in a later section
  that details handling of the OpenCL buffer creation flags.  

Constant Memory
  This memory region contains content that remains constant during the
  execution of a kernel.  OpenCL C program code and constant data defined in
  that code would be placed in this region.  For this implementation, constant
  memory is mapped to the portion of DDR3 partitioned as CMEM contiguous
  memory.  

Local Memory 
  The local memory region is not defined by the spec to be accessible from the
  host (ARM A15 cores). This memory is local to a work group.  It can be viewed
  as a core local scratchpad memory and in fact for this implementation it is
  mapped to L2 that is reserved for this purpose.  The use case for local
  memory is for an OpenCL work-group to migrate a portion of a global buffer
  to/from a local buffer for performance reasons.  This use case is optional
  for users as access to global buffers in DDR will be cached in both the L2
  cache and the L1D cache on the C66 DSPs.  However, performance can often be
  improved by taking the extra step in OpenCL C programs to manage local memory
  as a scratchpad.  

Private Memory 
  This memory region is for values that are private to a work-item and these
  values are typically allocated to registers in the C66 DSP core.  Sometimes
  it may be necessary for these values to exist in memory.  In these cases the
  values are stored on the C66 DSP stack which resides in the reserved portion
  of the L2 memory.

OpenCL Buffers
=====================================================

Global Buffers
--------------------------------------------------

OpenCL global buffers are the conduit through which data is communicated from
the host application to OpenCL C kernels running on the C66 DSP.  The C
prototype for the OpenCL API function that creates global buffers is::

    cl_mem clCreateBuffer (cl_context context, cl_mem_flags flags, size_t size,
                           void *host_ptr, cl_int *errcode_ret);

The C++ binding for OpenCL specifies a Buffer object and the constructor for
that object has the following prototype::

    Buffer(const Context& context, cl_mem_flags flags, size_t size,
           void* host_ptr = NULL, cl_int* err = NULL);

For the remainder of this section on OpenCL Buffers, the examples will use the
C++ binding and the Buffer constructor. Conversion to the C API is straight
forward as the arguments to both methods are the same. The C++ Buffer
constructor does have default values of NULL for host_ptr and err, so in
examples where those arguments are not specified, conversion to the C API will
require adding NULL arguments in those parameter slots.

Also for the remainder of this section we will assume an OpenCL context named
``ctx`` has been created with only the DSP's present in the context. The
C++ code to create such a context is::

    Context ctx(CL_DEVICE_TYPE_ACCELERATOR);

Note that the device type ``CL_DEVICE_TYPE_ACCELERATOR`` is used in the context
constructor. In this OpenCL implementation accelerator equates to DSP.

With the context parameter now fixed to the context ``ctx``, and default
parameters of NULL for host_ptr and err, buffer creation is dependent on the
flags argument and the size argument. The size argument is relatively
straightforward as well. It should always be specified and represents the size
**in bytes** of the buffer. 

.. Note::
    The size of a buffer is in bytes. It is a frequent error to attempt to
    specify the size in number of elements. For example, if a buffer of 100
    ints is required, you need to pass in 400 or sizeof(int)*100 as the size
    and not just 100.

The flags argument defines some important properties for the buffer. Section
5.2.1 in the OpenCL 1.1 spec defines the flag values. They are also listed
below with their significance to this implementation. In general the flag
values may be or'ed together to create buffers with a combination of
properties. The OpenCL 1.1 spec enumerates the cases of mutually exclusive
buffer creation flags.

The flags are:

CL_MEM_READ_WRITE
  The OpenCL Kernels will both read and write the buffer

CL_MEM_WRITE_ONLY
  The OpenCL Kernels will only read the buffer

CL_MEM_READ_ONLY
  The OpenCL Kernels will only write the buffer

  The above three flags are mutually exclusive. Only one should be specified.
  If none are specified then CL_MEM_READ_WRITE is assumed. These flags indicate
  to the OpenCL runtime how the buffer will be accessed from the perspective of
  OpenCL C kernels running on the DSP.  These flags are used to control cache 
  coherency operations that the OpenCL runtime performs for you. 
  The ARM A15 devices are not cache coherent with the C66
  DSPs, so the OpenCL runtime will issue cache coherency operations between the
  writing of a buffer on one device and the reading of the buffer on a
  different device. When read only or write only is specified, some coherency
  operations may be skipped for performance.

CL_MEM_USE_HOST_PTR
  If using this buffer creation flag, a non-NULL host_ptr argument must also be
  provided. This flag indicates to the OpenCL runtime that the underlying
  memory store for this buffer object should be the memory area pointed to by
  the host_ptr argument. ::

    int size = 1024 *sizeof(int);
    int *p = (int*) malloc(size);
    Buffer buf(ctx, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, size, p);

  The above code fragment will allocate an area sized for 1K int values in the
  A15 Linux heap. It will then create a buffer using the
  ``CL_MEM_USE_HOST_PTR`` flag and will pass in the address of the heap
  area as the host_ptr. The result will be an OpenCL buffer whose underlying
  memory will be on the Linux heap at address p.

  Recall from the previous section that the DSP cannot reliably read from Linux
  system memory because it can be paged and non-contiguous. The DSP requires a
  contiguous buffer and so when a buffer created with this flag is passed to an
  OpenCL C kernel, it will require an area of CMEM memory to be allocated, a
  copy from the host heap memory into the CMEM area, a dispatch of the kernel
  and copy from CMEM back to the host heap memory. This is clearly not ideal
  from a performance perspective since there are multiple memory copies
  involved and they are just in time before kernel dispatch and just after
  kernel dispatch and may lengthen a critical path involving the kernel
  invocation. Other buffer creation flags and OpenCL API calls can eliminate
  both of these performance drawbacks.

  The benefit of using this flag is it can simplify the OpenCL API calls in
  your program. You would not need to explicitly read/write the buffer, nor
  explicitly map/unmap the buffer. You could write code in the manner shown
  below::

    for (i = 0; i < 1024; ++i) p[i] = ...
    foo(buf).wait();
    for (i = 0; i < 1024; ++i) ... = p[i];

  The above uses a previously defined C++ kernel functor named foo to enqueue a
  kernel using buf as an argument. Please see the OpenCL C++ binding specification
  http://www.khronos.org/registry/cl/specs/opencl-cplusplus-1.1.pdf
  for details on OpenCL kernel functors. For the purposes of
  this example, it enqueues a kernel with the buffer buf as an argument and
  then it waits for completion of the kernel. It is recommended that this flag
  not be used for performance critical OpenCL code. Although, as you can see it
  does simplify the API calls and can be used for prototyping.

CL_MEM_ALLOC_HOST_PTR
  A host_ptr argument is not necessary for buffers created with this creation
  flag. The default NULL value is valid. This flag is mutually exclusive with
  CL_MEM_USE_HOST_PTR. This flag indicates that OpenCL should allocate an
  underlying memory store for the buffer than can be accessed from the host.
  For this implementation, a buffer created with this flag is allocated memory
  in the CMEM contiguous memory region and can be accessed directly from both
  the host A15 and the C66 DSPs. This flag is recommended for for performance
  in buffer handling. It is also the default flag if none of
  CL_MEM_USE_HOST_PTR, CL_MEM_ALLOC_HOST_PTR or CL_MEM_COPY_HOST_PTR is
  specified in the creation API. Buffers of this type can be used with the read
  and write buffer OpenCL APIs or they can be used with the map and unmap APIs
  for zero copy operation. The read/write and map/unmap APIs will be described
  later in this section.

CL_MEM_COPY_HOST_PTR
  A host_ptr argument is required for this buffer creation flag. This creation
  flag is identical to the CL_MEM_ALLOC_HOST_PTR flag in allocation and usage.
  The only difference is that on creation (or at least before first use) of a
  buffer with this flag, the memory pointed to by the argument host_ptr is used
  to initialize the underlying memory store for the buffer which will be in
  CMEM contiguous memory.

CL_MEM_USE_MSMC_TI
  This flag is a TI extension to standard OpenCL on 66AK2H devices only. It can
  be used in combination with the other buffer creation flags, except for
  CL_MEM_USE_HOST_PTR. When this flag is used, the buffer will be allocated to
  a CMEM block in the MSMC memory area, rather than a CMEM block in the DDR3
  area. The MSMC area available for OpenCL buffers is limited, so use of this
  flag must be judicial. However, in most circumstances the DSP can access MSMC
  buffers significantly faster than DDR buffers. This flag only affects the
  underlying memory store used for the buffer. It will still be considered a
  global buffer and can be used anywhere a global buffer can be used.

Global buffers can contain persistent data from one kernel invocation to the
next kernel invocation. It is possible for OpenCL C kernels to communicate data
between them in time by simply having kernel 1 produce data and kernel 2
consume data all on the C66 DSP. Other than creating the buffer through which
the communication will occur and sequencing the kernel enqueues, it is not
necessary for the host A15 to be involved in that data communication from
kernel 1 to kernel 2, i.e. the A15 does not need to read the data from kernel 1
and transfer it to kernel 2, the data can simply persist on the C66 DSP.

Local Buffers
--------------------------------------------------

Local buffers are quite different than global buffers. You cannot access local
buffers from the host and you do not create them using API's like global
buffers. Local buffers will be allocated from local memory which in this
implementation exists in the L2 scratchpad memory on the C66 DSP cores. Data
cannot persist from kernel to kernel in a local buffer. The lifetime of a local
buffer is the same as the dynamic lifetime of the kernel execution. Local
buffers are never required to be used, but are often used in OpenCL C kernels
for potential performance improvement. The typical use case for local buffers
in a kernel that is passed a global buffer, is for the local buffer to be used
explicitly by the user's OpenCL C kernel as a fast scratchpad memory for the
larger and slower global buffer. This scratchpad memory would be managed by the
user using asynchronous built-in functions to move the data between the global
and local buffers. Again local buffers are never required and the OpenCL C
kernel can depend on the C66 DSP cache to alleviate DDR access delay rather
than use local buffers. However, it is often the case that manual data movement
to/from local buffers can be advantageous to performance.

Local buffers can be defined in two ways. The first way is to simply define an
array in your OpenCL C kernel that is defined with the local keyword. For
example, the following OpenCL C kernel defines a local buffer named scratch and
then calls the async_work_group_copy builtin function to copy 100 char values
from the passed in global buffer to the local buffer.  The limitation to this
method, is that the local buffers are statically sized, in this case to 100
chars. ::

    kernel void foo(global char *buf)
    {
        local char scratch[100];
        async_work_group_copy(scratch, buf, 100, 0);
        ...
    }

Alternatively, local buffers can be passed to OpenCL C kernels as an argument
and can be sized dynamically. In this method you simply define your OpenCL C
kernel with a local buffer argument. For example::

    kernel void foo(global char *buf, local char *scratch)
    {
        async_work_group_copy(scratch, buf, 100, 0);
        ...
    }

and then from the host side you setup an argument to the local buffer by
passing a null pointer and a size to the clSetKernelArg function.

The OpenCL API for setting an argument to a kernel has the following prototype::

    cl_int clSetKernelArg (cl_kernel kernel, cl_uint arg_index, size_t arg_size, 
                           const void *arg_value);

To setup the 1st argument to the kernel foo with a global buffer, the API call
would look like::

    cl_mem buf = clCreateBuffer(...);
    clSetKernelArg(foo, 0, sizeof(buf), &buf);

To setup the 2nd argument to kernel foo with a local buffer, the API call would
look like::

    clSetKernelArg(foo, 1, 100, NULL);

The OpenCL runtime will interpret the size and null pointer passed to
clSetKernelArg as a local buffer and will temporarily allocate an area of local
memory (L2 in this implementation) of that size and will pass a pointer to that
area rather as the local buffer argument.

If the host code is using the C++ bindings then the previous two code boxes
combined would look like::

    Buffer buf(...);
    foo.setArg(0, buf);
    foo.setArg(1, __local(100));

In the C++ case, the __local() object is used to indicate a global buffer of
size 100 bytes.

Sub-Buffers
--------------------------------------------------

OpenCL Sub-Buffers are aliases to existing OpenCL global Buffers. Creating a
sub-buffer does not result in any underlying memory store allocation above what
is already required for the aliased buffer.  There are two primary use cases
for sub-buffers:

1. Accessing a buffer with different access flags than were specified in buffer
   creation, or 
2. Accessing a subset of a buffer.

The C++ API's for creating SubBuffers are described below. Please see the
OpenCL 1.1 specification or the OpenCL 1.1 Online Reference
http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/ 
for the sytax of the C API for sub-buffer creation.::

    typedef struct _cl_buffer_region { size_t origin; size_t size;} cl_buffer_region;

    Buffer createSubBuffer(cl_mem_flags flags, cl_buffer_create_type buffer_create_type,
                           const void * buffer_create_info, cl_int * err = NULL);

createSubBuffer is a member function of the OpenCL C++ Buffer object. The flags
argument should be one of ``CL_MEM_READ_WRITE, CL_MEM_READ_ONLY,
CL_MEM_WRITE_ONLY``. The buffer_create_type should be
``CL_BUFFER_CREATE_TYPE_REGION``. That is the only cl_buffer_create_type
supported in OpenCL 1.1. The buffer_create_info argument should be a pointer to
a cl_buffer_region structure, in which you define the buffer subset for the
sub-buffer. Usage of these API's may look like::

    Buffer buf(ctx, CL_MEM_READ_WRITE, bufsize);
    cl_buffer_region rgn = {0, bufsize};

    Buffer buf_rd = buf.createSubBuffer(CL_MEM_READ_ONLY,  CL_BUFFER_CREATE_TYPE_REGION, &rgn);
    Buffer buf_wt = buf.createSubBuffer(CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &rgn);

The prior subsection indicated that global buffers can be persistent from one
kernel invocation to the next. It is a common use case that kernel K1 only
writes a buffer and kernel K2 only reads the buffer. The buffer must be created
with the CL_MEM_READ_WRITE access flag, because the buffer is being both read
and written by OpenCL C kernels running on the C66 DSPs. However, no individual
kernel is both reading and writing the buffer, so the CL_MEM_READ_WRITE
property that the buffer has, may result in underlying cache coherency
operations that are unneccessary. For performace reasons, sub-buffers can be
used to specify more restrictive buffer access flags and they can be customized
for the behavior of the particular kernel to which the buffer is being passed
as an argument. The above illustration on SubBuffer creation is the setup for
this process. A Buffer buf has been defined as both read/write and two
sub-buffer aliases have been setup; one as read only and the other as write
only. These new sub-buffers may then be passed to kernels K1 and K2 instead of
the buffer buf directly. This process will ensure that the OpenCL runtime does
not perform any unnecessary cache coherency operations.

The other use case for sub-buffers is to create an object representing a subset
of a buffer. For example, it may be desirable to process a buffer in chunks.
Sub-buffers can be used to achieve those chunks in a form suitable for
arguments to OpenCL C Kernels. Assuming an OpenCL queue named Q and a Kernel
name K are already setup, the following code would result in K being dispatched
twice, once with the first half of Buffer buf and again with the second half of
Buffer buf.::

    Buffer bufA(ctx, CL_MEM_READ_ONLY,  bufsize);

    cl_buffer_region rgn_half1 = {0,         bufsize/2};
    cl_buffer_region rgn_half2 = {bufsize/2, bufsize/2};

    Buffer buf_half1 = buf.createSubBuffer(CL_MEM_READ_ONLY, 
                                           CL_BUFFER_CREATE_TYPE_REGION, &rgn_half1);
    Buffer buf_half2 = buf.createSubBuffer(CL_MEM_READ_ONLY, 
                                           CL_BUFFER_CREATE_TYPE_REGION, &rgn_half2);

    K.setArg(0, buf_half1);
    Q.enqueueTask(K);

    K.setArg(0, buf_half2);
    Q.enqueueTask(K);


Buffer Read/Write vs. Map/Unmap
=====================================================

The OpenCL APIs support two mechanisms for the host application to interact
with OpenCL buffers. They can:

- Read and write buffers using clEnqueueReadBuffer and clEnqueueWriteBuffer in
  C or the member functions enqueueReadBuffer and enqueueWriteBuffer in C++, or
- Map and unmap using clEnqueueMapBuffer and clEnqueueUnmapMemObject in C.

The read and write API's imply a movement of data to and from OpenCL buffers.
This typically means a movement of data from linux system memory to CMEM memory
where an OpenCL buffer typically resides.

The map/unmap API's map the underlying memory store of a buffer into the host
address space and allows the host application to read and write directly
from/to the buffer's content.  This method has the advantages of:

1. Not requiring 2 storage areas containing the same data (one in Linux system
   memory and one in the Buffer in CMEM memory), and 
2. Not requiring extra data movement between the two storage areas.

There are situations where the read/write buffer is preferable, however.  For
smaller buffers, the overhead of the extra copies is small and the extra
commands enqueued to the CommandQueue do have some overhead.  In the below
examples of read/write and map/unmap use, you can see that there are 4 commands
enqueued for data movement in the map/unmap case and there are only two
commands enqueued for data movement in the read/write case.

The map/unmap commands will perform cache coherency operations and do entail
some cost.  The read and write buffer commands currently use memcpy for data
transfer.  

For the examples below, please refer to the OpenCL 1.1 specficication or online
reference pages for the details of the APIs. In these examples, most of the
arguments to the read/write or map/unmap enqueue commands are obvious with the
exception of CL_TRUE as the second argument and 0 as the third argument to
read/write and fourth argument to map/unmap.  The CL_TRUE argument indicates to
the OpenCL runtime that you would like this enqueue command to block  until the
operation is complete. OpenCL enqueue commands are typically asynchronous, the
command is enqueued and the main thread continues execution in parallel with
the operations enqueued.  If these APIs are passed CL_FALSE as a second
argument, then they behave asynchronously as well.  

The 0 argument is an offset into the buffer being read, written, mapped or
unmapped.

The below code fragment illustrates a write/read buffer use case using the C++
OpenCL Binding. An OpenCL context ctx, CommandQueue Q and Kernel K are already
created and bufsize represents the number of bytes in the buffers. Note that
bufsize bytes are allocated in the Linux heap for the array ary and an
additional bufsize bytes are allocated in CMEM for the buffer buf.  This double
allocation is clearly a limitation if bufsize is particularly large.  If it is
not large, then an application can double buffer using this approach.  After
the buffer write, the memory pointed to by ary can be repopulated and a
pipeline can be established. Obviously, in that use case, the example would
need some modification to not reuse ary for the read buffer.::

    int *ary = (int*) malloc(bufsize);

    // populate ary

    Buffer buf (ctx, CL_MEM_READ_WRITE, bufsize);
    K.setArg(0, buf);

    Q.enqueueWriteBuffer(buf, CL_TRUE, 0, bufsize, ary);
    Q.enqueueTask(K);
    Q.enqueueReadBuffer (buf, CL_TRUE, 0, bufsize, ary);

    // consume ary

The below code fragment illustrates a map/unmap buffer use case using the C++
OpenCL Binding. ::

    Buffer buf (ctx, CL_MEM_READ_WRITE, bufsize);
    K.setArg(0, buf);

    int * ary = (int*)Q.enqueueMapBuffer(buf, CL_TRUE, CL_MAP_WRITE, 0, bufsize);
    // populate ary
    Q.enqueueUnmapMemObject(buf, ary);

    Q.enqueueTask(K);

    ary = (int*)Q.enqueueMapBuffer(buf, CL_TRUE, CL_MAP_READ, 0, bufsize);
    // consume ary
    Q.enqueueUnmapMemObject(buf, ary);


Discovering OpenCL Memory Sizes and Limits
=====================================================

TBD

Cache Coherency
=====================================================
The A15 CPUs are cache coherent with each other, but they are not cache
coherent with the C66 DSPs.  The C66 DSPs are not cache coherent with the A15s
or with other C66 DSPs. The OpenCL runtime will manage coherency of the various
device caches through software cache coherency calls.

TBD

Using DMA for data movement within a DSP kernel
=====================================================

TBD

