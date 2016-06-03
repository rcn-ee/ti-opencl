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
OCL_RELEASE_VER=9
OCL_PATCH_VER=00
OCL_VER=$(OCL_MAJOR_VER).$(OCL_MINOR_VER).$(OCL_RELEASE_VER)
OCL_DPKG_NAME=ti-opencl
OCL_VERSIONED_NAME=$(OCL_DPKG_NAME)_$(OCL_VER)

OCL_FULL_VER=$(OCL_MAJOR_VER).$(OCL_MINOR_VER).$(OCL_RELEASE_VER).$(OCL_PATCH_VER)

# ARM GCC

ifeq (,$(findstring arm, $(UNAME_M)))
    ARM_GCC47_DIR?=/cgnas/gcc-linaro-arm-linux-gnueabihf-4.7-2013.03-20130313_linux
    GCC_ARM_NONE_TOOLCHAIN?=/cgnas/gcc-arm-none-eabi-4_8-2014q3
    ARM_GCC49_DIR?=/cgnas/gcc-linaro-4.9-2015.05-x86_64_arm-linux-gnueabihf
endif

# TI C6x CGT

ifeq (,$(findstring arm, $(UNAME_M)))
    TI_OCL_CGT_INSTALL?=/cgnas/ti-cgt-c6000-8.1.0-release-linux
else
    TI_OCL_CGT_INSTALL?=/cgnas/ti-cgt-c6000-8.1.0-release-armlinuxa8hf
endif

# LLVM

ARM_LLVM_DIR?=/cgnas/llvm-3.6.0-20151119-arm
ifeq ($(BUILD_OS), SYS_BIOS)
    ARM_LLVM_DIR=/cgnas/llvm-3.6.0-arm-rtos
endif

ifeq (,$(findstring x86_64, $(UNAME_M)))
    X86_LLVM_DIR?=/cgnas/llvm-3.6.0-20151119-x86
else
    X86_LLVM_DIR?=/cgnas/llvm-3.6.0-20151119-x86_64
endif

# OMP 

OMP_DIR_K2X?=/cgnas/openmp_dsp_k2x_02_03_01_00-20160602
OMP_DIR_K2G?=/cgnas/openmp_dsp_k2g_02_03_01_00-20160602
OMP_DIR_AM57?=/cgnas/openmp_dsp_am57xx_02_03_01_00-20160602

PSDK_VERSION ?= 02.00.02.10-release

# SOC PSDK
ifeq ($(BUILD_K2H),1)
    DEFAULT_PSDK_RTOS_K2X?=/cgnas/ti-processor-sdk-rtos-k2hk-evm-$(PSDK_VERSION)
    DEFAULT_PSDK_LINUX_K2X?=/cgnas/ti-processor-sdk-linux-k2hk-evm-$(PSDK_VERSION)
    OMP_DIR?=$(OMP_DIR_K2X)
else ifeq ($(BUILD_K2L),1)
    DEFAULT_PSDK_RTOS_K2X?=/cgnas/ti-processor-sdk-rtos-k2l-evm-$(PSDK_VERSION)
    DEFAULT_PSDK_LINUX_K2X?=/cgnas/ti-processor-sdk-linux-k2l-evm-$(PSDK_VERSION)
    OMP_DIR?=$(OMP_DIR_K2X)
else ifeq ($(BUILD_K2E),1)
    DEFAULT_PSDK_RTOS_K2X?=/cgnas/ti-processor-sdk-rtos-k2e-evm-$(PSDK_VERSION)
    DEFAULT_PSDK_LINUX_K2X?=/cgnas/ti-processor-sdk-linux-k2e-evm-$(PSDK_VERSION)
    OMP_DIR?=$(OMP_DIR_K2X)
else ifeq ($(BUILD_K2G),1)
    DEFAULT_PSDK_RTOS_K2X?=/cgnas/ti-processor-sdk-rtos-k2g-evm-$(PSDK_VERSION)
    DEFAULT_PSDK_LINUX_K2X?=/cgnas/ti-processor-sdk-linux-k2g-evm-$(PSDK_VERSION)
    OMP_DIR?=$(OMP_DIR_K2G)
else ifeq ($(BUILD_AM57),1)
    DEFAULT_PSDK_RTOS?=/cgnas/ti-processor-sdk-rtos-am57xx-evm-$(PSDK_VERSION)
    DEFAULT_PSDK_LINUX?=/cgnas/ti-processor-sdk-linux-am57xx-evm-$(PSDK_VERSION)
    OMP_DIR?=$(OMP_DIR_AM57)
else
endif

PSDK_LINUX_DEVKIT_ROOT?=$(DEFAULT_PSDK_LINUX)/linux-devkit/sysroots/cortexa15hf-vfp-neon-linux-gnueabi

ifeq (,$(findstring arm, $(UNAME_M)))
    PSDK_LINUX_DEVKIT_ROOT_K2X?=$(DEFAULT_PSDK_LINUX_K2X)/linux-devkit/sysroots/cortexa15hf-vfp-neon-linux-gnueabi
else
    PSDK_LINUX_DEVKIT_ROOT_K2X?=/
endif

# K2X MCSDK - NOT USED? EJS

DEFAULT_MCSDK?=/cgnas/mcsdk_3_01_04_07-20151002
ifeq (,$(findstring arm, $(UNAME_M)))
    MCSDK_LINUX_DEVKIT_ROOT?=$(DEFAULT_MCSDK)/mcsdk_linux_3_01_04_07/linux-devkit/arago-2015.05/sysroots/cortexa15t2hf-vfp-neon-linux-gnueabi
else
    MCSDK_LINUX_DEVKIT_ROOT?=/
endif

# DSPC SDKs

# note: also uses $(DEFAULT_MCSDK)
DEFAULT_DLSDK?=/cgnas/desktop-linux-sdk_01_00_03_00
C6678_PDK_DIR?=/cgnas/pdk_C6678_2_1_3_7
C6678_PDK1_DIR?=/cgnas/pdk_C6678_1_1_2_5
