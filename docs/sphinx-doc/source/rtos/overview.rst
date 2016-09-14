******************************************************
Overview
******************************************************

OpenCL on RTOS Package
=============================================
OpenCL on RTOS is being shipped as a component in `Processor SDK TI-RTOS`_
release.  After the SDK is downloaded and installed, OpenCL package resides
inside the installed SDK directory (aka folder on Windows), for example,
ti-processor-sdk-rtos-am57xx-evm-03.00.00.03/opencl_rtos_am57xx_01_01_09_01.

Inside the OpenCL directory, ``examples`` directory contains a set of pre-built
OpenCL on RTOS examples that can be loaded onto the hardware for execution,
and can be used as templates by users for developing their own OpenCL
applications on RTOS.

``packages`` directory contains a conventional RTOS RTSC package as well as
other OpenCL runtime related contents.  The OpenCL RTSC package provides a
``ti.opencl.OpenCL`` module that is used on the host/A15 side, and a
``ti.opencl.DSPMonitor`` module that is used on the device/DSP side.  Other
OpenCL runtime related contents will be explained when we discuss how to run
examples and how to use OpenCL RTOS package to develop your own OpenCL
application.

.. _Processor SDK TI-RTOS: http://www.ti.com/tool/ti-rtos-proc


Running Examples Shipped with OpenCL Package
=============================================
We recommend downloading and installing the version of Code Composer Studio (CCS) specified in the Processor SDK RTOS release notes 
before running OpenCL RTOS examples and starting OpenCL RTOS application
development.  From the installed CCS, create the target configuration using
your emulator and EVM, launch the target configuration (see
:doc:`../debug/debug_ccs`).  Once launched, connect to CortexA15_0 first.
With CortexA15_0 being connected and highlighted, click "Scripts", "CLOCK
Configuration", "OPPHIGH" to set EVM frequency into high performance mode,
then click "Scripts", "MULTICORE Initialization", "Enable all cores" to
enable other cores on the SoC.  Connect to DSP1 and DSP2.  Now you are ready
to run pre-built OpenCL RTOS examples.

We use the ``vecadd`` example to illustrate the process of loading and running
in CCS.  Here are the steps:

1. Click DSP1, click "Run", "Load", "Load program", browse for
   ``packages/ti/opencl/usr/share/ti/opencl/dsp0.out`` and load it.
2. Click DSP2, click "Run", "Load", "Load program", browse for
   ``packages/ti/opencl/usr/share/ti/opencl/dsp1.out`` and load it.
3. Click CortexA15_0, click "Run", "Load", "Load program", browse for
   ``examples/vecadd/bin/release/vecadd.xa15fg`` and load it.
4. Click DSP1, click "Run", "Resume", do the same for DSP2 and CortexA15_0.
5. You will see the application output on the CCS console.

You can repeat the above process and select a different CortexA15_0 executable
from a different example's ``bin/release`` directory to run a different
example.

.. Note::
  Due to a known issue, you might have to terminate current
  CCS debug session before you can run another example.  We will resolve this
  known issue in the future OpenCL RTOS product releases.  The steps to
  terminate current CCS debug session are: Click CortexA15_0, click "Run",
  "Reset", "System Reset", click "Run", "Terminate", in CCS Edit interface,
  launch the target configuration and repeat the steps in this section for
  running another OpenCL RTOS example.
