********************
Example Descriptions
********************

The examples in this section are included in the installation of the OpenCL
product.  Not all of these examples are applicable to all supported device
platforms.

These examples are are intended to illustrate a technique, an extension, or
a mode of operation.  The following table provides
a high level map of the example name to the features that are highlighted by
that example.

The key to the codes in the table are in subsequent tables.

================== ======= =============== ============== ============ ========= ========================= ==================
Name               Type    Execute Model   Kernel Compile Buffer Model Profiling Extensions                Techniques
================== ======= =============== ============== ============ ========= ========================= ==================
abort_exit         S       ndr,iot,oot     B/E            read                   abort,exit
ccode              S       1wi             S/F            read                   C
conv1d             P       ndr,1wi         B/E            map          host      C, edma                   async, local, query, vec
dgemm              P       iot             B/E            host         host      C, omp, msmc, edma, cache
dspheap            S       1wi             B/F                                   dspheap, msmc             functor
dsplib_fft         P       ndr,1wi         B/E            host         host      C
edmamgr            S       1wi             B/E            read                   C, edma
edmabw             I       iot             B/E            host                   C                         async
float_compute      S       ndr             B/F            host         host                                local, async, vec
mandelbrot         S       ndr             S/F            read         host                                nDev
matmpy             S       1wi             B/F            read         host      C, msmc                   nDev, async, local
null               I       iot             S/E                         host
offline            S       ndr             B/F            read         event                               vec
offline_embed      S       ndr             B/E            read         event                               vec
ooo                S       oot             S/E            read         host                                event, native
ooo_callback       S       oot             S/E            read         host                                event, callback
ooo_map            S       oot             S/E            map          host                                event, native
platforms          I                                      query
tidl               P       custom                         host         host
sgemm              P       1wi             B/E            map          host      C, msmc, edma, cache      local, vec
Simple             S       ndr             S/E            read                                             functor
timeout            S       ndr,iot,oot     B/E            read                   timeout
vecadd             S       ndr             S/E            host                                             vec
vecadd_mpax        S       ndr             S/E            map                                              extMem, query, vec
vecadd_openmp      S       iot             S/F            read         event     C, omp
vecadd_openmp_t    S       iot             S/F            read         event     C, omp
vecadd_subdevice   S       ndr             S/F            host         host                                vec
================== ======= =============== ============== ============ ========= ========================= ==================

======= =====================
Type
======= =====================
S       Simple illustration
P       Performance motivated
I       Information gathering
======= =====================

============ ======================================================
Buffer Model
============ ======================================================
read         Uses enqueueReadBuffer and enqueueWriteBuffer
map          Uses enqueueMapBuffer and enqueueUnmapMemObject
host         Uses the CL_MEM_USE_HOST_PTR buffer creation attribute
============ ======================================================

============== ===================================================================
Kernel Compile
============== ===================================================================
S/E            Creates kernel program from Source Embedded in the host application
S/F            Creates kernel program from Source read from a File
B/E            Creates kernel program from Binary Embedded in the host application
B/F            Creates kernel program from Binary read from a File
============== ===================================================================

========= ======================================================================
Profiling
========= ======================================================================
event     Uses profiling timestamp information queried from OpenCL Event objects
host      Uses the host clock_gettime function to measure elapsed time
device    Uses __clock() or __clock64() to measure elapsed cycles on the DSP
========= ======================================================================

=============== ================================================================
Execution Model
=============== ================================================================
ndr             Queues a generic NDRangeKernel with > 1 work-item per work-group
1wi             Queues a NDRangeKernel with 1 work-item per work-group
iot             Queues a Task (1 work-item) in an In Order Queue
oot             Queues a Task (1 work-item) in an Out of Order Queue
custom          Queues a builtin kernel onto a custom device
=============== ================================================================

========== ============================================================
Extensions
========== ============================================================
C          Kernels contain calls to standard C code
omp        Kernels contain calls to standard C code with OpenMP pragmas
msmc       Buffers created in on-chip MSMC memory are used
edma       Kernels use the EdmaMgr builtin functions for DMA control
cache      Kernels use the cache re-configuration builtin functions
dspheap    Kernels create user defined heaps on the DSP
abort      Kernels call abort() to terminate execution
exit       Kernels call exit() to terminate execution
timeout    Kernels terminate if the set timeout limit expires
========== ============================================================

========== ===========================================================================================
Techniques
========== ===========================================================================================
functor    The C++ binding's Kernel Functor object is used
event      OpenCL Events are used to set dependencies between enqueued commands
nDev       The OpenCL application can be dynamically partitioned across multiple OpenCL devices
native     The OpenCL application uses native kernels on the host
callback   The callback feature is used to asynchronously call a host function on event status change
extMem     The extended memory capability is used to access memory beyond the 32-bit address space
async      The async_work_group_copy functions are used to move data between memory spaces
local      OpenCL Local Buffers are used for performance improvement
query      OpenCL platforms and/or devices are queried for attributes
vec        OpenCL C vector data types are used in kernels
========== ===========================================================================================


.. _platforms-example:

platforms example
====================

This example uses the OpenCL C++ bindings to discover key platform and device
information from the OpenCL implementation and print it to the screen.
It also reports the version of the installed TI OpenCL product.

.. _simple-example:

simple example
=================

This is a 'hello world' type of example that illustrates the minimum steps
needed to dispatch a kernel to a DSP device and read a buffer of data back.

.. _mandlebrot-example:

mandelbrot, mandelbrot_native examples
=======================================

The 'mandelbrot' example is an OpenCL demo that uses OpenCL to generate the
pixels of a Mandelbrot set image. This example also uses the C++ OpenCL
binding. The OpenCL kernels are repeatedly called generating images that are
zoomed in from the previous image. This repeats until the zoom factor reaches
1E15.

This example illustrates several key OpenCL features:

- OpenCL queues tied to potentially multiple DSP devices and a dispatch
  structure that allows the DSPs to cooperatively generate pixel data,
- The event wait feature of OpenCL,
- The division of one time setup of OpenCL to the repetitive en-queuing of
  kernels, and
- The ease with which kernels can be shifted from one device type to another.

The 'mandelbrot_native' example is non-OpenCL native implementation (no
dispatch to the DSPs) that can be used for comparison purposes. It uses OpenMP
for dispatch to each ARM core. Note: The display of the resulting
Mandelbrot images is currently disabled when run on the default EVM Linux
file system included in the Processor SDK. Instead it will output frame information.

.. _ccode-example:

ccode example
==============================

This example illustrates the TI extension to OpenCL that allows OpenCL C code
to call standard C code that has been compiled off-line into an object file or
static library. This mechanism can be used to allow optimized C or C callable
assembly routines to be called from OpenCL C code. It can also be used to
essentially dispatch a standard C function, by wrapping it with an OpenCL C
wrapper. Calling C++ routines from OpenCL C is not yet supported. You should
also ensure that the standard C function and the call tree resulting from the
standard C function do not allocate device memory, change the cache structure,
or use any resources already being used by the OpenCL runtime.

.. _matmpy-example:

matmpy example
==============================

This example performs a 1K x 1K matrix multiply using both OpenCL and a native
ARM OpenMP implementation (GCC libgomp). The output is the execution time for
each approach (OpenCL dispatch to the DSP vs. OpenMP dispatching to the 4 ARM
A15s).

.. _offline-example:

offline example
==============================

This example performs a vector addition by pre-compiling an OpenCL kernel into
a device executable file. The OpenCL program reads the file containing the
pre-compiled kernel in and uses it directly. If you use offline compilation to
generate a .out file containing the OpenCL C program and you subsequently move
the executable, you will either need to move the .out as well or the
application will need to specify a non-relative path to the .out file.

.. _vecadd_openmp-example:

vecadd_openmp example
==============================

This is an OpenCL + OpenMP example. OpenCL program is running on the host,
managing data transfers, and dispatching an OpenCL wrapper kernel to the
device. The OpenCL wrapper kernel will use the ccode mode (see ccode example)
to call the C function that has been compiled with OpenMP options (omp). To
facilitate OpenMP mode, the OpenCL wrapper kernel needs to be dispatched as an
OpenCL Task to an In-Order OpenCL Queue.

.. _vecadd_openmp_t-example:

vecadd_openmp_t example
========================

This is another OpenCL + OpenMP example, similar to vecadd_openmp. The main
difference with respect to vecadd_openmp is that this example uses OpenMP tasks
within the OpenMP parallel region to distribute computation across the DSP cores.

.. _vecadd-example:

vecadd example
================

The same functionality as the vecadd_openmp example, but expressed fully as an
OpenCL application without OpenMP. Included for comparison purposes.

.. _vecadd_mpax-example:

vecadd_mpax example
==========================

The same functionality as the vecadd example, but with extended buffers. The
example iteratively traverses smaller chunks (sub-buffers) of large buffers.
During each iteration, the smaller chunks are mapped/unmapped for read/write.
The sub-buffers are then passed to the kernels for processing. This example
could also be converted to use a pipelined scheme where different iterations of
CPU computation and device computation are overlapped. NOTE: The size of the
buffers in the example (determined by the variable 'NumElements') is dependent
on the available CMEM block size. Currently this example is configured to use
buffers sizes for memory configurations that can support 1.5 GB total buffer
size. The example can be modified to use more (or less) based on the platform
memory configuration.

.. _vecadd_mpax_openmp-example:

vecadd_mpax_openmp example
==========================

Similar to vecadd_mpax example, but used OpenMP to perform the parallelization
and the computation. This example also illustrates that printf() could be used
in OpenMP C code for debugging.

.. _vecadd_subdevice-example:

vecadd_subdevice example
========================

The same functionality as the vecadd example, but using sub devices. This
example illustrates the use of sub devices using the OpenCL C API. It performs
vecadd on the root device as well as equally partitioned individual sub devices
and measures the time taken by each of them.

.. _dsplib_fft-example:

dsplib_fft example
===================

An example to compute multiple channels of FFTs using a routine from the
dsplib library. This illustrates calling a standard C library function from
an OpenCL kernel.  It also illustrates how to improve performance over
multiple channels by moving data from DDR into internal local L2 memory
with EDMA, and overlapping computation with data movement using double
buffering.

.. _ooo-examples:

ooo, ooo_map examples
=======================

This application illustrates several features of OpenCL.

- Using a combination of In-Order and Out-Of-Order queues
- Using native kernels on the CPU
- Using events to manage dependencies among the tasks to be executed. A JPEG in
  this directory illustrates the dependence graph being enforced in the
  application using events.

The ooo_map version additionally illustrates the use of OpenCL map and unmap
operations for accessing shared memory between a host and a device. The
Map/Unmap protocol can be used instead of read/write protocol on shared memory
platforms.

Requires the  TI_OCL_CPU_DEVICE_ENABLE environment variable to be set. For
details, refer :doc:`../environment_variables`

.. _null-example:

null example
===============

This application is intended to report the time overhead that OpenCL requires
to submit and dispatch a kernel. A null(empty) kernel is created and dispatched
so that the OpenCL profiling times queried from the OpenCL events reflects only
the OpenCL overhead necessary to submit and execute the kernel on the device.
This overhead is for the round-trip for a single kernel dispatch. In practice,
when multiple tasks are being enqueued, this overhead is pipelined with
execution and can approach zero.

.. _sgemm-example:

sgemm example
================

This example illustrates how to efficiently offload the CBLAS SGEMM routine
(single precision matrix multiply) to the DSPs using OpenCL. The results
obtained on the DSP are compared against a cblas_sgemm call on the ARM. The
example reports performance in GFlops for both DSP and ARM variants.

.. _dgemm-example:

dgemm example
===============

This example illustrates how to efficiently offload the CBLAS DGEMM routine
(double precision matrix multiply) to the DSPs using OpenCL. The results
obtained on the DSP are compared against a cblas_dgemm call on the ARM. The
example reports performance in GFlops for both DSP and ARM variants.

.. _conv1d-example:

conv1d example
===============

This example illustrates step by step how to optimize a 1D convolution
kernel applied to 2D data.  The results obtained on the DSP are compared
against the same computation performed on the ARM.  Optimization techniques
include software pipelining improvement, SIMDization, and asynchronous
data movement with double buffering into faster memory to overlap computation
with data movement.  Details can be found in
:doc:`../optimization/example_conv1d`.

.. note::

   The conv1d example is available in Processor SDK version >= 3.3.

.. _edmamgr-example:

edmamgr example
=================

This application illustrates how to use the edmamgr API to asynchronously move
data around the DSP memory hierarchy from OpenCL C kernels. The edmamgr.h
header file in this directory enumerates the APIs available from the edmamgr
package.

.. _edmabw-example:

edmabw example
=================

This application measures the average data transfer times between different
memory regions (DDR, MSMC, L2 SRAM) for a DSP core using EDMA operations via
the async_work_group_copy API. It also demonstrates the use of sub devices via
the C++ API and the __dsp_frequency() builtin function within the OpenCL C
kernel.

.. _dspheap-example:

dspheap example
=================
This application illustrates how to use the user defined heaps feature to allow
C code called from OpenCL C code to define custom and use custom heaps on the DSP
devices.  See :doc:`../memory/dsp-malloc-extension`

.. _abort_exit-example:

abort_exit example
==================
This example illustrates how to call abort() or exit() in kernel code
for early kernel termination, and how to check corresponding kernel
event status to determine if abort() or exit() has been called.
Two extended kernel event status are ``CL_ERROR_KERNEL_ABORT_TI`` and
``CL_ERROR_KERNEL_EXIT_TI``.
Note that these two functions can be called from either OpenCL C code
or standard C code.

.. note::
  The latest TI RTOS migrated to use newlib-nano and disabled C++
  exceptions (`see limitations of newlib-nano libc <http://processors.wiki.ti.com/index.php/SYS/BIOS_with_GCC_(CortexA)#What_are_the_limitations_of_newlib-nano_libc_compared_to_newlib_libc_.3F>`_).
  As a result, in OpenCL RTOS setup, this example won't run to full completion.
  OpenCL Linux is not affected.

.. _timeout-example:

timeout example
=================
This example illustrates how to query the OpenCL device queue properties
for timeout extension, how to create a command queue with timeout
property, how to set a timeout on a kernel, and how to query kernel
event status to determine if a timeout has occurred.  Details of timeout
extension can be found in :doc:`../extensions/kernel-timeout`.

.. note::
  The latest TI RTOS migrated to use newlib-nano and disabled C++
  exceptions (`see limitations of newlib-nano libc <http://processors.wiki.ti.com/index.php/SYS/BIOS_with_GCC_(CortexA)#What_are_the_limitations_of_newlib-nano_libc_compared_to_newlib_libc_.3F>`_).
  As a result, in OpenCL RTOS setup, this example won't run to full completion.
  OpenCL Linux is not affected.

.. note::

   The following examples are available only available on 66AK2x

   * mandelbrot, mandelbrot_native
   * vecadd_mpax, vecadd_mpax_openmp (not available on 66AK2G)
