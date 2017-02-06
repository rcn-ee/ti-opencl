*****************************************************************
Alternate Host malloc/free Extension for Zero Copy OpenCL Kernels
*****************************************************************

The TI OpenCL implementation adds 4 new host functions

.. cpp:function:: void* __malloc_ddr (size_t size)
.. cpp:function:: void  __free_ddr   (void* p)
.. cpp:function:: void* __malloc_msmc(size_t size)
.. cpp:function:: void  __free_msmc  (void* p)

These new APIs in the TI OpenCL implementation are not specified as part of
OpenCL specification, but are TI extensions that are roughly modeled after
OpenCL 2.0 specified functions clSVMAlloc and clSVMFree. They are not the same
as those functions and therefore will not inherit those names in order to
avoid confusion. 

The __malloc_ddr/__free_ddr APIs allocate and free memory in the global 
address space in off-chip DDR memory. The __malloc_msmc/__free_msmc APIs 
allocate and free memory in the global address space in on-chip MSMC memory. 

The rationale for these memory allocation extensions is to allow OpenCL APIs 
to be hidden in lower-level implementation routines so that top level algorithmic 
code can remain free from OpenCL mechanics.  This abstraction of OpenCL APIs is 
possible without these extensions, but would typically require data copy from Linux 
managed memory to OpenCL managed memory.  These extensions were provided to allow 
the top level algorithmic code to originate data allocation in OpenCL managed memory, 
thus eliminated the need for data copy and improving performance.

Without these new APIs the host application and lower level function might look like::

    main()
    {
        float *p = (float*) malloc(size);
        
        // populate p

        fftw(p, size);

        // consume modified p

        free(p);
    }

    void fftw(float *v1, size_t size)
    {
        Buffer buffer v1Buf (context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, size, v1);
        fftw_kernel.setArg(0, v1Buf);
        fftw_kernel.setArg(1, size);

        Event ev;
        Q.enqueueTask(fftw_kernel, 0, &ev);
        ev.wait();
        ...
    }

The issue with the above code is that the enqueue of the fftw_kernel would entail a
copy of the Linux heap based underlying memory store in the OpenCL buffer to a
copy allocated from :ref:`CMEM<CMEM>`.  It would also entail a copy
back after the enqueue of the kernel.  Since the SoC contains shared
memory between the ARM and the DSP, it would be preferable to have a zero copy
setup. This would be impossible with a Linux heap based malloc pointer.

With the new functions, the above code might look like ::

    main()
    {
        float *p = __malloc_ddr(size);
        
        // populate p

        fftw(p, size);

        // consume modified p

        __free_ddr(p);
    }

    void fftw(float *v1, size_t size)
    {
        Buffer buffer v1Buf (context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, size, v1);
        fftw_kernel.setArg(0, v1Buf);
        fftw_kernel.setArg(1, size);

        Event ev;
        Q.enqueueTask(fftw_kernel, 0, &ev);
        ev.wait();
        ...
    }

In this version, the user simply replaced malloc/free with
__malloc_ddr/__free_ddr and otherwise the user's code remains the same.

The low level fftw function is not modified, but since the incoming pointer v1 now 
points to OpenCL managed :ref:`CMEM<CMEM>` memory, the OpenCL implementation no longer needs 
to copy the data.  It may still perform cache coherency operations surrounding the 
kernel enqueue.

A nice benefit to the above code setup is that the low level function fftw works 
with malloc memory or __malloc_ddr memory.  This is good for cases when the low level 
functions may be part of a 3rd party library.  The use of __malloc_ddr is not required
for correctness and therefore the user of fftw can then choose whether they want to 
modify their source to get the additional performance boost resulting from zero copy. 

OpenCL subbuffers created from OpenCL buffers defined with CL_MEM_USE_HOST_PTR 
and a supplied pointer originating from __malloc_ddr, will also benefit from the 
underlying subbuffer memory residing in :ref:`CMEM<CMEM>`.

.. Caution::
    OpenCL buffers can also be created with CL_MEM_USE_HOST_PTR and a supplied
    pointer in the middle of a __malloc_ddr/__malloc_msmc allocated region.
    However, when doing so, extra caution should be taken not to create
    overlapping buffers, as it is undefined behavior when they are accessed
    by the same kernel and at least one of them is write access.
    If such undefined scenario is absolutely intended, user needs to pass
    "-a" option to the kernel compiler, telling the compiler that kernel
    arguments could alias each other.  "-a" should be specified in the
    clBuildProgram API call for online compilation and on the clocl command
    line for offline compilation.
    Even so, it is still user's responsibility to ensure correct parallel
    execution semantics, for example, when there are multiple workgroups.

.. Important::

    Since the ARM CPU is a 32 bit architecture, Linux will only support 4GB
    of virtual memory at any given time, therefore the amount of memory available to
    __malloc_ddr or __malloc_msmc will be limited in size to a limit below 4GB.
    This is in contrast to defining an OpenCL buffer in the host application that
    is only limited by the maximum block available in any CMEM heap.  These can
    exceed 4GB in length.  This can occur because the allocation of the buffer does
    not imply a map into the virtual memory space.  That would occur independently
    with the mapBuffer commands (which would be limited in size).  A malloc on the
    other hand implies a readiness to use the returned pointer and thus any memory
    returned by clMalloc would be auto mapped into the virtual address space. 

.. Important::

    Obviously, creating a buffer using a __malloc_ddr/__malloc_msmc pointer
    will require size specified at buffer creation time to be less than or equal to
    the size of the allocated memory using __malloc_ddr/__malloc_msmc. 

.. Error::

    Calling __free_ddr() or __free_msmc() on memory that is underlying an
    OpenCL Buffer while the buffer is still in use is undefined behavior.


Memory Alignment
==================
Similar to global buffers and local buffers, memory allocated by
``__malloc_ddr`` and ``__malloc_msmc`` are always aligned at 128-byte boundary.
