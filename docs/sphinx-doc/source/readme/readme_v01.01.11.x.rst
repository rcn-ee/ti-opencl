*************************
v01.01.11.x
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
* Support use of exit/abort in C code called from OpenCL C kernels

Defect Fixes
------------

* [MCT-499] Destruction of the Platform singleton causes a crash in a subsequent clReleaseProgram() during teardown of ViennaCL OCL examples
* [MCT-648] C++ I/O broken for OpenCL RTOS examples with 4Q BIOS
* [MCT-649] OOT being reported as IOT on K2E


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
