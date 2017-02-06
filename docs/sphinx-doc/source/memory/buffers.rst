******************************************************
OpenCL Buffers
******************************************************

Global Buffers
=====================================================

OpenCL global buffers are the conduit through which data is communicated from
the host application to OpenCL C kernels running on the C66x DSP.  The C
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
``ctx`` has been created with only the DSPs present in the context. The
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
values may be ORed together to create buffers with a combination of
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
  The ARM A15 devices are not cache coherent with the C66x
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
  not be used for performance-critical OpenCL code. However, this flag
  does simplify the API calls and can be used for prototyping.

CL_MEM_ALLOC_HOST_PTR
  A host_ptr argument is not necessary for buffers created with this creation
  flag. The default NULL value is valid. This flag is mutually exclusive with
  CL_MEM_USE_HOST_PTR. This flag indicates that OpenCL should allocate an
  underlying memory store for the buffer than can be accessed from the host.
  For this implementation, a buffer created with this flag is allocated memory
  in the CMEM contiguous memory region and can be accessed directly from both
  the host A15 and the C66 DSPs. This flag is recommended for performance
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
  This flag is a TI extension to standard OpenCL on 66AK2x devices only. It can
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
consume data all on the C66x DSP. Other than creating the buffer through which
the communication will occur and sequencing the kernel enqueues, it is not
necessary for the host A15 to be involved in that data communication from
kernel 1 to kernel 2, i.e. the A15 does not need to read the data from kernel 1
and transfer it to kernel 2, the data can simply persist on the C66x DSP.

Local Buffers
=====================================================

Local buffers are quite different than global buffers. You cannot access local
buffers from the host and you do not create them using API's like global
buffers. Local buffers will be allocated from local memory which in this
implementation exists in the L2 scratchpad memory on the C66x DSP cores. Data
cannot persist from kernel to kernel in a local buffer. The lifetime of a local
buffer is the same as the dynamic lifetime of the kernel execution. Local
buffers are never required to be used, but are often used in OpenCL C kernels
for potential performance improvement. The typical use case for local buffers
in a kernel that is passed a global buffer, is for the local buffer to be used
explicitly by the user's OpenCL C kernel as a fast scratchpad memory for the
larger and slower global buffer. This scratchpad memory would be managed by the
user using asynchronous built-in functions to move the data between the global
and local buffers. Again local buffers are never required and the OpenCL C
kernel can depend on the C66x DSP cache to alleviate DDR access delay rather
than use local buffers. However, it is often the case that manual data movement
to/from local buffers can be advantageous to performance.

Local buffers can be defined in two ways. The first way is to simply define an
array in your OpenCL C kernel that is defined with the local keyword. For
example, the following OpenCL C kernel defines a local buffer named scratch and
then calls the async_work_group_copy built-in function to copy 100 char values
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
=====================================================

OpenCL Sub-Buffers are aliases to existing OpenCL global Buffers. Creating a
sub-buffer does not result in any underlying memory store allocation above what
is already required for the aliased buffer.  There are two primary use cases
for sub-buffers:

1. Accessing a buffer with different access flags than were specified in buffer
   creation, or 
2. Accessing a subset of a buffer.

The C++ API's for creating SubBuffers are described below. Please see the
OpenCL 1.1 specification or the OpenCL 1.1 On-line Reference
http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/ 
for the syntax of the C API for sub-buffer creation.::

    typedef struct _cl_buffer_region { size_t origin; size_t size;} cl_buffer_region;

    Buffer createSubBuffer(cl_mem_flags flags, cl_buffer_create_type buffer_create_type,
                           const void * buffer_create_info, cl_int * err = NULL);

createSubBuffer is a member function of the OpenCL C++ Buffer object. The flags
argument should be one of ``CL_MEM_READ_WRITE, CL_MEM_READ_ONLY,
CL_MEM_WRITE_ONLY``. The buffer_create_type should be
``CL_BUFFER_CREATE_TYPE_REGION``. That is the only cl_buffer_create_type
supported in OpenCL 1.1. The buffer_create_info argument should be a pointer to
a cl_buffer_region structure, in which you define the buffer subset for the
sub-buffer. Usage of these APIs may look like::

    Buffer buf(ctx, CL_MEM_READ_WRITE, bufsize);
    cl_buffer_region rgn = {0, bufsize};

    Buffer buf_rd = buf.createSubBuffer(CL_MEM_READ_ONLY,  CL_BUFFER_CREATE_TYPE_REGION, &rgn);
    Buffer buf_wt = buf.createSubBuffer(CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &rgn);

The prior subsection indicated that global buffers can be persistent from one
kernel invocation to the next. It is a common use case that kernel K1 only
writes a buffer and kernel K2 only reads the buffer. The buffer must be created
with the CL_MEM_READ_WRITE access flag, because the buffer is being both read
and written by OpenCL C kernels running on the C66x DSPs. However, no individual
kernel is both reading and writing the buffer, so the CL_MEM_READ_WRITE
property that the buffer has, may result in underlying cache coherency
operations that are unnecessary. For performance reasons, sub-buffers can be
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

Buffer Alignment
=======================

In TI's implementation, global buffers (both DDR and TI extended MSMC)
and local buffers are always aligned at 128-byte memory boundary.  Sub-buffers
are aligned accordingly to their origin/offset in the existing global buffers.
