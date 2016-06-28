************
Introduction
************


OpenCL is a framework for writing programs that execute across heterogeneous
systems. This documentation describes the TI implementation of the 
Khronos `OpenCL 1.1 specification`_. The Texas Instruments OpenCL implementation is currently supported on the following systems:

============= =============================== ============================== =========================
SoC           System                          Khronos Conformance            Installation Instructions
============= =============================== ============================== =========================
AM572_         `AM572 EVM`_                   OpenCL v1.1 Conformant         `Processor SDK for AM57x`_
66AK2H_       `66AK2H EVM`_                   OpenCL v1.1 Conformant         `Processor SDK for K2H`_
66AK2L_       `66AK2L EVM`_                   Not submitted for conformance  `Processor SDK for K2L`_
66AK2E_       `66AK2E EVM`_                   Not submitted for conformance  `Processor SDK for K2E`_
66AK2G_       `66AK2G EVM`_                   Not submitted for conformance  `Processor SDK for K2G`_
TMS320C6678_  `Advantech DSPC8681`_           Not submitted for conformance  :doc:`dspc8681-getting-started`
============= =============================== ============================== =========================


The OpenCL specification defines a platform model with a Host and
Compute Devices. The following table defines host and compute device for
TI OpenCL implementations:

===================== ========================================= =============================================
System                Host                                      Compute Device
===================== ========================================= =============================================
`AM572 EVM`_          2 ARM Cortex-A15 CPUs, SMP Linux          1 device with 2 C66x DSP compute units
`AM572 EVM`_          2 ARM Cortex-A15 CPUs, TI RTOS            1 device with 2 C66x DSP compute units
`66AK2H EVM`_         4 ARM Cortex-A15 CPUs, SMP Linux          1 device with 8 C66x DSP compute units
`66AK2L EVM`_         2 ARM Cortex-A15 CPUs, SMP Linux          1 device with 4 C66x DSP compute units
`66AK2E EVM`_         4 ARM Cortex-A15 CPUs, SMP Linux          1 device with 1 C66x DSP compute unit
`66AK2G EVM`_         1 ARM Cortex-A15 CPU, SMP Linux           1 device with 1 C66x DSP compute unit
`Advantech DSPC8681`_ x86 based CPU, Ubuntu 12.04 Linux         4 devices, each with 8 C66X DSP compute units
===================== ========================================= =============================================


.. _Advantech DSPC8681: http://www2.advantech.com/products/HALF-LENGTH_PCIE_CARD1/DSP-8681/mod_1404A7C7-3680-4BA8-ABDB-0D218FFECA36.aspx
.. _66AK2H:             http://www.ti.com/product/66ak2h14
.. _66AK2L:             http://www.ti.com/product/66ak2l06
.. _66AK2E:             http://www.ti.com/product/66ak2e05
.. _66AK2G:             http://www.ti.com/product/66ak2g02
.. _66AK2H EVM:         http://www.ti.com/tool/EVMK2H
.. _66AK2L EVM:         http://www.ti.com/tool/XEVMK2LX
.. _66AK2E EVM:         http://www.ti.com/tool/XEVMK2EX
.. _66AK2G EVM:         http://www.ti.com/tool/EVMK2G
.. _HP m800 Moonshot:   http://www8.hp.com/us/en/products/moonshot-systems/product-detail.html?oid=6532018
.. _TMS320C6678:        http://www.ti.com/product/tms320c6678
.. _MCSDK-HPC for EVM:  http://processors.wiki.ti.com/index.php/MCSDK_HPC_3.x_Getting_Started_Guide
.. _MCSDK-HPC for m800: http://processors.wiki.ti.com/index.php/MCSDK_HPC_3.x_Getting_Started_Guide_for_HP_ProLiant_m800
.. _OpenCL 1.1 specification: https://www.khronos.org/registry/cl/specs/opencl-1.1.pdf
.. _AM572:              http://www.ti.com/product/AM5728
.. _AM572 EVM:          http://www.ti.com/tool/tmdsevm572x
.. _Processor SDK for AM57x:          http://www.ti.com/tool/processor-sdk-am57x
.. _Processor SDK for K2H:            http://www.ti.com/tool/processor-sdk-k2h
.. _Processor SDK for K2L:            http://www.ti.com/tool/processor-sdk-k2l
.. _Processor SDK for K2E:            http://www.ti.com/tool/processor-sdk-k2e
.. _Processor SDK for K2G:            http://www.ti.com/tool/processor-sdk-k2g
.. _Processor SDK:  http://www.ti.com/lsds/ti/tools-software/processor_sw.page
