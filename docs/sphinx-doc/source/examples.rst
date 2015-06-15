OpenCL Examples
***************

.. role:: cpp(code)
    :language: cpp

There are several OpenCL examples that are part of the OpenCL package installation. They are located in the /usr/share/ti/examples/opencl[+openmp] directories on the target file system.

The examples can be cross-compiled in an X86 development environment, or compiled native on the ARM A15, depending on the availability of native g++ or cross-compiled arm-linux-gnueabihf-g++ tool sets.

On an X86 development environment the example makefiles are setup to cross-compile by default and assume an ARM cross-compile environment has been installed. If the cross compiler is not installed, execute the following command to install it:

:command:`sudo apt-get install g++-4.6-arm-linux-gnueabihf`

The environment variables defined in [[OpenCL_Environment_Variables|Required Environment Variables]] are expected to be defined in order to compile and execute the examples.

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


.. note:: 

   The following examples are available only available on 66AK2H

   * mandelbrot, mandelbrot_native
   * vecadd_openmp, vecadd_openmp_t
   * vecadd_mpax, vecadd_mpax_openmp
   * sgemm
   * dgemm


