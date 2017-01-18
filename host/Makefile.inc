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
OCL_RELEASE_VER=12
OCL_PATCH_VER=00
OCL_VER=$(OCL_MAJOR_VER).$(OCL_MINOR_VER).$(OCL_RELEASE_VER)
OCL_DPKG_NAME=ti-opencl
OCL_VERSIONED_NAME=$(OCL_DPKG_NAME)_$(OCL_VER)

OCL_FULL_VER=$(OCL_MAJOR_VER).$(OCL_MINOR_VER).$(OCL_RELEASE_VER).$(OCL_PATCH_VER)

SDOMC_SHARED?=/cgnas

# ARM GCC

ifeq (,$(findstring arm, $(UNAME_M)))
    ARM_GCC_DIR?=$(SDOMC_SHARED)/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf
    GCC_ARM_NONE_TOOLCHAIN?=$(SDOMC_SHARED)/gcc-arm-none-eabi-4_9-2015q3
endif

# TI C6x CGT

ifeq (,$(findstring arm, $(UNAME_M)))
    TI_OCL_CGT_INSTALL?=$(SDOMC_SHARED)/ti-cgt-c6000-8.1.0-release-linux
else
    TI_OCL_CGT_INSTALL?=$(SDOMC_SHARED)/ti-cgt-c6000-8.1.0-release-armlinuxa8hf
endif

# LLVM

ifeq ($(BUILD_OS), SYS_BIOS)
    ARM_LLVM_DIR=$(SDOMC_SHARED)/llvm-3.6.0-20161107-sysbios
else
    ARM_LLVM_DIR=$(SDOMC_SHARED)/llvm-3.6.0-20161107-arm
endif

ifeq (,$(findstring x86_64, $(UNAME_M)))
    X86_LLVM_DIR?=$(SDOMC_SHARED)/llvm-3.6.0-20161107-x86
else
    X86_LLVM_DIR?=$(SDOMC_SHARED)/llvm-3.6.0-20161107-x86_64
endif

# OMP 

OMP_DIR_K2X?=$(SDOMC_SHARED)/openmp_dsp_k2x_02_06_00_00-20170118
OMP_DIR_K2G?=$(SDOMC_SHARED)/openmp_dsp_k2g_02_06_00_00-20170118
OMP_DIR_AM57?=$(SDOMC_SHARED)/openmp_dsp_am57xx_02_06_00_00-20170118

# SOC PSDK

PSDK_VERSION ?= 03.01.00.06-release

ifeq ($(BUILD_K2H),1)
    DEFAULT_PSDK_RTOS?=$(SDOMC_SHARED)/ti-processor-sdk-rtos-k2hk-evm-$(PSDK_VERSION)
    DEFAULT_PSDK_LINUX?=$(SDOMC_SHARED)/ti-processor-sdk-linux-k2hk-evm-$(PSDK_VERSION)
    OMP_DIR?=$(OMP_DIR_K2X)
else ifeq ($(BUILD_K2L),1)
    DEFAULT_PSDK_RTOS?=$(SDOMC_SHARED)/ti-processor-sdk-rtos-k2l-evm-$(PSDK_VERSION)
    DEFAULT_PSDK_LINUX?=$(SDOMC_SHARED)/ti-processor-sdk-linux-k2l-evm-$(PSDK_VERSION)
    OMP_DIR?=$(OMP_DIR_K2X)
else ifeq ($(BUILD_K2E),1)
    DEFAULT_PSDK_RTOS?=$(SDOMC_SHARED)/ti-processor-sdk-rtos-k2e-evm-$(PSDK_VERSION)
    DEFAULT_PSDK_LINUX?=$(SDOMC_SHARED)/ti-processor-sdk-linux-k2e-evm-$(PSDK_VERSION)
    OMP_DIR?=$(OMP_DIR_K2X)
else ifeq ($(BUILD_K2G),1)
    DEFAULT_PSDK_RTOS?=$(SDOMC_SHARED)/ti-processor-sdk-rtos-k2g-evm-$(PSDK_VERSION)
    DEFAULT_PSDK_LINUX?=$(SDOMC_SHARED)/ti-processor-sdk-linux-k2g-evm-$(PSDK_VERSION)
    OMP_DIR?=$(OMP_DIR_K2G)
else ifeq ($(BUILD_AM57),1)
    DEFAULT_PSDK_RTOS?=$(SDOMC_SHARED)/ti-processor-sdk-rtos-am57xx-evm-$(PSDK_VERSION)
    DEFAULT_PSDK_LINUX?=$(SDOMC_SHARED)/ti-processor-sdk-linux-am57xx-evm-$(PSDK_VERSION)
    OMP_DIR?=$(OMP_DIR_AM57)
else
endif

ifeq (,$(findstring arm, $(UNAME_M)))
    PSDK_LINUX_DEVKIT_ROOT?=$(DEFAULT_PSDK_LINUX)/linux-devkit/sysroots/armv7ahf-neon-linux-gnueabi
else
    PSDK_LINUX_DEVKIT_ROOT?=/
endif

# DSPC SDKs

DEFAULT_DLSDK?=$(SDOMC_SHARED)/desktop-linux-sdk_01_00_03_00
C6678_PDK_DIR?=$(SDOMC_SHARED)/pdk_C6678_2_1_3_7
C6678_PDK1_DIR?=$(SDOMC_SHARED)/pdk_C6678_1_1_2_5
