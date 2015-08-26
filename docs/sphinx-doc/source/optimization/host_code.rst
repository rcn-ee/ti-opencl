Optimization Techniques for Host Code
*************************************

Use Off-line, Embedded Compilation Model  
---------------------------------------
OpenCL allows device code to be compiled on the fly as the host code runs.
This allows for portability of the application but obviously it will slow down
the host application as the compilation occurs.  To speed up the host
application, device code should be compiled off-line, i.e. before the host
application runs.  There are two compilation models that use off-line
compilation documented in the :doc:`../compilation` section. For fastest
operation, the off-line compilation with the embedded object model will be the
fastest.  For details on structuring your code for that model, see
:ref:`offline-embedded`.

Avoid the read/write Buffer model on shared memory SoC platforms
----------------------------------------------------------------
On shared memory SoC platforms, the host and devices have the ability to read
and write the same memory region. However, the Linux system memory is not
shareable with a device.  Therefore, fast OpenCL applications should avoid
copying data between the Linux system memory and the shareable memory regions.
See :doc:`../memory/ddr-partition` for details on the Linux/opencl memory partition.

The read buffer and write buffer OpenCL operations perform copies and should be
avoided.  Alternatively, fast OpenCL applications will allocate OpenCL buffers
in shared memory and will allow the host to read and write the underlying
memory directly. This can be accomplished two ways.

    #. Create buffers normally and use the map buffer and unmap buffer OpenCL APIs to map the underlying buffer memory into the host address space. See :doc:`../memory/buffers` for buffer creation information and see :doc:`../memory/access-model` for map/unmap buffer information.

    #. Use __malloc_ddr or __malloc_msmc and use the resulting pointer to create buffers with the CL_MEM_USE_HOST_PTR attribute. See :doc:`../memory/host-malloc-extension` for details on __malloc_ddr and __malloc_msmc and see :doc:`../memory/buffers` for usage of the CL_MEM_USE_HOST_PTR attribute.


Use MSMC Buffers Whenever Possible
----------------------------------
TI SoCs typically have an on-chip shared memory area, referred to as MSMC.
Memory access latency is much smaller for MSMC than for DDR, therefore
operations on MSMC buffers will perform better than operations on DDR buffers.
This will be particularly true computation cost per byte loaded is low, i.e.
bandwidth limited algorithms.  The TI OpenCL implementation has an extension to
allow global buffers to be created in MSMC memory.  See
:doc:`../extensions/msmc-buffers` for details of that extension.  You can also use
the __malloc_msmc memory allocation extension and pass the returned pointer to
the buffer create operation and also assert the CL_MEM_USE_HOST_PTR attribute,
as in the previous subsection.

.. Note::
   MSMC shared memory is not available on the AM57 family of SoCs

Dispatch Appropriate Compute Loads
----------------------------------
Dispatching computation from the host to a device naturally requires some
amount of overhead.  Dispatching individual, small computations will not result
in improved performance. If you have the flexibility to control the size of computation, then a good rule a thumb would be to keep the overhead below 10% of the total dispatch round-trip.  Of course, you will need to know the overhead in order to calculate a minimum target computation load. 

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


