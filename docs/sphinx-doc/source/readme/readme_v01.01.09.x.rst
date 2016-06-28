*************************
OpenCL v01.01.09.x Readme
*************************

Platforms supported
===================

* `AM572 EVM`_ (Linux and RTOS)
* `66AK2H EVM`_
* `66AK2L EVM`_
* `66AK2E EVM`_
* `66AK2G EVM`_


Release Notes
=============
* AM572x: Added support for the OpenCL over TI-RTOS on Cortex-A15(s).
* AM572x: Enable building RTOS OpenCL examples on Windows (requires C6000 CGT 8.1.1)
* AM572x: OpenCL runtime configurable with respect to number of DSP cores using environment variable TI_OCL_COMPUTE_UNIT_LIST 
* AM572x: Out-of-order task scheduler improved to better utilize both DSP cores when available.

Defect Fixes
------------
* [MCT-489] - Crash in barrier when offloading openmp code from opencl
* [MCT-497] - A NULL value passed as a kernel buffer argument causes crash in isSubBufferAligned()

Compiler Versions
=================
This release requires the following compiler versions:

========           ========
Compiler           Version
========           ========
ARM GCC            5.3.1 (arm-linux-gnueabi)
ARM GCC            4.9.3 (arm-none-eabi, for OpenCL over TI-RTOS)
C6000              8.1.x
========           ========


.. _AM572 EVM:          http://www.ti.com/tool/tmdsevm572x
.. _66AK2H EVM:         http://www.ti.com/tool/EVMK2H
.. _66AK2L EVM:         http://www.ti.com/tool/XEVMK2LX
.. _66AK2E EVM:         http://www.ti.com/tool/XEVMK2EX
.. _66AK2G EVM:         http://www.ti.com/tool/EVMK2G
