*****************************************
Example: Optimizing 1D convolution kernel
*****************************************

Overview
========
In this :ref:`conv1d-example` shipped with OpenCL product, we show how to
optimize an OpenCL 1D convolution kernel step by step.  In general, there
are three areas that we want to optimize:

#. Instruction pipeline: has the loop been software pipelined at a low II
   (initiation interval)?
#. SIMD efficiency: has the available SIMD instructions (e.g. C66x) been
   fully utilized?
#. Memory hierarchy performance: can the input and output data be EDMAed
   with double buffering into faster memory to overlap computation and data
   movement?

Summary of results
==================
The example 1D convolution kernel is applied to each row of a 2D data,
which could represent an image, a collection of independent channels,
and so on.  The 1D convolution kernel/filter size is 5x1.
We write a generic kernel for asymmetric filters.  If your filter is
symmetric, you are welcome to optimize away two multiplications.

We use the timing of a simple straight-forward kernel as the baseline,
and report the speedup of optimized versions against that baseline.  Execution
time is measured with a 1920x1080 input image on the host side and reported
as microseconds.  The same experiments are conducted on both AM572x (2 DSP
cores) and K2H (8 DSP cores) EVMs.  By varying ``NUMCOMPUNITS`` in the
code, we obtain the performance of  dispatching the kernel to 1, 2, 4 and
8 DSP cores.  Each experiment is repeated 5 times and the minimum in each
category is reported here.

+---------------------+--------------+--------------+--------------+--------------+--------------+--------------+
| Kernel version      | Execution Time (us) / Speedup vs. baseline                                              |
+---------------------+--------------+--------------+--------------+--------------+--------------+--------------+
|                     | AM57 1 Core  | AM57 2 Cores | K2H 1 Core   | K2H 2 Cores  | K2H 4 Cores  | K2H 8 Cores  |
+=====================+==============+==============+==============+==============+==============+==============+
| host_compute        | 22796 / 1.40 | 22796 / 0.74 | 25150 / 0.56 | 25150 / 0.65 | 25150 / 0.65 | 25150 / 0.58 |
+---------------------+--------------+--------------+--------------+--------------+--------------+--------------+
| k_baseline          | 31957 / 1.00 | 16997 / 1.00 | 13964 / 1.00 | 16415 / 1.00 | 16395 / 1.00 | 14595 / 1.00 |
+---------------------+--------------+--------------+--------------+--------------+--------------+--------------+
| k_loop              | 30397 / 1.05 | 16769 / 1.01 | 13084 / 1.07 | 14937 / 1.10 | 16593 / 0.99 | 14048 / 1.04 |
+---------------------+--------------+--------------+--------------+--------------+--------------+--------------+
| k_loop_simd         | 18075 / 1.77 |  9986 / 1.70 |  8403 / 1.66 |  9700 / 1.69 | 11867 / 1.38 | 10579 / 1.38 |
+---------------------+--------------+--------------+--------------+--------------+--------------+--------------+
| k_loop_db           | 12783 / 2.50 |  6548 / 2.60 |  8498 / 1.64 |  4407 / 3.72 |  2741 / 5.98 |  3086 / 4.73 |
+---------------------+--------------+--------------+--------------+--------------+--------------+--------------+
| k_loop_simd_db      |  9816 / 3.26 |  5072 / 3.35 |  6213 / 2.25 |  3254 / 5.04 |  2681 / 6.11 |  2989 / 4.88 |
+---------------------+--------------+--------------+--------------+--------------+--------------+--------------+
| k_loop_simd_db_extc |  8200 / 3.90 |  4321 / 3.93 |  5428 / 2.57 |  2854 / 5.75 |  2800 / 5.85 |  3056 / 4.78 |
+---------------------+--------------+--------------+--------------+--------------+--------------+--------------+

.. Note::
    This example hit the memory bandwidth constraint on K2H after
    2 DSP cores.  In other words, there simply isn't enough computation to
    go around more DSP cores and keep them all busy, per maximum of data that
    can be transferred to the chip at a time.  If the computation to data
    ratio is higher, we do see benefits of using more DSPs.  ``sgemm`` and
    ``dgemm`` are two of these examples that are shipped with OpenCL product.

    +------------+--------------------+-----------------+
    | Example    | AM572x 2 DSP cores | K2H 8 DSP cores |
    +============+====================+=================+
    | sgemm      | 13.378 GFlops      | 93.654 GFlops   |
    +------------+--------------------+-----------------+
    | dgemm      |  3.609 GFlops      | 25.891 GFlops   |
    +------------+--------------------+-----------------+

In what follows, we go through the optimizations that we applied to
achieve the performance improvements.

Driver code setup
=================

The 1D convolution kernel is applied in a 2D image.  We wrote a drive code
that initialize a 2D image with random data and call the OpenCL kernels
accordingly.  We chose HD image size 1920x1080 as input and output.
The same kernel computation is performed on the host (ARM) side and its results
are verified against the kernel results from DSP to ensure the correctness.
Performance is measured as the elapsed time on the host side before enqueuing
the kernel and after the kernel has finished.

Initially we partitioned OpenCL global size (1920, 1080) into NUMCOMPUNITS
workgroups, each of local size (1920, 1080/NUMCOMPUNITS), so that each DSP
core will get a workgroup.

k_baseline: Ensure correct measurements
=======================================

TI's OpenCL runtime will lazily load the device program upon the first enqueue
of a kernel from the program, so the elapsed time overall from the first
enqueue will be longer to account for the loading of the program.  To remove
program loading overhead from kernel performance, we can enqueue a null kernel
before running other kernels.

k_baseline: Check software pipelining
=====================================

We can look at the assembly output to see if the compiler has successfully
software pipelined the kernel.  For the original 2 dimensional kernel,
compiler will add two implicit loops around the kernel to create an OpenCL
workgroup and try to software pipeline the innermost loop.  Run compiler
with an option, ``-k``, to keep the assembly code::

  clocl -k ti_kernels.cl

Look at the ``k_conv1d_5x1`` function in ``ti_kernels.asm``, search
for ``SOFTWARE PIPELINE`` and we see these two lines::

  ;*      Searching for software pipeline schedule at ...
  ;*         ii = 7  Schedule found with 5 iterations in parallel

So the original kernel has already been software pipelined.  A loop iteration
in the innermost dimension is started every 7 cycles.

k_loop: Improve software pipelining
===================================

A closer look at the baseline source code and we see that the loop is dealing
with some boundary conditions.  If we can peel those boundary iterations from
the main loop, then the main loop may be scheduled at a lower ii.
To do that, we also need to reduce OpenCL kernel work space from 2D
to 1D so that the innermost loop becomes explicit in the kernel code.
Kernel ``k_loop`` is the result of such transformations.
From the assembly file, we can see that the main loop is schedule at
ii=3, which means an iteration is started every 3 cycles::

  ;*      Searching for software pipeline schedule at ...
  ;*         ii = 3  Schedule found with 10 iterations in parallel

Summary

#. Make col-dimension loop explicit in the kernel, reduce OpenCL kernel
   work space from 2D to 1D
#. Peel the boundary conditions and remove the boundary checks

Alternatively, you may pad the input data or reduce the output size so that
the boundary conditions go away.

With reduced ii, we didn't see much performance improvement from execution
when compared to the baseline version.  One possible reason is that the
software pipeline stalls due to cache misses have dominated the execution.
It is time to optimize for the memory hierarchy.  Before doing that, let's
see if we can optimize for the SIMD features available on C66 DSPs.

k_loop_simd: Improve software pipelining with SIMDization
=========================================================

Sometimes, compiler may not be able to auto-SIMDize the loop.  We can
look at the involved memory accesses and computations and perform
SIMDization by hand.  Due to the OpenCL C vector semantics, we have
to assume that each row is properly aligned on the 8-byte boundary for
using vector type of ``float2``.  First we SIMDize the memory accesses and
computations, next we seek the opportunity to pipeline the loaded values
in the registers.  ``k_loop_simd`` is the result of SIMDization.  From the
assembly, we can see that an unrolled iteration (corresponding to two
baseline iterations) is started every 5 cycles::

  ;*      Searching for software pipeline schedule at ...
  ;*         ii = 5  Schedule found with 5 iterations in parallel

Summary

#. Unroll col-loop by a factor of 2 by hand
#. Data layout requirement: each row is aligned on 8-byte double word boundary
#. SIMDize loads and stores
#. SIMDize computation
#. Pipeline loaded values in registers if possible

k_loop_db: EDMA and double buffer k_loop
========================================

TI's OpenCL implementation designate part of L2 SRAM on each core for OpenCL
local memory.  We can use EDMA to move data from global buffers (DDR) into
local buffers (L2), perform computation on the local buffers, then store
results from local buffers (L2) back to global buffers (DDR).  OpenCL C
kernel language has built-in async_work_group_*() functions that we map
to TI's EDMA routines.  To best utilize the asynchronous feature of EDMA,
we use double buffering (ping-pong) to effectively overlap data movement
and computation.

For this particular kernel, each row requires
``COLS*sizeof(float) + COLS*sizeof(float)`` bytes for input and output.
With double buffering, each row requires ``16 * COLS`` bytes for input and
output.  Given ``COLS = 1920`` that we chose, we can fit a maximum of four
rows into the 128KB local memory, or a maximum of 25 rows into 768KB local
memory::

    4  * (2 * (1920*4 + 1920*4)) <= 128 * 1024
    25 * (2 * (1920*4 + 1920*4)) <= 768 * 1024

To ensure that the double buffering pipeline executes at least a few times,
say 8, we can cap the BLOCK_HEIGHT to ``ROWS / NUMCOMPUNITS / 8 + 1``.
In the kernel, before computing current block of rows of image in local
memory, we prefect next block of rows into local memory with EDMA.

Another transformation is that the kernel now explicitly iterates through
the row dimension as well, because of the requirement of double buffering.
Accordingly, we need to set required kernel work group size to (1, 1, 1).
In the host code, we only need to specify the number of workgroups,
which we use the number of compute units, when enqueuing the ND range kernel.

We added three additional arguments to the kernel: block height, local
buffer for input and local buffer for output.  Local buffers are allocated
automatically by OpenCL runtime, OpenCL application code only needs to specify
the sizes.

With all these transformation, we see that non-SIMDized ``k_loop_db``
outperforms not only baseline ``k_loop``, but also SIMDized ``k_loop_simd``.

Summary

#. Require 8-byte alignment for each row
#. Determine the block height for double buffering
#. Set required work group size to (1,1,1) for kernel
#. Set OpenCL workspace to (NUMCOMPUNITS, 1, 1), each work group will figure
   out which rows to work on
#. Double buffer with EDMA on input and output, computation only loads from
   and stores to local buffers

k_loop_simd_db: EDMA and double buffer k_loop_simd
==================================================

We apply the same EDMA and double buffering transformation on
``k_loop_simd`` as above.  Now we see similar performance improvements
upon ``k_loop_simd``.

k_loop_simd_db_extc: Use external C function for k_loop_simd_db
===============================================================

While we can handle this example completely in OpenCL C language, sometimes
OpenCL C has limitations in expressiveness with regard to our C66 DSP.
For example, C66 DSP can do more patterns of EDMA transfers than
async_work_group_*() OpenCL C built-in functions can represent, C66 DSP
support non-aligned SIMD loads and stores.  When these limitations do affect
user applications, we can work around them in standard C functions and invoke
them from within OpenCL C code.

We use this version as an example how to incorporate standard C functions
into OpenCL.  We move the body of ``k_loop_simd_db`` into an external C
function, and treat the OpenCL C function as a simple wrapper function.
The external C function is compiled with C66 C compiler and you can use C66
C intrinsics.  Similarly, you can re-utilize existing optimized C
implementations and libraries developed by themselves or TI. 
Of course, this is a TI's extension and is not applicable to OpenCL platforms
from other vendors.

``c_loop_simd_db_extc()`` in ``k_ext.c`` is the rewritten C function.  Note
the explicit use of EdmaMgr functions and C66 SIMD intrinsics.
With this version, we got slightly better performance.

Summary

#. Move kernel body to an external standard C function
#. Use EdmaMgr_*() functions directly, cover non-consecutive transfers
#. Use C66 C SIMD intrinsic built-in functions, cover non-aligned SIMD loads
   and stores
#. Link separately compiled C object back to kernel executable

