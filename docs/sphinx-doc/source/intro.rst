************
Introduction
************

OpenCL is a framework for writing programs that execute across heterogeneous
systems. This documentation describes the TI implementation of the 
Khronos `OpenCL 1.1 specification`_. The Texas Instruments OpenCL implementation is
currently supported on the following systems):

============= =============================== ============================== =========================
SoC           System                          Khronos Conformance            Installation Instructions                  
============= =============================== ============================== =========================
66AK2H_       `66AK2H EVM`_                   OpenCL v1.1 Conformant         `MCSDK-HPC for EVM`_
66AK2H_       `HP m800 Moonshot`_             OpenCL v1.1 Conformant         `MCSDK-HPC for m800`_
TMS320C6678_  `Advantech DSPC8681`_           Not submitted for conformance  :doc:`dspc8681-getting-started`
AM572x        Beagle Board X-15               Not submitted for conformance  Coming Soon
============= =============================== ============================== =========================


The OpenCL specification defines a platform model with a Host and
Compute Devices. The following table defines host and compute device for
TI OpenCL implementations:

===================== ========================================= =============================================
System                Host                                      Compute Device
===================== ========================================= =============================================
`66AK2H EVM`_         4 ARM Cortex-A15 CPUs, SMP Linux          1 device with 8 C66x DSP compute units
`HP m800 Moonshot`_   4 ARM Cortex-A15 CPUs, Ubuntu 14.04 Linux 1 device with 8 C66x DSP compute units
`Advantech DSPC8681`_ x86 based CPU, Ubuntu 12.04 Linux         4 devices, each with 8 C66X DSP compute units
Beagle Board X-15     2 ARM Cortex-A15 CPUs, SMP Linux          1 device with 2 C66x DSP compute units
===================== ========================================= =============================================

.. _Advantech DSPC8681: http://www2.advantech.com/products/HALF-LENGTH_PCIE_CARD1/DSP-8681/mod_1404A7C7-3680-4BA8-ABDB-0D218FFECA36.aspx
.. _66AK2H:             http://www.ti.com/product/66ak2h14
.. _66AK2H EVM:         http://www.ti.com/tool/EVMK2h
.. _HP m800 Moonshot:   http://www8.hp.com/us/en/products/moonshot-systems/product-detail.html?oid=6532018
.. _TMS320C6678:        http://www.ti.com/product/tms320c6678
.. _MCSDK-HPC for EVM:  http://processors.wiki.ti.com/index.php/MCSDK_HPC_3.x_Getting_Started_Guide
.. _MCSDK-HPC for m800: http://processors.wiki.ti.com/index.php/MCSDK_HPC_3.x_Getting_Started_Guide_for_HP_ProLiant_m800
.. _OpenCL 1.1 specification: https://www.khronos.org/registry/cl/specs/opencl-1.1.pdf
