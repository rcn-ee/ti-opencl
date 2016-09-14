********************
Building and Running
********************

There are several OpenCL examples that are part of the OpenCL package
installation. They are located in the ``/usr/share/ti/examples/opencl[+openmp]``
directories on the target file system.

The examples can be cross-compiled in an X86 development environment, or
compiled natively on Linux running on the Cortex-A15s.

Native Compilation on the EVM
=============================
All the examples can be built by invoking 'make' from the top-level
directory where the OpenCL examples are installed/copied to. Individual
examples can be built by navigating to the desired directory and issuing
'make'.

Cross-Compiling
===============
On an X86/Linux development environment the example makefiles are setup to
cross-compile by default. 

Required Environment Variables 
++++++++++++++++++++++++++++++

.. envvar::  TARGET_ROOTDIR 

    Points to the linux devkit. 
    E.g. ``<Processor SDK Linux install path>/linux-devkit/sysroots/armv7ahf-neon-linux-gnueabi`` 
    Please check your SDK install, the name of the directory under sysroots can change with SDK versions.

.. envvar::  TI_OCL_CGT_INSTALL         

    The OpenCL runtime is dependent on the C66x DSP compiler product for
    the compilation of OpenCL C kernels. When OpenCL C kernels are compiled on the
    target ARM/Linux system, the C66x compiler is assumed to be installed in the
    standard Linux locations. However, off-line cross compilation of OpenCL C
    kernels is also supported from x86 Ubuntu machines and in that use case, it
    is required that this environment variable is set to the top level
    directory path where the C66x cross compiler tools are installed. 


Additions to the path
+++++++++++++++++++++
#. ``<Processor SDK Linux install path>/linux-devkit/sysroots/x86_64-arago-linux/usr/bin`` to get the cross compilers (e.g. arm-linux-gnueabihf-g++).
#. ``$TI_OCL_CGT_INSTALL/bin`` to get the TI C6000 code generation tools (cl6x).

After setting the environment variables described above and making the additions to your path, the examples can be built by invoking 'make' from examples directory.


