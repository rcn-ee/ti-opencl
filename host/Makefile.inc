UNAME_M:=$(shell uname -m)

OCL_MAJOR_VER=1
OCL_MINOR_VER=1
OCL_PATCH_VER=7
OCL_VER=$(OCL_MAJOR_VER).$(OCL_MINOR_VER).$(OCL_PATCH_VER)
OCL_DPKG_NAME=ti-opencl
OCL_VERSIONED_NAME=$(OCL_DPKG_NAME)_$(OCL_VER)

# ARM GCC

ifeq (,$(findstring arm, $(UNAME_M)))
    ARM_GCC_DIR?=/cgnas/gcc-linaro-arm-linux-gnueabihf-4.7-2013.03-20130313_linux
    export PATH:=$(ARM_GCC_DIR)/bin:$(PATH)
endif

# TI C6x CGT

ifeq (,$(findstring arm, $(UNAME_M)))
    TI_OCL_CGT_INSTALL?=/cgnas/ti-cgt-c6000-8.1.0A15292-linux
else
    TI_OCL_CGT_INSTALL?=/cgnas/ti-cgt-c6000-8.1.0A15292-armlinuxa8hf
endif

# LLVM

ARM_LLVM_DIR?=/cgnas/llvm-3.6.0-20151019-arm

ifeq (,$(findstring x86_64, $(UNAME_M)))
    X86_LLVM_DIR?=/cgnas/llvm-3.6.0-20151019-x86
else
    X86_LLVM_DIR?=/cgnas/llvm-3.6.0-20151019-x86_64
endif

# K2X MCSDK

DEFAULT_MCSDK?=/cgnas/mcsdk_3_01_04_07-20151002
ifeq (,$(findstring arm, $(UNAME_M)))
    MCSDK_LINUX_DEVKIT_ROOT?=$(DEFAULT_MCSDK)/mcsdk_linux_3_01_04_07/linux-devkit/arago-2015.05/sysroots/cortexa15t2hf-vfp-neon-linux-gnueabi
else
    MCSDK_LINUX_DEVKIT_ROOT?=/
endif

# AM57 Processor SDK

DEFAULT_PSDK_RTOS?=/cgnas/ti-processor-sdk-rtos-am57xx-evm-02.00.00.00-20151007-release
DEFAULT_PSDK_LINUX?=/cgnas/ti-processor-sdk-linux-am57xx-evm-02.00.00.00-20151007-release
PSDK_LINUX_DEVKIT_ROOT?=$(DEFAULT_PSDK_LINUX)/linux-devkit/sysroots/cortexa15hf-vfp-neon-linux-gnueabi

# DSPC SDKs

# note: also uses $(DEFAULT_MCSDK)
DEFAULT_DLSDK?=/cgnas/desktop-linux-sdk_01_00_03_00
C6678_PDK_DIR?=/cgnas/pdk_C6678_2_1_3_7
C6678_PDK1_DIR?=/cgnas/pdk_C6678_1_1_2_5
