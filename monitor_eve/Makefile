#
#  Copyright (c) 2013-2015 Texas Instruments Incorporated - http://www.ti.com
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  *  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#  *  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#  *  Neither the name of Texas Instruments Incorporated nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
#  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

#
#  ======== makefile ========
#

include ../host/Makefile.inc

DEVICE     = DEVICE_AM57
SOC        = SOC_TDA2XX
BUILD_TYPE = release

# If not specified, assume the opencl source directory is at same location as
# monitor
OPENCL_SRC_DIR ?= ../host

IPC_DIR        ?= $(RTOS_INSTALL_DIR)/ti-ipc-tree
BIOS_DIR       ?= $(RTOS_INSTALL_DIR)/ti-sysbios-tree
PDK_DIR        ?= /opt/ti/PROCESSOR_SDK_VISION_03_00_00_00/ti_components/drivers/pdk_01_07_00_16

TI_OCL_M4_CGT_INSTALL ?= /home/a0323918/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS

#VISIONSDK_UTILS ?= /opt/ti/PROCESSOR_SDK_VISION_03_00_00_00/vision_sdk/links_fw
VISIONSDK_UTILS ?= src/visionsdk_utils

SOURCES  = MainIpu1.c eve_firmware.c
SOURCES += $(VISIONSDK_UTILS)/src/rtos/utils_common/src/utils_eveloader.c
SOURCES += $(VISIONSDK_UTILS)/src/rtos/utils_common/src/eveloader/utils_eve_boot.c

VPATH = src:$(VISIONSDK_UTILS)/src/rtos/utils_common/src:$(VISIONSDK_UTILS)/src/rtos/utils_common/src/eveloader

OBJS  = $(addprefix bin/$(BUILD_TYPE)/obj/,$(patsubst %.c,%.oem4,$(notdir $(SOURCES))))
CONFIG = bin/$(BUILD_TYPE)/configuro
CONFIGS  = cfg/Ipu1.cfg cfg/IpuAmmu.cfg cfg/config.bld
CONFIGS += cfg/ipc.cfg.xs cfg/mem_segment_definition_linux.xs

PKGPATH := $(BIOS_DIR)/packages
PKGPATH := $(PKGPATH)+$(IPC_DIR)/packages
PKGPATH := $(PKGPATH)+$(XDC_DIR)/packages
PKGPATH := $(PKGPATH)+$(PDK_DIR)/packages

INCLUDES  = -i. -i $(OPENCL_SRC_DIR)/src/core/dsp
INCLUDES += -i $(VISIONSDK_UTILS)
INCLUDES += -D$(DEVICE) -D$(SOC) -DNON_VISIONSDK_BUILD
INCLUDES += -DIPU1_LOAD_EVES -DPROC_EVE1_INCLUDE
INCLUDES += -DPROC_EVE2_INCLUDE -DPROC_EVE3_INCLUDE -DPROC_EVE4_INCLUDE

libs = $(PDK_DIR)/packages/ti/boot/sbl_auto/lib/tda2xx-evm/m4/release/sbl_lib.aem4
libs += $(PDK_DIR)/packages/ti/drv/pm/lib/tda2xx/m4/release/pm_hal.aem4
libs += $(PDK_DIR)/packages/ti/csl/lib/tda2xx/m4/release/ti.csl.aem4


.PRECIOUS: %/compiler.opt %/linker.cmd

# always check if eve_firmware needs to be rebuild
.PHONY: eve_firmware

all: eve_firmware
	$(MAKE) server_ipu1.x

eve_firmware:
	$(MAKE) -f Makefile.eve BUILD_AM57=1 -j4

server_ipu1.x: bin/$(BUILD_TYPE)/server_ipu1.xem4
bin/$(BUILD_TYPE)/server_ipu1.xem4: $(OBJS) $(libs) $(CONFIG)/linker.cmd
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(LD) $(LDFLAGS) -o $@ $(OBJS) \
            $(addprefix -l ,$(libs)) $(CONFIG)/linker.cmd $(LDLIBS)

bin/$(BUILD_TYPE)/obj/%.oem4: %.c $(CONFIG)/compiler.opt
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) --output_file=$@ -fc $<

src/eve_firmware.c: src/eve_firmware.h
	touch $@

%/linker.cmd %/compiler.opt: $(CONFIG)/.config ;
$(CONFIG)/.config: $(CONFIGS)
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(XDC_DIR)/xs --xdcpath="$(subst +,;,$(PKGPATH))" \
            xdc.tools.configuro -o $(CONFIG) \
            -t ti.targets.arm.elf.M4 \
            -c $(TI_OCL_M4_CGT_INSTALL) \
            -p ti.platforms.evmDRA7XX:ipu1 \
            -b cfg/config.bld -r $(BUILD_TYPE) \
            -DCORE="IPU1" \
            cfg/Ipu1.cfg
	@$(ECHO) "" > $@

install:
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	@$(MKDIR) $(EXEC_DIR)/debug
	$(CP) bin/debug/server_ipu1.xem4 $(EXEC_DIR)/debug
	@$(MKDIR) $(EXEC_DIR)/release
	$(CP) bin/release/server_ipu1.xem4 $(EXEC_DIR)/release

help:
	@$(ECHO) "make                   # build executable"
	@$(ECHO) "make clean             # clean everything"

clean::
	$(RMDIR) bin

#  ======== install validation ========
ifeq (install,$(MAKECMDGOALS))
ifeq (,$(EXEC_DIR))
$(error must specify EXEC_DIR)
endif
endif

#  ======== toolchain macros ========
CC = $(TI_OCL_M4_CGT_INSTALL)/bin/armcl -c
LD = $(TI_OCL_M4_CGT_INSTALL)/bin/armcl -z

CPPFLAGS =
CFLAGS = -qq -pdsw225 -ppd=$@.dep -ppa $(CCPROFILE_$(BUILD_TYPE)) -@$(CONFIG)/compiler.opt -I.

# entry point is set to an aligned address so that IPC can load the slave
LDFLAGS = -w -q -u _c_int00 -c -m $(@D)/obj/$(@F).map
LDLIBS = -l $(TI_OCL_M4_CGT_INSTALL)/lib/libc.a

CCPROFILE_debug = -D_DEBUG_=1 --symdebug:dwarf
CCPROFILE_release = -O2

#  ======== standard macros ========
ifneq (,$(wildcard $(XDC_DIR)/xdc.exe))
    # use these on Windows
    CP      = $(XDC_DIR)/bin/cp
    ECHO    = $(XDC_DIR)/bin/echo
    MKDIR   = $(XDC_DIR)/bin/mkdir -p
    RM      = $(XDC_DIR)/bin/rm -f
    RMDIR   = $(XDC_DIR)/bin/rm -rf
else
    # use these on Linux
    CP      = cp
    ECHO    = echo
    MKDIR   = mkdir -p
    RM      = rm -f
    RMDIR   = rm -rf
endif

#  ======== create output directories ========
ifneq (clean,$(MAKECMDGOALS))
ifneq (,$(BUILD_TYPE))
ifeq (,$(wildcard bin/$(BUILD_TYPE)/obj))
    $(shell $(MKDIR) -p bin/$(BUILD_TYPE)/obj)
endif
endif
endif
