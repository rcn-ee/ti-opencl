*************************
v01.01.15.x
*************************

New Features
=============
* Support for OpenCL sub-devices (see ``examples/vecadd_subdevice``)
* OpenCL custom device for TI Deep Learning Library (TIDL)

Defect Fixes
============
* [MCT-855] Invoking clEnqueueNDRangeKernel with local_work_size set to NULL results in segmentation fault
* [MCT-809] Multiple processes offloading kernels containing OpenMP code crash the monitor

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
ARM GCC            7.2.1 (arm-linux-gnueabi)
ARM GCC            4.9.3 (arm-none-eabi, for OpenCL over TI-RTOS)
C6000              8.2.2
========           ========


.. _AM572 EVM:          http://www.ti.com/tool/tmdsevm572x
.. _AM571 EVM:          http://www.ti.com/tool/tmdsevm572x
.. _66AK2H EVM:         http://www.ti.com/tool/EVMK2H
.. _66AK2L EVM:         http://www.ti.com/tool/XEVMK2LX
.. _66AK2E EVM:         http://www.ti.com/tool/XEVMK2EX
.. _66AK2G EVM:         http://www.ti.com/tool/EVMK2G
