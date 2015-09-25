******************************************************
How DDR3 is Partitioned for Linux System and OpenCL
******************************************************

.. _CMEM:

The DDR3 is partitioned into three components:

1. Linux system memory,
2. CMEM contiguous memory, and
3. Reserved memory.

The Linux system memory is the underlying memory store for the Linux virtual
memory system and would contain standard items like:

- A15 stacks,
- A15 heaps,
- A15 application code,
- A15 application variables, etc.

The CMEM contiguous memory is controlled by a Linux kernel module that
guarantees contiguous virtual addresses within a range are mapped to
contiguous physical addresses within the range. This is required for
buffer communication between the A15 and C66 cores, because the C66 cores
do not access memory through a shared MMU with the A15 CPUs and thus require
that buffers be allocated in contiguous physical memory. The CMEM memory
areas will be managed by OpenCL for allocation to OpenCL buffers and OpenCL C
programs.

The reserved memory is a very small portion of the DDR3 memory that is used in
the OpenCL implementation and is exposed to neither CMEM nor Linux.

66AK2x
=====================================================

The 8GB of attached DDR3 memory is accessible to the K2x device through a
64-bit bus. The 8GB of DDR3 is populated in the K2x 36-bit address space at
locations 8:0000:0000 through 9:FFFF:FFFF.

The first 2GB of DDR3 are fixed in usage to the following:

========================== ===================
Memory Range               Usage
========================== ===================
8:0000:0000 - 8:1FFF:FFFF  512M Linux System
8:2000:0000 - 8:22FF:FFFF  48M Reserved
8:2300:0000 - 8:7FFF:FFFF  1488M CMEM
========================== ===================

The remaining 6GB of DDR3 can be split between Linux and CMEM using boot time
variables. The default partition of the remaining 6GB would be:

========================== ===================
Memory Range               Usage
========================== ===================
8:8000:0000 - 8:BFFF:FFFF  1GB Linux System
8:C000:0000 - 9:FFFF:FFFF  5GB CMEM
========================== ===================

You can verify the partition in your system by viewing the ``/proc/iomem``
system file. The bottom of this file will contain the external DDR memory map,
for example::

    800000000-81fffffff : System RAM
      800008000-8006b5277 : Kernel code
      8006fa000-8007ace53 : Kernel data
    823000000-87fffffff : CMEM
    8 80000000-8bfffffff : System RAM
    8c0000000-9ffffffff : CMEM

The default partition of 1.5GB Linux system memory and 6.48GB CMEM provides a
minimum suggested Linux system memory size and a larger area for OpenCL buffer
and program space.

.. Note::
    The m800 K2H system ships with 8GB of DDR3. The K2H EVM ships with
    2GB of DDR3 and can be upgraded to 8GB by replacing the DIMM

.. only:: am57

    AM57
    =====================================================
    The 2GB of attached DDR3 memory is accessible to the AM57 device through a
    32-bit bus. The 2GB of DDR3 is populated in the 32-bit address space at
    locations 8000:0000 through FFFF:FFFF. The default partition is 1.875GB
    for Linux system memory and 128MB of CMEM. ::

        80000000-9fffffff : System RAM
          80008000-808330b3 : Kernel code
          80888000-8091e34b : Kernel data
        a0000000-a7ffffff : CMEM
        a8000000-ffdfffff : System RAM
        fff00000-ffffefff : System RAM

    The holes in System RAM ranges (e.g. FFE0:0000 to FFEF:FFFF, 1MB and FFFF:F000
    to FFFF:FFFF, 4KB) are reserved memory ranges.
