Introduction
************


OpenCL is a framework for writing programs that execute across heterogeneous systems. This documentation describes the TI implementation of the Khronos OpenCL 1.1 specification. The Texas Instruments OpenCL implementation is currently supported on the following SoCs (System-On-Chip):

+---------------------------------------------------+----------------------------------------------------------------------------------------------------------------+------------------------------------------------+
| SoC                                               | System                                                                                                         | Khronos Conformance                            |
+---------------------------------------------------+----------------------------------------------------------------------------------------------------------------+------------------------------------------------+
| `66AK2H <http://www.ti.com/product/66ak2h14>`__   | -  `66AK2H EVM <http://www.ti.com/tool/EVMK2h>`__, or                                                          | Conformant to OpenCL v1.1                      |
|                                                   | -  `HP m800 Moonshot <http://www8.hp.com/us/en/products/moonshot-systems/product-detail.html?oid=6532018>`__   |                                                |
+---------------------------------------------------+----------------------------------------------------------------------------------------------------------------+------------------------------------------------+
| AM572x                                            | Coming Soon                                                                                                    | Not submitted for conformance                  |
+---------------------------------------------------+----------------------------------------------------------------------------------------------------------------+------------------------------------------------+

The OpenCL specification defines a platform model with a Host and
Compute Devices. The following table defines host and compute device for
TI OpenCL implementations:

+----------+---------------------------------------------------------------+------------------------------------------------------------------------------+
| SoC      | Host                                                          | Compute Device                                                               |
+----------+---------------------------------------------------------------+------------------------------------------------------------------------------+
| 66AK2H   | ARM MPCore cluster with 4 Cortex-A15 CPUs running SMP Linux   | 8 TI C66x DSP cores exposed as one accelerator device with 8 compute units   |
+----------+---------------------------------------------------------------+------------------------------------------------------------------------------+
| AM572x   | ARM MPCore cluster with 2 Cortex-A15 CPUs running SMP Linux   | 2 TI C66x DSP cores exposed as one accelerator device with 2 compute units   |
+----------+---------------------------------------------------------------+------------------------------------------------------------------------------+
