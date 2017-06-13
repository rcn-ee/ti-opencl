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
buffer communication between the A15 and C66x cores, because the C66x cores
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

AM57
=====================================================
The 2GB of attached DDR3 memory is accessible to the AM57 device through a
32-bit bus. The 2GB of DDR3 is populated in the 32-bit address space at
locations 8000:0000 through FFFF:FFFF. The default partition is ~1.84GB
for Linux system memory and 160MB of CMEM. ::

    80000000-9fffffff : System RAM
      80008000-808330b3 : Kernel code
      80888000-8091e34b : Kernel data
    a0000000-a9ffffff : CMEM
    aa000000-ffdfffff : System RAM
    fff00000-ffffefff : System RAM

The holes in System RAM ranges (e.g. FFE0:0000 to FFEF:FFFF, 1MB and FFFF:F000
to FFFF:FFFF, 4KB) are reserved memory ranges.

.. _CHANGE_DDR3_PARTITION_FOR_OPENCL:

Changing DDR3 Partition for OpenCL
=====================================================
Starting from Processor SDK 2.0.1.x, DDR3 partitioned for OpenCL use is
statically reserved in the device tree file.  Should you wish to change the
size of DDR reserved for OpenCL for various reasons (e.g. you upgraded DDR on
your 66AK2x EVM to 8GB and wish to allocate more DDR for OpenCL), you can
modify the device tree with the "dtc" tool from the device-tree-compiler
package.

The following are the steps to increase DDR reserved for OpenCL from default
384MB in Processor SDK 3.3 to either 768MB or 1.25GB on K2HK, as examples
(other platforms are similar).  You can make changes with the sizes that suit
your use case.

 #. Make a copy of the original k2hk-evm.dtb (if you care)
    ::

      cp /var/lib/tftpboot/k2hk-evm.dtb /var/lib/tftpboot.k2hk-evm.dtb.orig

 #. Convert device tree blob format to source format
    ::

      dtc -I dtb -O dts /var/lib/tftpboot/k2hk-evm.dtb -o tmp.dts

 #. Modify the sizes in both cmem_block@0 and corresponding cmem_block_mem
    nodes
    ::

      --- k2hk-evm.dts.orig     2017-03-15 15:05:58.779020849 -0500
      +++ k2hk-evm.dts          2017-03-15 15:06:33.083021624 -0500
      @@ -2814,7 +2814,7 @@
                };
       
                cmem_block_mem@830000000 {
      -                 reg = <0x8 0x30000000 0x0 0x18000000>;
      +                 reg = <0x8 0x30000000 0x0 0x30000000>;
                        no-map;
                        status = "okay";
                        linux,phandle = <0x5f>;
      @@ -2872,7 +2872,7 @@
                cmem_block@0 {
                        reg = <0x0>;
                        memory-region = <0x5f>;
      -                 cmem-buf-pools = <0x1 0x0 0x18000000>;
      +                 cmem-buf-pools = <0x1 0x0 0x30000000>;
                };
       
                cmem_block@1 {

    Or, if 1.25GB of CMEM is desired, change the sizes to
    ::

      -                 reg = <0x8 0x30000000 0x0 0x18000000>;
      +                 reg = <0x8 0x30000000 0x0 0x50000000>;
      -                 cmem-buf-pools = <0x1 0x0 0x18000000>;
      +                 cmem-buf-pools = <0x1 0x0 0x50000000>;

 #. Convert device tree source back to blob format
    ::

      dtc -I dts -O dtb tmp.dts -o /var/lib/tftpboot/k2hk-evm.dtb

 #. Reboot your evm, check /proc/iomem or run OpenCL "platforms" example to
    verify the changes (use the 1.25GB cmem block as an example)
    ::

      # cat /proc/iomem | grep CMEM
      830000000-87fffffff : CMEM

      # /usr/share/ti/examples/opencl/platforms/platforms
      PLATFORM: TI KeyStone II
        Version: OpenCL 1.1 TI product version 01.01.12.0 (Mar  5 2017 00:53:44)
        Vendor : Texas Instruments, Inc.
        Profile: FULL_PROFILE
          DEVICE: TI Multicore C66 DSP
            Type       : ACCELERATOR
            CompUnits  : 8
            Frequency  : 1.2 GHz
            Glb Mem    : 1310720 KB
            GlbExt1 Mem:       0 KB
            GlbExt2 Mem:       0 KB
            Msmc Mem   :    4608 KB
            Loc Mem    :     864 KB
            Max Alloc  : 1048576 KB

.. Note::
    This method of changing DDR partitioning for OpenCL does NOT apply to
    the m800 K2H system.

.. Note::
    Starting from Processor SDK 2.0.1.x, uboot variable, "mem_reserve", is
    no longer used to reserve memory for CMEM.  If you still have leftover
    "mem_reserve" in your uboot environment, please unset it by
    "setenv mem_reserve" followed by "saveenv".

