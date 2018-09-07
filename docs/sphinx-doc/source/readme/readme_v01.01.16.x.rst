*************************
v01.01.16.x
*************************

New Features
=============
* Export range based cache wb/inv functions for use from DSP device code

  .. code-block:: c++

        void     __cache_l2_wbinv  (uint8_t* __ptr, size_t __size);
        void     __cache_l2_inv    (uint8_t* __ptr, size_t __size);

* EVE frequency - put EVE in same OPP mode as C66x DSP. E.g. Default DSP is OPP_HIGH, then put EVE in OPP_HIGH as well - 650MHz.

* Changes required for TIDL API v1.1

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
