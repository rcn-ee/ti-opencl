*************************
v01.01.17.x
*************************

New Features
=============
* Reduce OpenCL runtime overhead by removing calls to setpriority and usleep in worker threads

  These calls were required when OpenCL worker threads used to busy-wait on
  mailbox receive APIs on K2x devices. With the transition to MessageQ, the
  receive APIs no longer busy wait and the setpriority/sleep calls are no
  longer required.

  Removing setpriority improves the response time of worker threads to
  kernel completion messages, reducing the overhead of the OpenCL runtime.

* Changes required for TIDL API v1.2

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
