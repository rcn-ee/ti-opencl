*********************************************************
User Defined DSP Heap Extension
*********************************************************


User Defined DSP Heap Built-in Functions
===========================================
The natural way to handle memory allocation in an OpenCL application is through
the use of OpenCL buffers.  However, when using the TI OpenCL extension that
allows calling standard C code from OpenCL C code, the standard C code will
often contain C standard library heap allocation routines (malloc and free).  

The OpenCL runtime does allow for a pre-allocated but usually small heap to 
service these malloc type calls.  For cases where that heap is too small or 
a heap in MSMC or L2 are required, TI OpenCL has provided built-in functions 
for initializing user defined heaps and alternate built-in functions for 
allocating, reallocating and freeing from these heaps.

The following three built-in functions create user defined heaps.

.. cpp:function::    void __heap_init_ddr (void *ptr, int size)

    Initialize a user defined heap in DDR of *size* bytes.  The memory that
    *ptr* points to should be DDR. 

.. cpp:function::    void __heap_init_msmc(void *ptr, int size)

    Initialize a user defined heap in MSMC of *size* bytes.  The memory that
    *ptr* points to should be MSMC. 

.. cpp:function::    void __heap_init_l2  (void *ptr, int size)

    Initialize a user defined heap in L2 of *size* bytes.  The memory that
    *ptr* points to should be L2. 

Note that *ptr* is a pointer to underlying memory to be configured as a
user-controlled heap. Therefore, the underlying memory must be allocated before
calling the heap initialization function. Initialized heaps are persistent
across kernels until the underlying memory regions for them are deallocated.

The user defined heap initialization functions for DDR and MSMC must be called 
by **only one** of the DSP cores to initialize internal heap data structures 
before making any allocation calls such as __malloc_ddr or __malloc_msmc. 
Once initialized, the heaps are accessible by all the DSP cores. These APIs are 
thread safe under the OpenCL programming model on the DSP.

The user defined heap initialization functions for L2 can be called by any or 
all of the DSP cores as these heaps are core private.  __heap_init_l2 must be called 
to initialize internal heap data structures before making any __malloc_l2 calls.

.. Note:: 

    If data allocated on the heap is shared across DSP cores using OpenMP on 
    the DSP, cache consistency is managed by the OpenMP runtime. For all other 
    cases where the cores are sharing data, the programmer is
    responsible for cache consistency. 

.. Note:: 

    For heaps created in MSMC or DDR, the contents of the heap are shared 
    across the cores.  If the application is written such that multiple DSP 
    cores are writing and reading the same locations, the programmer is responsible 
    for synchronizaton to prevent data race conditions. 


The following five built-in functions replace the C standard library functions:
malloc, calloc, realloc, free, and memalign for user defined heaps in DDR.

.. cpp:function:: void *__malloc_ddr   (size_t size)
.. cpp:function:: void *__calloc_ddr   (size_t num, size_t size)
.. cpp:function:: void *__realloc_ddr  (void *ptr,  size_t size)
.. cpp:function:: void  __free_ddr     (void *ptr)
.. cpp:function:: void *__memalign_ddr (size_t alignment, size_t size)

The following five built-in functions replace the C standard library functions:
malloc, calloc, realloc, free, and memalign for user defined heaps in MSMC.

.. cpp:function:: void *__malloc_msmc   (size_t size)
.. cpp:function:: void *__calloc_msmc   (size_t num, size_t size)
.. cpp:function:: void *__realloc_msmc  (void *ptr, size_t size)
.. cpp:function:: void  __free_msmc     (void *ptr)
.. cpp:function:: void *__memalign_msmc (size_t alignment, size_t size)

The following built-in function replaces the C standard library functions:
malloc for user defined heaps in L2. __free_l2() is not provided as the L2
user defined heaps are deallocated as a whole at the end of every kernel 
execution.

.. cpp:function:: void *__malloc_l2 (size_t size)

     return a pointer to L2 memory.  The pointer returned is aligned 
     to an 8 byte boundary. Malloced memory from L2 will cease to exist at 
     every kernel execution boundary.  It is therefore not possible to use 
     this mechanism to create L2 based heaps that persist from one kernel
     enqueue to another kernel enqueue.


Allocation of the Underlying Memory for User Defined DSP Heaps
==============================================================

For DDR or MSMC user defined heaps
----------------------------------

From OpenCL C code, a DDR or MSMC heap can be initialized using 
kernel parameters for a global buffer and a size of that buffer.  The below
example illustrates a DDR heap, but the mechanism for MSMC is exactly the same
kernel code.  The only difference is that the host code should pass a global
buffer defined to be in MSMC ::

    kernel void heap_init_ddr(global char *p, size_t bytes)
    {
        __heap_init_ddr(p, bytes); 
    }

From standard C code called from OpenCL C, a DDR or MSMC heap can be initialized using 
an object defined to be in a specific section bound to either MSMC or DDR. 
The below example illustrates a MSMC heap, but the mechanism for DDR is the same ::

    #define MSMC_HEAP_SIZE (16<<20)
    #pragma DATA_SECTION(msmc_heap, ".mem_msm")
    char msmc_heap[MSMC_HEAP_SIZE];

    void foo()
    {
        __heap_init_msmc ((void *)msmc_heap, MSMC_HEAP_SIZE);
        ...
    }
    

For L2 user defined heaps
------------------------------

Use OpenCL local buffers to allocate of chunk of memory in L2 OpenCL. 
This chunk can be used to initialize the heap:

From OpenCL C code ::

    kernel void example(local void *ptr, int size)
    {
        __heap_init_l2(ptr, size);
        ...
        __malloc_l2(sizeof(double)));
        ...
    }

    // The host code to define a local buffer and set it as an 
    // argument would look like the following

    Kernel kernel2(program, "example");
    kernel2.setArg(0, __local(L2_HEAP_SIZE));
    kernel2.setArg(1, L2_HEAP_SIZE);

or a static local buffer can be used ::

    kernel void example()
    {
        local l2_heap_area[1024];

        __heap_init_l2((void*) l2_heap_area, 1024);
        ...
        __malloc_l2(sizeof(double)));
        ...
    }

From standard C code called from OpenCL C, an L2 heap can be initialized using 
an object defined to be in a specific section bound to L2. ::

    #define L2_HEAP_SIZE (1024)
    #pragma DATA_SECTION(l2_heap, ".mem_l2")
    char l2_heap[L2_HEAP_SIZE];

    void foo()
    {
        __heap_init_l2 ((void *)l2_heap, L2_HEAP_SIZE);
        ...
        __malloc_l2(sizeof(double)));
        ...
    }


Putting it all Together
=======================

The following code illustrate how to allocate memory for user defined heaps and
call the initialization functions. The :ref:`dspheap-example`, shipped with the 
product contains complete source code.

OpenCL Kernel Code ::

    /*-----------------------------------------------------------------------------
    * These kernels initialize user controlled heaps,  they do not have to be
    * separate kernels.  The call to __heap_init_xxx can be rolled into an existing
    * kernel and called before any __malloc_xxx calls are made.
    *
    * These heaps can be persistent across kernel boundaries as long as the
    * underlying memory (aka buffers pointed to by p are not deallocated.
    *----------------------------------------------------------------------------*/
    kernel void heap_init_ddr(void *p, size_t bytes)
        { __heap_init_ddr(p,bytes); }

    kernel void heap_init_msmc(void *p, size_t bytes)
        { __heap_init_msmc(p,bytes); }

    /*-----------------------------------------------------------------------------
    * This kernel will allocate from the heaps and then free them memory.
    *----------------------------------------------------------------------------*/
    kernel void alloc_and_free(int bytes)
    {
        char *p1 = __malloc_ddr(bytes);
        char *p2 = __malloc_msmc(bytes);

        if (!p1 || !p2) return;

        printf("DDR  heap pointer is 0x%08x\n", p1);
        printf("MSMC heap pointer is 0x%08x\n", p2);

        __free_ddr(p1);
        __free_msmc(p2);
    }

OpenCL Host Code ::

    /*------------------------------------------------------------------------
    * Create the underlying memory store for the heaps with OpenCL Buffers
    *-----------------------------------------------------------------------*/
    int ddr_heap_size  = 16 << 20;  // 16MB
    int msmc_heap_size = 1 << 20;   // 1MB
    Buffer HeapDDR (context, CL_MEM_READ_WRITE, ddr_heap_size);
    Buffer HeapMSMC(context, CL_MEM_READ_WRITE|CL_MEM_USE_MSMC_TI, msmc_heap_size);

    ...

    /*------------------------------------------------------------------------
    * Create a command queue and kernelfunctors for all kernels in our program
    *-----------------------------------------------------------------------*/
    CommandQueue Q(context, devices[0]);
    KernelFunctor heap_init_ddr  = Kernel(program, "heap_init_ddr") .bind(Q, NDRange(1), NDRange(1));
    KernelFunctor heap_init_msmc = Kernel(program, "heap_init_msmc").bind(Q, NDRange(1), NDRange(1));
    KernelFunctor alloc_and_free = Kernel(program, "alloc_and_free").bind(Q, NDRange(8), NDRange(1));

    /*------------------------------------------------------------------------
    * Call kernels to initialize a DDR based and a MSMC based heap, the init
    * step only needs to run once and one 1 core only.  See the functor
    * mapping above that defines the global size to be 1.
    *-----------------------------------------------------------------------*/
    heap_init_ddr (HeapDDR,  ddr_heap_size) .wait();
    heap_init_msmc(HeapMSMC, msmc_heap_size).wait();

    /*------------------------------------------------------------------------
    * On each core alloc memory from both ddr and msmc and the free it.
    *-----------------------------------------------------------------------*/
    alloc_and_free(1024).wait();

