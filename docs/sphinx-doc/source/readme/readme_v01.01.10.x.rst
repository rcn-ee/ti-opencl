*************************
OpenCL v01.01.10.x Readme
*************************

Platforms supported
===================

* `AM572 EVM`_ (Linux and RTOS)
* AM571x
* `66AK2H EVM`_
* `66AK2L EVM`_
* `66AK2E EVM`_
* `66AK2G EVM`_


Release Notes
=============
* Added support for AM571x
* AM57x: Add DSP performance monitoring support for OpenCL applications 

Defect Fixes
------------
* [MCT-566] - C66 intrinsics not available in OpenCL C
* [MCT-571] - dsptop does not work on AM57 with DSP suspend/resume enabled
* [MCT-536] - clocl creates .dsp_h file even if link to build .out file fails

* [MCT-516] - OpenCL-RTOS: Exit ARM and DSP programs on completion
* [MCT-572] - Defect in OpenCL CMEM error message regarding number of CMEM blocks required
* [MCT-575] - OpenCL-RTOS: clocl shipped with RTOS does not find the correct version of libstdc++.so.6
* [MCT-593] - MessageQ error results in debugss error messages on subsequent run
* [MCT-594] - OpenCL matmpy intermittent DSP1 crash due to EdmaMgr issues w/ suspend/resume
* [MCT-597] - Race condition in OpenCL cleanup code.

Compiler Versions
=================
This release requires the following compiler versions:

========           ========
Compiler           Version
========           ========
ARM GCC            5.3.1 (arm-linux-gnueabi)
ARM GCC            4.9.3 (arm-none-eabi, for OpenCL over TI-RTOS)
C6000              8.1.0
========           ========


.. _AM572 EVM:          http://www.ti.com/tool/tmdsevm572x
.. _66AK2H EVM:         http://www.ti.com/tool/EVMK2H
.. _66AK2L EVM:         http://www.ti.com/tool/XEVMK2LX
.. _66AK2E EVM:         http://www.ti.com/tool/XEVMK2EX
.. _66AK2G EVM:         http://www.ti.com/tool/EVMK2G
