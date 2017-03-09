*************************
v01.01.12.x
*************************

New Features
=============
* Multiprocess support on Linux (refer :doc:`../multiprocess` for details)
* Added the ability to specify a timeout for OpenCL kernels (refer :doc:`../extensions/kernel-timeout` for details)
* Switched K2H, K2L and K2E to using IPC message queues for ARM-DSP communication

Defect Fixes
============
* [MCT-692] Fixed wrong results with sgemm example on K2H due to EDMA operations exceeding boundaries of matrices.

Supported Evaluation Modules (EVMs)
===================================
* `AM572 EVM`_ (Linux and TI-RTOS hosts)
* `AM571 EVM`_ 
* `66AK2H EVM`_
* `66AK2L EVM`_
* `66AK2E EVM`_
* `66AK2G EVM`_

Compiler Versions
=================
This release requires the following compiler versions:

========           ========
Compiler           Version
========           ========
ARM GCC            5.3.1 (arm-linux-gnueabi)
ARM GCC            4.9.3 (arm-none-eabi, for OpenCL over TI-RTOS)
C6000              8.1.3
========           ========


.. _AM572 EVM:          http://www.ti.com/tool/tmdsevm572x
.. _AM571 EVM:          http://www.ti.com/tool/tmdsevm572x
.. _66AK2H EVM:         http://www.ti.com/tool/EVMK2H
.. _66AK2L EVM:         http://www.ti.com/tool/XEVMK2LX
.. _66AK2E EVM:         http://www.ti.com/tool/XEVMK2EX
.. _66AK2G EVM:         http://www.ti.com/tool/EVMK2G
