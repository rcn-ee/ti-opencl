Examples
********

There are several OpenCL examples that are part of the OpenCL package installation. They are located in the /usr/share/ti/examples/opencl[+openmp] directories on the target file system.

The examples can be cross-compiled in an X86 development environment, or compiled native on the ARM A15, depending on the availability of native g++ or cross-compiled arm-linux-gnueabihf-g++ tool sets.

On an X86 development environment the example makefiles are setup to cross-compile by default and assume an ARM cross-compile environment has been installed. If the cross compiler is not installed, execute the following command to install it:

:command:`sudo apt-get install g++-4.6-arm-linux-gnueabihf`


Building and Running the Examples
=================================

All the examples can be built at one time by invoking 'make' from the top-level directory where the OpenCL examples are installed/copied to. Individual examples can be built by navigating to the desired directory and also issuing 'make'.
<br>

Descriptions
============

+-------------------------+------------------------------------------------------------------------------------------------+
| Sample Application      | Details                                                                                        |
+-------------------------+------------------------------------------------------------------------------------------------+
| platforms               | This example uses the OpenCL C++ bindings to discover key platform and device information from |
|                         | the OpenCL implementation and print it to the screen.                                          |
+-------------------------+------------------------------------------------------------------------------------------------+
| simple                  | This is a 'hello world' type of example that illustrates the minimum steps needed to dispatch  |
|                         | a kernel to a DSP device and read a buffer of data back.                                       |
+-------------------------+------------------------------------------------------------------------------------------------+
| mandelbrot and          | The 'mandelbrot' example is an OpenCL demo that uses OpenCL to generate the pixels of a        |
| mandelbrot_native       | Mandelbrot set image. This example also uses the C++ OpenCL binding. The OpenCL kernels are    |
|                         | repeatedly called generating images that are zoomed in from the previous image. This repeats   |
|                         | until the zoom factor reaches 1E15.                                                            |
|                         |                                                                                                |
|                         | This example illustrates several key OpenCL features:                                          |
|                         |                                                                                                |
|                         | - OpenCL queues tied to potentially multiple DSP devices and a dispatch structure that allows  |
|                         |   the DSPs to cooperatively generate pixel data,                                               |
|                         | - The event wait feature of OpenCL,                                                            |
|                         | - The division of one time setup of OpenCL to the repetitive en-queuing of kernels, and        |
|                         | - The ease in which kernels can be shifted from one device type to another.                    |
|                         |                                                                                                |
|                         | The 'mandelbrot_native' example is non-OpenCL native implementation (no dispatch to the DSPs)  |
|                         | that can be used for comparison purposes. It uses OpenMP for dispatch to each ARM core.        |
|                         | Note: The display of the resulting Mandelbrot images is currently disabled when run on the     |
|                         | default EVM Linux file system included in the MCSDK. Instead it will output frame information. |
+-------------------------+------------------------------------------------------------------------------------------------+
| ccode                   | This example illustrates the TI extension to OpenCL that allows OpenCL C code to call standard |
|                         | C code that has been compiled off-line into an object file or static library. This mechanism   |
|                         | can be used to allow optimized C or C callable assembly routines to be called from OpenCL C    |
|                         | code. It can also be used to essentially dispatch a standard C function, by wrapping it with   |
|                         | an OpenCL C wrapper. Calling C++ routines from OpenCL C is not yet supported. You should also  |
|                         | ensure that the standard C function and the call tree resulting from the standard C function   |
|                         | do not allocate device memory, change the cache structure, or use any resources already being  |
|                         | used by the OpenCL runtime.                                                                    |
+-------------------------+------------------------------------------------------------------------------------------------+
| matmpy                  | This example performs a 1K x 1K matrix multiply using both OpenCL and a native ARM OpenMP      |
|                         | implementation (GCC libgomp). The output is the execution time for each approach               |
|                         | ( OpenCL dispatch to the DSP vs. OpenMP dispatching to the 4 ARM A15s ).                       |
+-------------------------+------------------------------------------------------------------------------------------------+
| offline                 | This example performs a vector addition by pre-compiling an OpenCL kernel into a device        |
|                         | executable file. The OpenCL program reads the file containing the pre-compiled kernel in and   |
|                         | uses it directly. If you use offline compilation to generate a .out file containing the        |
|                         | OpenCL C program and you subsequently move the executable, you will either need to move the    |
|                         | .out as well or the application will need to specificy a non relative path to the .out file.   |
+-------------------------+------------------------------------------------------------------------------------------------+
| vecadd_openmp           | This is an OpenCL + OpenMP example. OpenCL program is running on the host, managing data       |
|                         | transfers, and dispatching an OpenCL wrapper kernel to the device. The OpenCL wrapper kernel   |
|                         | will use the ccode mode (see ccode example) to call the C function that has been compiled with |
|                         | OpenMP options (omp). To facilitate OpenMP mode, the OpenCL wrapper kernel needs to be         |
|                         | dispatched as an OpenCL Task to an In-Order OpenCL Queue.                                      |
+-------------------------+------------------------------------------------------------------------------------------------+
| vecadd_openmp_t         | This is another OpenCL + OpenMP example, similar to vecadd_openmp. The main difference w.r.t   |
|                         | vecadd_openmp is that this example uses OpenMP tasks within the OpenMP parallel region to      |
|                         | distribute computation across the DSP cores.                                                   |
+-------------------------+------------------------------------------------------------------------------------------------+
| vecadd                  | The same functionality as the vecadd_openmp example, but expressed fully as an OpenCL          |
|                         | application without OpenMP. Included for comparison purposes.                                  |
+-------------------------+------------------------------------------------------------------------------------------------+
| vecadd_mpax             | The same functionality as the vecadd example, but with extended buffers. The example           |
|                         | iteratively traverses smaller chunks (sub-buffers) of large buffers. During each iteration,    |
|                         | the smaller chunks are mapped/unmapped for read/write. The sub-buffers are then passed to the  |
|                         | kernels for processing. This example could also be converted to use a pipelined scheme where   |
|                         | different iterations of CPU computation and device computation are overlapped. NOTE: The size  |
|                         | of the buffers in the example (determined by the variable 'NumElements') is dependent on the   |
|                         | available CMEM block size. Currently this example is configured to use buffers sizes for       |
|                         | memory configurations that can support 1.5 GB total buffer size. The example can be modified   |
|                         | to use more (or less) based on the platform memory configuration.                              |
+-------------------------+------------------------------------------------------------------------------------------------+
| vecadd_mpax_openmp      | Similar to vecadd_mpax example, but used OpenMP to perform the parallelization and the         |
|                         | computation. This example also illustrates that printf() could be used in OpenMP C code        |
|                         | for debugging.                                                                                 |
+-------------------------+------------------------------------------------------------------------------------------------+
| dsplib_fft              | An example to compute FFT's using a routine from the dsplib library. This illustrates calling  |
|                         | a standard C library function from an OpenCL kernel.                                           |
+-------------------------+------------------------------------------------------------------------------------------------+
| ooo, ooo_map            | This application illustrates several features of OpenCL.                                       |
|                         |                                                                                                |
|                         | - Using a combination of In-Order and Out-Of-Order queues                                      |
|                         | - Using native kernels on the CPU                                                              |
|                         | - Using events to manage dependencies among the tasks to be executed. A JPEG in this           |
|                         |   directory illustrates the dependence graph being enforced in the application using events.   |
|                         |                                                                                                |
|                         | The ooo_map version additionally illustrates the use of OpenCL map and unmap operations for    |
|                         | accessing shared memory between a host and a device. The Map/Unmap protocol can be used        |
|                         | instead of read/write protocol on shared memory platforms.                                     |
|                         |                                                                                                |
|                         | Requires the  TI_OCL_CPU_DEVICE_ENABLE environment variable to be set. For details, refer      |
|                         | :doc:`environment_variables`                                                                   |
+-------------------------+------------------------------------------------------------------------------------------------+
| null                    | This application is intended to report the time overhead that OpenCL requires to submit and    |
|                         | dispatch a kernel. A null(empty) kernel is created and dispatched so that the OpenCL profiling |
|                         | times queried from the OpenCL events reflects only the OpenCL overhead necessary to submit and |
|                         | execute the kernel on the device. This overhead is for the roundtrip for a single kernel       |
|                         | dispatch. In practice, when multiple tasks are being enqueued, this overhead is pipelined      |
|                         | with execution and can approach zero.                                                          |
+-------------------------+------------------------------------------------------------------------------------------------+
| sgemm                   | This example illustrates how to efficiently offload the CBLAS SGEMM routine (single precision  |
|                         | matrix multiply) to the DSPs using OpenCL. The results obtained on the DSP are compared        |
|                         | against a cbas_sgemm call on the ARM. The example reports performance in GFlops for both DSP   |
|                         | and ARM variants.                                                                              |
+-------------------------+------------------------------------------------------------------------------------------------+
| dgemm                   | This example illustrates how to efficiently offload the CBLAS DGEMM routine (double precision  |
|                         | matrix multiply) to the DSPs using OpenCL. The results obtained on the DSP are compared        |
|                         | against a cbas_dgemm call on the ARM. The example reports performance in GFlops for both       |
|                         | DSP and ARM variants.                                                                          |
+-------------------------+------------------------------------------------------------------------------------------------+
| edmamgr                 | This application illustrates how to use the edmamgr api to asynchronously move data around     |
|                         | the DSP memory hierarchy from OpenCL C kernels. The edmamgr.h header file in this directory    |
|                         | enumerates the APIs available from the edmamgr package.                                        |
+-------------------------+------------------------------------------------------------------------------------------------+

.. note:: 

   The following examples are available only available on 66AK2H

   * mandelbrot, mandelbrot_native
   * vecadd_openmp, vecadd_openmp_t
   * vecadd_mpax, vecadd_mpax_openmp
   * sgemm
   * dgemm


