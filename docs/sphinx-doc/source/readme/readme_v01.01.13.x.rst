*************************
v01.01.13.x
*************************

New Features
=============
* Added support for configuring the Multicore tools daemon using a JSON file
* Added a section to OpenCL User's Guide describing how to offload to DSPs  

Defect Fixes
============
* [MCT-631] ViennaCL: sparse fails with Code generator does not support intrinsic
* [MCT-646] float_compute example prints fixed assumption of 2 CPU cores

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
ARM GCC            6.2.1 (arm-linux-gnueabi)
ARM GCC            4.9.3 (arm-none-eabi, for OpenCL over TI-RTOS)
C6000              8.1.3
========           ========


.. _AM572 EVM:          http://www.ti.com/tool/tmdsevm572x
.. _AM571 EVM:          http://www.ti.com/tool/tmdsevm572x
.. _66AK2H EVM:         http://www.ti.com/tool/EVMK2H
.. _66AK2L EVM:         http://www.ti.com/tool/XEVMK2LX
.. _66AK2E EVM:         http://www.ti.com/tool/XEVMK2EX
.. _66AK2G EVM:         http://www.ti.com/tool/EVMK2G
