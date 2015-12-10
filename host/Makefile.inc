UNAME_M:=$(shell uname -m)

OCL_MAJOR_VER=1
OCL_MINOR_VER=1
OCL_PATCH_VER=7
OCL_VER=$(OCL_MAJOR_VER).$(OCL_MINOR_VER).$(OCL_PATCH_VER)
OCL_DPKG_NAME=ti-opencl
OCL_VERSIONED_NAME=$(OCL_DPKG_NAME)_$(OCL_VER)

# ARM GCC

ifeq (,$(findstring arm, $(UNAME_M)))
    ARM_GCC47_DIR?=/cgnas/gcc-linaro-arm-linux-gnueabihf-4.7-2013.03-20130313_linux
    ARM_GCC49_DIR?=/cgnas/gcc-linaro-4.9-2015.05-x86_64_arm-linux-gnueabihf
endif

# TI C6x CGT

ifeq (,$(findstring arm, $(UNAME_M)))
    TI_OCL_CGT_INSTALL?=/cgnas/ti-cgt-c6000-8.1.0B1-linux
else
    TI_OCL_CGT_INSTALL?=/cgnas/ti-cgt-c6000-8.1.0B1-armlinuxa8hf
endif

# LLVM

ARM_LLVM_DIR?=/cgnas/llvm-3.6.0-20151113-arm

ifeq (,$(findstring x86_64, $(UNAME_M)))
    X86_LLVM_DIR?=/cgnas/llvm-3.6.0-20151113-x86
else
    X86_LLVM_DIR?=/cgnas/llvm-3.6.0-20151113-x86_64
endif

# OMP

KEYSTONE_OMP_DIR?=/cgnas/openmp_dsp_2_02_00_01_ks_alpha
AM57_OMP_DIR?=/cgnas/openmp_dsp_2_02_00_01_am57xx_alpha

# K2X PSDK
ifeq ($(BUILD_K2H),1)
    DEFAULT_PSDK_RTOS_K2X?=/cgnas/ti-processor-sdk-rtos-k2hk-evm-02.00.01.04-20151205
    DEFAULT_PSDK_LINUX_K2X?=/cgnas/ti-processor-sdk-linux-k2hk-evm-02.00.01.04-20151203
else ifeq ($(BUILD_K2L),1)
    DEFAULT_PSDK_RTOS_K2X?=/cgnas/ti-processor-sdk-rtos-k2hk-evm-02.00.01.04-20151205
    DEFAULT_PSDK_LINUX_K2X?=/cgnas/ti-processor-sdk-linux-k2hk-evm-02.00.01.04-20151203
else ifeq ($(BUILD_K2E),1)
    DEFAULT_PSDK_RTOS_K2X?=/cgnas/ti-processor-sdk-rtos-k2hk-evm-02.00.01.04-20151205
    DEFAULT_PSDK_LINUX_K2X?=/cgnas/ti-processor-sdk-linux-k2hk-evm-02.00.01.04-20151203
else
endif

ifeq (,$(findstring arm, $(UNAME_M)))
    PSDK_LINUX_DEVKIT_ROOT_K2X?=$(DEFAULT_PSDK_LINUX_K2X)/linux-devkit/sysroots/cortexa15hf-vfp-neon-linux-gnueabi
else
    PSDK_LINUX_DEVKIT_ROOT_K2X?=/
endif

# K2X MCSDK

DEFAULT_MCSDK?=/cgnas/mcsdk_3_01_04_07-20151002
ifeq (,$(findstring arm, $(UNAME_M)))
    MCSDK_LINUX_DEVKIT_ROOT?=$(DEFAULT_MCSDK)/mcsdk_linux_3_01_04_07/linux-devkit/arago-2015.05/sysroots/cortexa15t2hf-vfp-neon-linux-gnueabi
else
    MCSDK_LINUX_DEVKIT_ROOT?=/
endif

# AM57 Processor SDK (2.0.1.x)

DEFAULT_PSDK_RTOS?=/cgnas/ti-processor-sdk-rtos-am57xx-evm-02.00.01.04-20151206
DEFAULT_PSDK_LINUX?=/cgnas/ti-processor-sdk-linux-am57xx-evm-02.00.01.04-20151206
PSDK_LINUX_DEVKIT_ROOT?=$(DEFAULT_PSDK_LINUX)/linux-devkit/sysroots/cortexa15hf-vfp-neon-linux-gnueabi

# DSPC SDKs

# note: also uses $(DEFAULT_MCSDK)
DEFAULT_DLSDK?=/cgnas/desktop-linux-sdk_01_00_03_00
C6678_PDK_DIR?=/cgnas/pdk_C6678_2_1_3_7
C6678_PDK1_DIR?=/cgnas/pdk_C6678_1_1_2_5
