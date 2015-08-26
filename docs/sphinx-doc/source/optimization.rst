*****************************
Optimization Tips
*****************************

OpenCL applications consist of a host application and a set of device
kernels.  There are optimization techniques for both the host code
and the device code.  There are some techniques that span the boundary between
host and device. This section provides tips for writing performant OpenCL 
applications targeting TI SoCs with DSPs as accelerator devices. These tips are organized into sections based on where the tip is applicable, ie. host or device.

Optimization Techniques for Host Code
=====================================

Use Offline, Embedded Compilation Model  
---------------------------------------
OpenCL allows device code to be compiled on the fly as the host code runs.
This allows for portability of the application but obviously it will slow down
the host application as the compilation occurs.  To speed up the host
application, device code should be compiled offline, i.e. before the host
application runs.  There are two compilation models that use offline
compilation documented in the :doc:`compilation` section. For fastest
operation, the offline compilation with the embedded object model will be the
fastest.  For details on structuring your code for that model, see
:ref:`offline-embedded`.

Avoid the read/write Buffer model on shared memory SoC platforms
----------------------------------------------------------------
On shared memory SoC platforms, the host and devices have the ability to read
and write the same memory region. However, the linux system memory is not
shareable with a device.  Therefore, fast OpenCL applications should avoid
copying data between the linux system memory and the shareable memory regions.
See :doc:`memory/ddr-partition` for details on the linux/opencl memory partition.

The read buffer and write buffer OpenCL operations perform copies and should be
avoided.  Alternatively, fast OpenCL applications will allocate OpenCL buffers
in shared memory and will allow the host to read and write the underlying
memory directly. This can be accomplished two ways.

    #. Create buffers normally and use the map buffer and unmap buffer OpenCL APIs to map the underlying buffer memory into the host address space. See :doc:`memory/buffers` for buffer creation information and see :doc:`memory/access-model` for map/unmap buffer information.

    #. Use __malloc_ddr or __malloc_msmc and use the resulting pointer to create buffers with the CL_MEM_USE_HOST_PTR attribute. See :doc:`memory/host-malloc-extension` for details on __malloc_ddr and __malloc_msmc and see :doc:`memory/buffers` for usage of the CL_MEM_USE_HOST_PTR attribute.


Use MSMC Buffers Whenever Possible
----------------------------------
TI SoCs typically have an on-chip shared memory area, referred to as MSMC.
Memory access latency is much smaller for MSMC than for DDR, therefore
operations on MSMC buffers will perform better than operations on DDR buffers.
This will be particularly true computation cost per byte loaded is low, i.e.
bandwidth limited algorithms.  The TI OpenCL implementation has an extension to
allow global buffers to be created in MSMC memory.  See
:doc:`extensions/msmc-buffers` for details of that extension.  You can also use
the __malloc_msmc memory allocation extension and pass the returned pointer to
the buffer create operation and also assert the CL_MEM_USE_HOST_PTR attribute,
as in the previous subsection.

.. Note::
   MSMC shared memory is not available on the AM57 family of SoCs

Dispatch Appropriate Compute Loads
----------------------------------
Dispatching computation from the host to a device naturally requires some
amount of overhead.  Dispatching individual, small computations will not result
in improved performance. If you have the flexibility to control the size of computation, then a good rule a thumb would be to keep the overhead below 10% of the total dispatch roundtrip.  Of course, you will need to know the overhead in order to calculate a minimum target computation load. 

The overhead of device dispatch is twofold:
    #. The raw OpenCL dispatch overhead which depending on device frequencies
       and which SoC platform is in use, will typically run between 60 and 180
       microseconds per dispatch. The :ref:`null-example` shipped with the TI
       OpenCL product can be used to measure this component of the overhead.

    #. The cost of explicit cache operations on the CPU when communicating
       shared buffers to/from devices. This calculation has some variability, but
       the formula 

           **microseconds = 3 + bytes/8096** 

       per buffer, per dispatch is a reasonable approximation.

As an example, if a kernel K accepted two 1MB buffers as input, then a rough
calculation of the overhead would be: 180 + (3+1024/8) + (3+1024/8) = 442us
and that would imply a recommended minimum compute for K to be 10 x overhead or
roughly 4.5 milliseconds (ms).

In addition to the minimum compute level, the type of compute can matter.  For
bandwidth limited algorithms, where the computation per byte loaded is low, the
device will unlikely perform the calculation faster than the CPU, so an
acceleration should not be expected.  However, it can still be useful to
dispatch such a calculation to the device in order to off-load the CPU and
allow the CPU to perform some other function.

Prefer Kernels with 1 work-item per work-group
----------------------------------------------
For better performance, create work groups with a single work-item and use iteration within the work-group.


Optimization Techniques for Device (DSP) Code
==============================================

Use Local Buffers
---------------------
Local buffers cannot be used to directly communicate between host and device,
but they are very good for storing temporary intermediate values in device code.
On TI SoCs, local buffers are located in L2 SRAM memory, whereas global buffers
are located in DDR3 memory.  The access time to L2 is greater than 10x faster
than to DDR.  The impact of local rather than global is further magnified when
writing values.  For algorithms, where values are written to a
buffer, and the buffer is subsequently used by another kernel or by the CPU host, it is almost
always better to write the values to a local buffer and then copy that local
buffer back to a global buffer using the OpenCL async_work_group_copy
function.  

The below two kernels perform the same simple vector addition operation. The
difference is that the first reads two inputs from ddr and writes a result back
to ddr, where the second reads two inputs from ddr and writes a result to local
L2 and then uses an async_work_group_copy to bulk move the local buffer back to
the global buffer.  The second version is almost 3 times faster than the first
version.

The first version of vector addition ::

    kernel void VectorAdd(global const short4* a, 
                          global const short4* b, 
                          global short4* c) 
    {
        int id = get_global_id(0);
        c[id] = a[id] + b[id];
    }


The second version of vector addition, using local buffers ::

    kernel void VectorAdd(global const short4* a, 
                          global const short4* b, 
                          global short4* c, 
                          local  short4* temp) 
    {
        int id  = get_global_id(0);
        int lid = get_local_id(0);
        int lsz = get_local_size(0);

        temp[lid]  = a[id] + b[id];

        event_t ev = async_work_group_copy(&c[lsz*get_group_id(0)], temp, lsz, 0); 
        wait_group_events(1,&ev); 
    }


Use async_work_group_copy and async_work_group_strided_copy
-----------------------------------------------------------
The previous section illustrated the use of an async_work_group_copy call.
Both OpenCL built-in functions async_work_group_copy and 
async_work_group_strided_copy use a system DMA operation to perform the 
movement of data from one location to another.  There are several reasons 
why this can be beneficial:

    #. As the name implies the async... functions are asynchronous, meaning that
       the call initiates a data transfer but it does not wait for completion 
       before returning.  The subsequent wait_group_events call blocks until 
       the data transfer is complete.  This allows additional work to be 
       performed concurrent with the data transfer.

    #. DDR writes through the system DMA occur in optimal burst sizes, whereas 
       DSP writes to DDR memory do not, because the caches are set to write 
       through mode on the DSPs in order to avoid a false-sharing problem that 
       could result in incorrect results.


Avoid DSP writes to directly to DDR
------------------------------------
See the previous two subsections.

Use the reqd_work_group_size attribute on kernels
----------------------------------------------------------------------------------------------
If you followed the Host optimization tip to "Prefer Kernels with 1
work-item per work-group", then you should annotate your kernel with the
**reqd_work_group_size** attribute to inform the OpenCL C compiler that the
kernel will only have the one work-item. This communicates information to the
OpenCL C compiler that it would otherwise not know and there are a number of
optimizations that are enabled based on this knowledge. An example using this
attribute would look like ::

    kernel __attribute__((reqd_work_group_size(1, 1, 1)))
    void k1wi(global int *p)
    {
       ...
    }

Even if the kernel does have > 1 work-item per work-group, this attribute is
useful to the OpenCL C compiler.  Of course, to use it you would be asserting
that the host code will enqueue this kernel with the same local size as the 
numbers you specify in the attribute. If the kernel is enqueued with a different 
local size than specified in the attribute, the runtime will give a well defined 
error. The below kernel is using the attribute to assert that dimension 1 has a
local size of 640 and dinension 2 has a local size of 480 and dimension three
is unused::

    kernel __attribute__((reqd_work_group_size(640, 480, 1)))
    void img_alg(global int *p)
    {
       ...
    }

Use the TI OpenCL extension than allows Standard C code to be called from OpenCL C code
----------------------------------------------------------------------------------------------
Call existing, optimized, std C code library functions.
Or write your own standard C code.

Avoid OpenCL C Barriers if possible and particularly private data live across barriers
--------------------------------------------------------------------------------------
Particularly prevent private data from being live across barriers.
barrier(), async...(), wait...()

Do Not Use Large Vector Types
------------------------------
Do not use vector types where the size of the vector type is > 64 bits. The C66x DSP has limited instruction support for long vector types, so their use is not performance beneficial.

Vector types with total size <= 64 bits may be beneficial, but the benefit is not guaranteed.

.. Native math operations vs. standard ones.
.. ----------------------------------------------------------------------------------------

.. Use the TI Std C intrinsics 
.. ----------------------------------------------------------------------------------------

.. Fixed point over floating point if possible
.. ----------------------------------------------------------------------------------------

.. Cconsecutive memory accesses
.. ----------------------------------------------------------------------------------------

.. Double buffer technique
.. ----------------------------------------------------------------------------------------

.. Low level DSP optimization
.. ----------------------------------------------------------------------------------------

Prefer the CPU style of writing OpenCL code over the GPU style
--------------------------------------------------------------
There is a large body of existing OpenCL code avaiable and the majority have
been targeted toward and optimized for either GPUs or CPUs.  Often, an
application will have different kernels optimized for each.  Generally, the
versions targeting CPUs will perform better than the version targeting GPUs,
when executed on TI SoC's and using the DSP as a device.


.. MISC
.. ==============
.. #. timing functions
.. #. additional overhead in first kernel dispatch

