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

Changing DDR3 Partition for OpenCL
=====================================================
Starting from Processor SDK 2.0.1.x, DDR3 partitioned for OpenCL use is
statically reserved in the device tree file.  Should you wish to change the
size of DDR reserved for OpenCL for various reasons (e.g. you upgraded DDR on
your 66AK2x EVM to 8GB and wish to allocate more DDR for OpenCL), you can
modify the device tree.  The following are the steps to increase DDR reserved
for OpenCL from default 368MB to 4GB on K2HK, as an example (other platforms
are similar).

 #. Go to the Processor SDK directory that you downloaded for your EVM,
    for example,
    ::

      export PSDK_DIR=$HOME/ti-processor-sdk-linux-k2hk-evm-02.00.01.07
      export DTS_DIR=$PSDK_DIR/board-support/linux-4.1.13+gitAUTOINC+8dc66170d9-g8dc6617/arch/arm/boot/dts
      cd $DTS_DIR

 #. Update cmem dts file ($DTS_DIR/k2hk-evm-cmem.dtsi) to increase the size
    of cmem_block_0,
    ::

      --- k2hk-evm-cmem.dtsi.orig	2016-01-25 14:46:28.929687202 -0600
      +++ k2hk-evm-cmem.dtsi	2016-01-25 14:47:14.217688225 -0600
      @@ -7,7 +7,7 @@
                       };
       
                       cmem_block_mem_0: cmem_block_mem@829000000 {
      -                        reg = <0x00000008 0x29000000 0x00000000 0x17000000>;
      +                        reg = <0x00000008 0x29000000 0x00000001 0x00000000>;
                               no-map;
                               status = "okay";
                       };
      @@ -37,7 +37,7 @@
                       cmem_block_0: cmem_block@0 {
                               reg = <0>;
                               memory-region = <&cmem_block_mem_0>;
      -                        cmem-buf-pools = <1 0x00000000 0x17000000>;
      +                        cmem-buf-pools = <1 0x00000001 0x00000000>;
                       };
       
                       cmem_block_1: cmem_block@1 {

 #. Recompile device tree source (.dts) into binary (.dtb).  In the directory
    where your EVM gets the dtb file (e.g. /var/lib/tftpboot/k2hk-evm.dtb on
    host machine if booting from net with tftp, /boot/k2hk-evm.dtb on EVM's
    file system if boot from sdcard), replace the old dtb file with the newly
    compiled one,
    ::

      cd $PSDK_DIR
      make linux-dtbs
      cd $EVM_DTB_DIR
      cp k2hk-evm.dtb k2hk-evm.dtb.orig
      cp $DTS_DIR/k2hk-evm.dtb .

 #. Reboot your evm, check /proc/iomem or run OpenCL platforms example to
    verify the changes,
    ::

      # cat /proc/iomem | grep CMEM
      0c100000-0c57ffff : CMEM
      822000000-828ffffff : CMEM
      829000000-928ffffff : CMEM

      # ./platforms 
      PLATFORM: TI KeyStone II
        Version: OpenCL 1.1 TI product version 01.01.08.00 (Jan 22 2016 15:18:29)
        Vendor : Texas Instruments, Inc.
        Profile: FULL_PROFILE
          DEVICE: TI Multicore C66 DSP
            Type       : ACCELERATOR
            CompUnits  : 8
            Frequency  : 1.2 GHz
            Glb Mem    :  376832 KB
            GlbExt1 Mem: 3817472 KB
            GlbExt2 Mem:       0 KB
            Msmc Mem   :    4608 KB
            Loc Mem    :     768 KB
            Max Alloc  :  376832 KB

.. Note::
    This method of changing DDR partitioning for OpenCL does NOT apply to
    the m800 K2H system.
