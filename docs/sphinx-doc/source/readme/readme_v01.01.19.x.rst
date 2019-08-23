*************************
v01.01.19.x
*************************

New Features
=============
* OpenCL 1.2 compatible printf routine with support for vector data types
* Support for select OpenCL 1.2 APIs including:
    * clGetKernelArgInfo()
    * clEnqueueBarrierWithWaitList()
    * clEnqueueMigrateMemObject()
* Support for OpenCL 1.2 buffer creation flags
    * CL_MEM_HOST_WRITE_ONLY
    * CL_MEM_HOST_READ_ONLY
    * CL_MEM_HOST_NO_ACCESS
* Support for cl_map flag CL_MAP_WRITE_INVALIDATE_REGION

Supported Evaluation Modules (EVMs)
===================================
* `AM574x IDK EVM`_
* `AM572 EVM`_ (Linux and TI-RTOS hosts)
* `AM571 EVM`_
* `66AK2H EVM`_
* `66AK2L EVM`_
* `66AK2E EVM`_
* `66AK2G EVM`_

.. _AM572 EVM:          http://www.ti.com/tool/tmdsevm572x
.. _AM571 EVM:          http://www.ti.com/tool/tmdsevm572x
.. _AM574x IDK EVM:     http://www.ti.com/tool/tmdsidk574
.. _66AK2H EVM:         http://www.ti.com/tool/EVMK2H
.. _66AK2L EVM:         http://www.ti.com/tool/XEVMK2LX
.. _66AK2E EVM:         http://www.ti.com/tool/XEVMK2EX
.. _66AK2G EVM:         http://www.ti.com/tool/EVMK2G
