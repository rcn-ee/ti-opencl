******************************************************
Basic OpenCL RTOS Application Development
******************************************************
A typical RTOS application development involves configuring many aspects
of the system and application, such as platform configuration, memory
configuration, IPC configuration, tasks running on each core, and so on.  
Basic OpenCL RTOS application development hides all these configurations
away from the user by using the default configurations shipped with the
OpenCL RTOS package.  User will only need to focus on the main application
running on the host side and the OpenCL kernels that will be dispatched to
the device side.  With a slightly modified Makefile.rtos from any existing
examples, user can build and run the OpenCL RTOS application.  In what
follows, we'll look at the building steps first.

Building Application on Linux
============================================
In each OpenCL RTOS example, there is a ``Makefile.rtos`` file.  They all
include a shared ``make_rtos.inc`` in the parent ``examples`` directory.
You need to edit ``make_rtos.inc`` to point the following environment
variables to the correct installation location on your system:
``DEFAULT_PSDK_RTOS``, ``GCC_ARM_NONE_TOOLCHAIN``, ``TI_OCL_CGT_INSTALL``
and ``TI_OCL_INSTALL``.  After that, building each example is as simple as
``make -f Makefile.rtos``, and cleaning the build is as simple as
``make -f Makefile.rtos clean``.  The built host executable is place in the
``bin/release`` sub-directory.

Building Application on Windows
============================================
Building on Windows requires one extra setup step.  You need to
download and install the 
`Windows version of TI C6000 compiler tools version 8.1.1`_.  Similar to
building on Linux, you need to edit ``examples\make_rtos.inc`` to point
the environment variables to correct installation location.  Once setup
is done, building each example is as simple as running
``C:\ti\ccsv6\utils\bin\gmake.exe -f Makefile.rtos`` inside each example
and cleaning is as simple as
``C:\ti\ccsv6\utils\bin\gmake.exe -f Makefile.rtos clean``.

.. _Windows version of TI C6000 Compiler Tools version 8.1.1: http://software-dl.ti.com/codegen/non-esd/downloads/download.htm

Creating an OpenCL RTOS Application
============================================
We recommend using an existing OpenCL RTOS example as a template to develop
your own OpenCL RTOS application (i.e. make a copy).  You will only need to
modify three files:

1. Makefile.rtos: you can rename the executable, host source and kernel source
   files.
2. ``*.cpp``: you can modify the ``ocl_main`` function to contain your own
   OpenCL host side code.
3. ``*.cl``: you can replace existing kernel functions with your own OpenCL
   kernels.

Once modification is done, you can build, load and run your application as
described previously.

Limited Customization: Participating DSP Core(s)
================================================
By default, OpenCL on RTOS on AM57 utilizes both DSP core 0 and core 1 as
OpenCL compute units.  If you wish to use only one core, you can customize
which core is participating. ``vecadd`` example illustrates such a use case.
If you set environment variable ``CORE0_ONLY`` before building ``vecadd``
example, ``make -f Makefile.rtos clean; CORE0_ONLY=1 make -f Makefile.rtos``,
then you only need to load built program onto CortexA15_0 and dsp0.out to
DSP1 and leave DSP2 to other purposes.  If you examine
``examples/vecadd/Makefile.rtos``, you will notice that when ``CORE0_ONLY`` is
set, a customized host config file is used.  This customized host config file
is copied from the default config file ``packages/ti/opencl/Host.cfg`` with
the modification that changes ``OpenCL.computeUnitList`` from default
``"0,1"`` to ``"0"``

.. code-block:: diff

    @@ -87,6 +90,7 @@
     xdc.global.SR0_cacheEnable = true;
     xdc.global.oclProcName = "HOST";
     var OpenCL = xdc.useModule('ti.opencl.OpenCL');
    +OpenCL.computeUnitList = "0";
     
     /* select ipc libraries */
     var Build = xdc.useModule('ti.sdo.ipc.Build');

Differences from OpenCL Linux (Host running Linux)
================================================================
If you have developed OpenCL application using TI's Processor SDK Linux
before, you will notice the following major differences between OpenCL Linux
application and OpenCL RTOS application:

1. TI-RTOS host program needs a separate main function that creates tasks
   and calls BIOS_start().  What used to be the main function in OpenCL Linux
   host program now becomes the task function in OpenCL RTOS host program.

2. OpenCL RTOS can only support offline compiled kernels whose binary 
   are embedded in the host program.

3. DSP programs are automatically loaded and run by either Linux or OpenCL
   host runtime for an OpenCL Linux application, while they have to be
   loaded and run manually by you in CCS for an OpenCL RTOS application.

