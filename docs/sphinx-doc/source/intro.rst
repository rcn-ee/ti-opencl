************
Introduction
************

OpenCL is a framework for writing programs that execute across heterogeneous
systems. This documentation describes the TI implementation of the Khronos
OpenCL 1.1 specification. The Texas Instruments OpenCL implementation is
currently supported on the following SoCs (System-On-Chip):

============= =============================== ==============================
SoC           System                          Khronos Conformance                             
============= =============================== ==============================
66AK2H_       `66AK2H EVM`_                   Conformant to OpenCL v1.1                       
66AK2H_       `HP m800 Moonshot`_             Conformant to OpenCL v1.1                       
TMS320C6678_  `Advantech DSPC8681 PCIe card`_ Not submitted for conformance                   
AM572x        Beagle Board X-15               Not submitted for conformance                   
============= =============================== ==============================

The OpenCL specification defines a platform model with a Host and
Compute Devices. The following table defines host and compute device for
TI OpenCL implementations:

=============================== ========================================= =============================================
System                          Host                                      Compute Device
=============================== ========================================= =============================================
`66AK2H EVM`_                   4 ARM Cortex-A15 CPUs, SMP Linux          1 device with 8 C66x DSP compute units
`HP m800 Moonshot`_             4 ARM Cortex-A15 CPUs, Ubuntu 14.04 Linux 1 device with 8 C66x DSP compute units
`Advantech DSPC8681 PCIe card`_ x86 based CPU, Ubuntu 12.04 Linux         4 devices, each with 8 C66X DSP compute units
Beagle Board X-15               2 ARM Cortex-A15 CPUs, SMP Linux          1 device with 2 C66x DSP compute units
=============================== ========================================= =============================================

.. _Advantech DSPC8681 PCIe card: http://www2.advantech.com/products/HALF-LENGTH_PCIE_CARD1/DSP-8681/mod_1404A7C7-3680-4BA8-ABDB-0D218FFECA36.aspx
.. _66AK2H:                       http://www.ti.com/product/66ak2h14
.. _66AK2H EVM:                   http://www.ti.com/tool/EVMK2h
.. _HP m800 Moonshot:             http://www8.hp.com/us/en/products/moonshot-systems/product-detail.html?oid=6532018
.. _TMS320C6678:                  http://www.ti.com/product/tms320c6678
