UNAME_M:=$(shell uname -m)

# project version
# The linux shared library versioning convention uses x.y.z format,
# with no leading zeros. For debian packaging versioning we use xx.yy.zz.ww
# formatting with leading zero's.  Opencl/clocl platform and version queries
# will report in this format.  The 'ww' represents the PATCH_VERSION of a
# particular release.  This number will not be reflected in the library
# .so name, only in the packaging.
# NOTE!!!!!! The packaging version numbers need to match what is specified in 
# debian/changelog!!!
OCL_MAJOR_VER=1
OCL_MINOR_VER=1
OCL_RELEASE_VER=14
OCL_PATCH_VER=04
OCL_VER=$(OCL_MAJOR_VER).$(OCL_MINOR_VER).$(OCL_RELEASE_VER)
OCL_DPKG_NAME=ti-opencl
OCL_VERSIONED_NAME=$(OCL_DPKG_NAME)_$(OCL_VER)

OCL_FULL_VER=$(OCL_MAJOR_VER).$(OCL_MINOR_VER).$(OCL_RELEASE_VER).$(OCL_PATCH_VER)

SDOMC_SHARED?=/cgnas

# XDC (not included in CoreSDK)
XDC_DIR?=$(SDOMC_SHARED)/xdctools_3_32_01_22_core

# TI C6x CGT

ifeq (,$(findstring arm, $(UNAME_M)))
    TI_OCL_CGT_INSTALL?=$(SDOMC_SHARED)/ti-cgt-c6000-8.2.0-release-linux
else
    TI_OCL_CGT_INSTALL?=$(SDOMC_SHARED)/ti-cgt-c6000-8.2.0-release-armlinuxa8hf
endif

# LLVM

ifeq ($(BUILD_OS), SYS_BIOS)
    ARM_LLVM_DIR=$(SDOMC_SHARED)/llvm-3.6.0-20170608-sysbios
else
    ARM_LLVM_DIR=$(SDOMC_SHARED)/llvm-3.6.0-20170608-arm
endif

ifeq (,$(findstring x86_64, $(UNAME_M)))
    X86_LLVM_DIR?=$(SDOMC_SHARED)/llvm-3.6.0-20170608-x86
else
    X86_LLVM_DIR?=$(SDOMC_SHARED)/llvm-3.6.0-20170608-x86_64
endif

CORESDK_VERSION ?= ti2017.04-rc2

ifeq ($(BUILD_K2H),1)
	CORE_SDK?=$(SDOMC_SHARED)/ti-processor-sdk-linux-k2hk-evm-$(CORESDK_VERSION)
else ifeq ($(BUILD_K2L),1)
	CORE_SDK?=$(SDOMC_SHARED)/ti-processor-sdk-linux-k2l-evm-$(CORESDK_VERSION)
else ifeq ($(BUILD_K2E),1)
	CORE_SDK?=$(SDOMC_SHARED)/ti-processor-sdk-linux-k2e-evm-$(CORESDK_VERSION)
else ifeq ($(BUILD_K2G),1)
	CORE_SDK?=$(SDOMC_SHARED)/ti-processor-sdk-linux-k2g-evm-$(CORESDK_VERSION)
else ifeq ($(BUILD_AM57),1)
	CORE_SDK?=$(SDOMC_SHARED)/ti-processor-sdk-linux-am57xx-evm-$(CORESDK_VERSION)
else
endif

ifeq (,$(findstring arm, $(UNAME_M)))
    LINUX_DEVKIT_ROOT?=$(CORE_SDK)/linux-devkit/sysroots/armv7ahf-neon-linux-gnueabi
else
    LINUX_DEVKIT_ROOT?=/
endif

# ARM GCC
ifeq (,$(findstring arm, $(UNAME_M)))
    ARM_GCC_DIR?=$(SDOMC_SHARED)/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf
    GCC_ARM_NONE_TOOLCHAIN?=$(SDOMC_SHARED)/gcc-arm-none-eabi-4_9-2015q3
endif

# RTOS packages (BIOS, IPC, FC, EDMA3_LLD etc.)
RTOS_INSTALL_DIR?=$(LINUX_DEVKIT_ROOT)/usr/share/ti

# OMP 
OMP_DIR?=$(RTOS_INSTALL_DIR)/ti-omp-tree
