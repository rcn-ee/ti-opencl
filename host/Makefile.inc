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
OCL_RELEASE_VER=19
OCL_PATCH_VER=00
OCL_VER=$(OCL_MAJOR_VER).$(OCL_MINOR_VER).$(OCL_RELEASE_VER)
OCL_DPKG_NAME=ti-opencl
OCL_VERSIONED_NAME=$(OCL_DPKG_NAME)_$(OCL_VER)

OCL_FULL_VER=$(OCL_MAJOR_VER).$(OCL_MINOR_VER).$(OCL_RELEASE_VER).$(OCL_PATCH_VER)

ifeq ($(shell git rev-parse --short HEAD 2>&1 1>/dev/null; echo $$?),0)
GIT_SHA?=$(shell git rev-parse --short HEAD)
endif


SDOMC_SHARED?=/cgnas

# XDC (not included in CoreSDK)
XDC_DIR?=$(SDOMC_SHARED)/xdctools_3_55_02_22_core

# TI C6x CGT
TI_OCL_CGT_INSTALL?=$(SDOMC_SHARED)/ti-cgt-c6000-8.3.2/linux

# TI M4 CGT
TI_OCL_M4_CGT_INSTALL ?= $(SDOMC_SHARED)/ti-cgt-arm_18.1.5.LTS

# Set to 1 to switch from CMEM to ION
USE_ION ?= 0

# LLVM

ifeq ($(BUILD_OS), SYS_BIOS)
    ARM_LLVM_DIR?=$(SDOMC_SHARED)/llvm-3.6.0-20180423-sysbios-gcc6.3.1
else
    ARM_LLVM_DIR?=$(SDOMC_SHARED)/llvm-3.6.0-20190820-arm-g++8.3.0
endif

NATIVE_GXX_VER := $(shell g++ -dumpversion | cut -f1 -d.)
NATIVE_GXX_PRE7 := $(shell test $(NATIVE_GXX_VER) -lt 7 && echo true || echo false)

ifeq (,$(findstring x86_64, $(UNAME_M)))
    X86_LLVM_DIR?=$(SDOMC_SHARED)/llvm-3.6.0-20170608-x86
else
    ifeq ($(NATIVE_GXX_PRE7), true)
        X86_LLVM_DIR?=$(SDOMC_SHARED)/llvm-3.6.0-20170608-x86_64
        NATIVE_GXX?=g++-5
    else
        X86_LLVM_DIR?=$(SDOMC_SHARED)/llvm-3.6.0-20190108-x86_64-g++7.3.0
        NATIVE_GXX?=g++
    endif
endif

ifeq ($(USE_ION), 1)
CORESDK_VERSION ?= 05.02.00.10-ion
else
CORESDK_VERSION ?= 06.01.00.04-20190826
endif

TIDL_VERSION    ?= 05_03_00_02-295-2019-02-27_10-52-08

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
	DEFAULT_PSDK_RTOS?=$(SDOMC_SHARED)/ti-processor-sdk-rtos-am57xx-evm-$(CORESDK_VERSION)
else
endif

ifeq (,$(findstring arm, $(UNAME_M)))
    LINUX_DEVKIT_ROOT?=$(CORE_SDK)/linux-devkit/sysroots/armv7at2hf-neon-linux-gnueabi
else
    LINUX_DEVKIT_ROOT?=/
endif

# ARM GCC
ifeq (,$(findstring arm, $(UNAME_M)))
    ARM_GCC_DIR?=$(SDOMC_SHARED)/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf
    GCC_ARM_NONE_TOOLCHAIN?=$(SDOMC_SHARED)/gcc-arm-none-eabi-7-2018-q2-update
endif

# RTOS packages (BIOS, IPC, FC, EDMA3_LLD etc.)
RTOS_INSTALL_DIR?=$(LINUX_DEVKIT_ROOT)/usr/share/ti

# OMP
OMP_DIR?=$(RTOS_INSTALL_DIR)/ti-omp-tree

# Processor SDK Vision (remove once components are integrated into Processor SDK)
PSDK_VISION?=PROCESSOR_SDK_VISION_03_03_00_00
