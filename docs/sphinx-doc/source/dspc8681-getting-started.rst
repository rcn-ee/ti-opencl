**************************************
OpenCL for DSPC8681 Getting Started
**************************************

Product Description
======================================

This product is an OpenCL 1.1 implementation.  The OpenCL specification defines
a platform model with a host and compute devices.  For this implementation the
host is a 32 or 64-bit x86 CPU running Ubuntu 12.04 and the compute devices are
4 Texas Instruments' TMS320C6678 DSPs resident on a PCIe card installed in the
Linux host.

Hardware and OS Requirements
======================================

- Ubuntu 12.04 installation running on a 32 or 64 bit x86 machine.
- An installed Advantech DSPC8681 quad DSP PCIe card configured for little endian operation.

Installation 
======================================

The following instructions use the Linux command :command:`wget` to retrieve
software packages.  If you are behind a corporate firewall, you  will need to
set the environment variable http_proxy to point to your proxy server.

- export http_proxy=http://<your_proxy_server>


One Time Setup for 64 bit Linux
-------------------------------------

#. cd $HOME 
#. sudo apt-get install wget libpciaccess-dev binutils-dev ia32-libs libsdl1.2-dev mesa-common-dev
#. wget http://software-dl.ti.com/codegen/esd/cgt_public_sw/C6000/8.0.3/ti_cgt_c6000_8.0.3_linux_installer_x86.bin
#. wget http://software-dl.ti.com/mctools/std/opencl/DSPC868X/1.1.3.0/exports/opencl-dspc8681_1.1.3.0_linux_x86_64.tar.gz
#. wget http://software-dl.ti.com/sdoemb/sdoemb_public_sw/desktop_linux_sdk/latest/exports/desktop-linux-sdk_01_00_03_00_64bit_setuplinux.bin
#. chmod +x \*.bin
#. ./desktop-linux-sdk_01_00_03_00_64bit_setuplinux.bin
    a. Choose your home directory as the installation location
#. tar xvf opencl-dspc8681_1.1.3.0_linux_x86_64.tar.gz
#. ./ti_cgt_c6000_8.0.3_linux_installer_x86.bin
#. cd  $HOME/desktop-linux-sdk_01_00_03_00
#. make
#. cd $HOME/desktop-linux-sdk_01_00_03_00/demos/scripts
#. sudo ./install_cmem_autoload.sh
#. sudo ./install_udev.sh
#. sudo reboot

One Time Setup for 32 bit Linux
-------------------------------------
#. cd $HOME 
#. sudo apt-get install wget libpciaccess-dev binutils-dev ia32-libs libsdl1.2-dev mesa-common-dev
#. wget http://software-dl.ti.com/codegen/esd/cgt_public_sw/C6000/8.0.3/ti_cgt_c6000_8.0.3_linux_installer_x86.bin
#. wget http://software-dl.ti.com/mctools/std/opencl/DSPC868X/1.1.3.0/exports/opencl-dspc8681_1.1.3.0_linux_i686.tar.gz
#. wget http://software-dl.ti.com/sdoemb/sdoemb_public_sw/desktop_linux_sdk/latest/exports/desktop-linux-sdk_01_00_03_00_32bit_setuplinux.bin
#. chmod +x \*.bin
#. ./desktop-linux-sdk_01_00_03_00_32bit_setuplinux.bin
    a. Choose your home directory as the installation location
#. tar xvf opencl-dspc8681_1.1.3.0_linux_i686.tar.gz
#. ./ti_cgt_c6000_8.0.3_linux_installer_x86.bin
#. cd  $HOME/desktop-linux-sdk_01_00_03_00
#. make
#. cd $HOME/desktop-linux-sdk_01_00_03_00/demos/scripts
#. sudo ./install_cmem_autoload.sh
#. sudo ./install_udev.sh
#. sudo reboot

Per Shell Setup 
-------------------------------------
The below commands are for sh or bash shells and are appropriate for a shell startup script.

#. export TI_OCL_INSTALL=$HOME/opencl-dspc8681_1.1.3.0
#. export TI_OCL_CGT_INSTALL=$HOME/ti-cgt-c6000_8.0.3
#. export C6X_C_DIR="$TI_OCL_CGT_INSTALL/include;$TI_OCL_CGT_INSTALL/lib"
#. export LD_LIBRARY_PATH="$TI_OCL_INSTALL/usr/lib:$LD_LIBRARY_PATH"
#. export PATH="$TI_OCL_INSTALL/usr/bin:$TI_OCL_CGT_INSTALL/bin:$PATH"

These Environment Variables are recommended, but not required:

#. export TI_OCL_CACHE_KERNELS=1
#. export TI_OCL_DSP_1_25GHZ=1

See :envvar:`TI_OCL_CACHE_KERNELS` and :envvar:`TI_OCL_DSP_1_25GHZ`
for descriptions of those environment variables.

Proper Setup Check 
-------------------------------------
#. cd $TI_OCL_INSTALL/examples/opencl/simple
#. make
#. ./simple

Documentation
=====================================

Please refer to :doc:`index` for product documentation. 

.. Note::

    The product documentation may not include information specific to this 
    alpha release of the DSPC8681 platform, but can be useful for generic 
    TI OpenCL information.

Limitations
=====================================

- The TI OpenCL implementation has passed conformance on other platforms with
  C66 DSPs as compute devices.  However, the implementation for the DSPC8681
  PCIe card is not yet a supported product and it has not passed Khronos OpenCL
  conformance.  Consider this installation an early alpha version of this
  product.  It is, however, complete enough to be useful under most circumstances.

- OpenCL Images and Samplers are optional features for non-GPU devices and are
  not supported for the DSP devices.

- This installation does not support the OpenCL ICD (Installable Client
  Driver).  To avoid potential conflict with existing ICD enabled OpenCL
  products, this version will not be installed in the typical /usr/lib,
  /usr/include locations.  It will instead be installed to a nonstandard
  location and an environment variable is used to locate the installation. To
  compile an opencl application you will need to point to the OpenCL headers
  and library.

        - Add ``-I$TI_OCL_INSTALL/usr/include`` as a compile options
        - Add ``-L$TI_OCL_INSTALL/usr/lib -lOpenCL -lbfd`` as link options

- This installation will not allow concurrent Linux processes that contain OpenCL 
  code. A file lock is used to sequentialize OpenCL access to the DSPs


Examples 
=====================================

See :doc:`examples/index`

.. Note::

    Product is based on a published Khronos Specification, and is expected to
    pass the Khronos Conformance Testing Process. Current conformance status can be
    found at www.khronos.org/conformance.

