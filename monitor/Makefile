##
##  Copyright (c) 2016, Texas Instruments Incorporated
##
##  Redistribution and use in source and binary forms, with or without
##  modification, are permitted provided that the following conditions
##  are met:
##
##  *  Redistributions of source code must retain the above copyright
##     notice, this list of conditions and the following disclaimer.
##
##  *  Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in the
##     documentation and/or other materials provided with the distribution.
##
##  *  Neither the name of Texas Instruments Incorporated nor the names of
##     its contributors may be used to endorse or promote products derived
##     from this software without specific prior written permission.
##
##  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
##  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
##  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
##  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
##  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
##  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
##  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
##  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
##  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
##  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
##  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##

include ../host/Makefile.inc

# If not specified, assume the opencl source directory is at same location as
# monitor
OPENCL_SRC_DIR ?= ../host

PDK_DIR        ?= $(RTOS_INSTALL_DIR)/ti-pdk-tree
IPC_DIR        ?= $(RTOS_INSTALL_DIR)/ti-ipc-tree
BIOS_DIR       ?= $(RTOS_INSTALL_DIR)/ti-sysbios-tree
FC_DIR         ?= $(RTOS_INSTALL_DIR)/ti-framework-components-tree
EDMA3LLD_DIR   ?= $(RTOS_INSTALL_DIR)/ti-edma3lld-tree
XDAIS_DIR      ?= $(RTOS_INSTALL_DIR)/ti-xdais-tree
ULM_DIR        ?= $(RTOS_INSTALL_DIR)/ulm
GDB_SERVER_DIR ?= $(RTOS_INSTALL_DIR)/gdbc6x
AET_DIR        ?= $(RTOS_INSTALL_DIR)/ctoolslib/aet

export XDC_DIR
export BIOS_DIR

# And for the devkit, when doing an x86 build ...
UNAME_M :=$(shell uname -m)

ifneq (,$(findstring 86, $(UNAME_M)))
    ifeq ($(LINUX_DEVKIT_ROOT),)
      $(error Monitor requires LINUX_DEVKIT_ROOT to point to AM57 linux-devkit root)
    endif
endif

ifeq ($(TI_OCL_CGT_INSTALL),)
  $(error Monitor requires TI_OCL_CGT_INSTALL to point to C6000 Codegen tools)
endif

#----------------------------------------------------------------------------
# LINKER DEFINES: dsp monitor build timestamp
#----------------------------------------------------------------------------
LNK_DEFS := --define=OCL_BUILD_VER=$(shell echo "" | \
                awk '{ printf ("0x%02d%02d%02d%02d", $(OCL_MAJOR_VER), \
                $(OCL_MINOR_VER), $(OCL_RELEASE_VER), $(OCL_PATCH_VER)); }') \
            --define=OCL_BUILD_SHA=$(shell git rev-parse HEAD | \
                awk '{ printf ("0x%.8s", $$1); }') \
            --define=OCL_BUILD_DATE=$(shell date +0x%Y%m%d) \
            --define=OCL_BUILD_TIME=$(shell date +0x%H%M%S)
.PHONY: $(LNK_DEFS)

ifeq ($(BUILD_TARGET), ARM_AM57)
    ifeq ($(BUILD_OS), SYS_BIOS)
    	include Makefile.am57x_rtos
    else
    	include Makefile.am57x
    endif
else ifeq ($(BUILD_TARGET), ARM_K2G)
    include Makefile.evmk2g
else ifeq ($(BUILD_TARGET), ARM_K2H)
    include Makefile.k2hle
else ifeq ($(BUILD_TARGET), ARM_K2L)
    include Makefile.k2hle
else ifeq ($(BUILD_TARGET), ARM_K2E)
    include Makefile.k2hle
else
    $(error BUILD_TARGET $(BUILD_TARGET) not supported)
endif
