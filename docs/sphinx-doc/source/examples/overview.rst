********************
Example Descriptions
********************

.. _platforms-example:

platforms example
====================

This example uses the OpenCL C++ bindings to discover key platform and device
information from the OpenCL implementation and print it to the screen.

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
file system included in the MCSDK. Instead it will output frame information. 

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

.. _dsplib_fft-example:

dsplib_fft example
===================

An example to compute FFTs using a routine from the dsplib library. This
illustrates calling a standard C library function from an OpenCL kernel.

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

.. _sgemm example:

sgemm example
================

This example illustrates how to efficiently offload the CBLAS SGEMM routine
(single precision matrix multiply) to the DSPs using OpenCL. The results
obtained on the DSP are compared against a cblas_sgemm call on the ARM. The
example reports performance in GFlops for both DSP and ARM variants.

.. _dgemm example:

dgemm example
===============

This example illustrates how to efficiently offload the CBLAS DGEMM routine
(double precision matrix multiply) to the DSPs using OpenCL. The results
obtained on the DSP are compared against a cblas_dgemm call on the ARM. The
example reports performance in GFlops for both DSP and ARM variants.

.. _edmamgr-example:

edmamgr example
=================

This application illustrates how to use the edmamgr API to asynchronously move
data around the DSP memory hierarchy from OpenCL C kernels. The edmamgr.h
header file in this directory enumerates the APIs available from the edmamgr
package.

.. _dspheap-example:

dspheap example
=================
This application illustrates how to use the user defined heaps feature to allow 
C code called from OpenCL C code to define custom and use custom heaps on the DSP
devices.  See :doc:`../memory/dsp-malloc-extension`

.. note:: 

   The following examples are available only available on 66AK2x

   * mandelbrot, mandelbrot_native
   * vecadd_openmp, vecadd_openmp_t
   * vecadd_mpax, vecadd_mpax_openmp
   * sgemm
   * dgemm
