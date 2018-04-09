#
#  Copyright (c) 2013-2018 Texas Instruments Incorporated - http://www.ti.com
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
PDK_DIR        ?= $(SDOMC_SHARED)/$(PSDK_VISION)/ti_components/drivers/pdk_01_09_00_17

CSL_LIB_M4    ?= $(PDK_DIR)/packages/ti/csl/lib/tda2xx/m4/release/ti.csl.aem4
PM_HAL_LIB_M4 ?= $(PDK_DIR)/packages/ti/drv/pm/lib/tda2xx/m4/release/pm_hal.aem4
SBL_LIB_M4    ?= lib/sbl_lib_1_9_0_9.aem4

SOURCES  = main_ipu1.c dla_firmware.c
SOURCES += utils_eveloader.c
SOURCES += utils_eve_boot.c
HEADERS  = custom_rsc_table_vayu_ipu1.h
HEADERS += $(OPENCL_SRC_DIR)/src/core/dsp/message.h
HEADERS += $(OPENCL_SRC_DIR)/src/core/dsp/tal/mbox_msgq_shared.h
HEADERS += sbl_lib.h sbl_lib_tda2xx.h utils_eveloader.h

VPATH = src

OBJS  = $(addprefix bin/$(BUILD_TYPE)/obj/,$(patsubst %.c,%.oem4,$(notdir $(SOURCES))))
CONFIG = bin/$(BUILD_TYPE)/configuro
CONFIGS  = cfg/Ipu1.cfg cfg/IpuAmmu.cfg cfg/config.bld
CONFIGS += cfg/ipc.cfg.xs cfg/mem_segment_definition_linux.xs

PKGPATH := $(BIOS_DIR)/packages
PKGPATH := $(PKGPATH)+$(IPC_DIR)/packages
PKGPATH := $(PKGPATH)+$(XDC_DIR)/packages
PKGPATH := $(PKGPATH)+$(PDK_DIR)/packages

ifeq ($(BUILD_DLA_FIRMWARE),1)
DLA_FIRMWARE = monitor_dla/src/dla_firmware.h
else
DLA_FIRMWARE = src/dla_firmware.h
endif
INCLUDES  = -I. -Isrc -I $(OPENCL_SRC_DIR)/src/core/dsp
INCLUDES += -D$(DEVICE) -D$(SOC) -DNON_VISIONSDK_BUILD
INCLUDES += -DIPU1_LOAD_EVES -DPROC_EVE1_INCLUDE
INCLUDES += -DPROC_EVE2_INCLUDE -DPROC_EVE3_INCLUDE -DPROC_EVE4_INCLUDE
INCLUDES += -DDLA_FIRMWARE="$(DLA_FIRMWARE)"

libs = $(CSL_LIB_M4) $(PM_HAL_LIB_M4) $(SBL_LIB_M4)

.PRECIOUS: %/compiler.opt %/linker.cmd

# always check if dla_firmware needs to be rebuild
.PHONY: dla_firmware

all: dla_firmware
	$(MAKE) server_ipu1.x

dla_firmware:
ifeq ($(BUILD_DLA_FIRMWARE),1)
	@$(ECHO) "Building development version of DLA firmware..."
	$(MAKE) -C monitor_dla CORE=eve BUILD_AM57=1 -j4
	@diff monitor_dla/src/dla_firmware.h src/dla_firmware.h > /dev/null ||\
		$(ECHO) "\n"\
	            "***********************************************************\n"\
		        "* dla_firmware.h: monitor_ipu/src/dla_firmware.h not in \n"\
	            "* sync with monitor_dla/src/dla_firmware.h.\n"\
				"* Copy dla_firmware.h into ti-opencl repo and commit.\n"\
	            "***********************************************************\n"
else
	@$(ECHO) "Using prebuilt DLA firmware..."
endif

server_ipu1.x: bin/$(BUILD_TYPE)/server_ipu1.xem4
bin/$(BUILD_TYPE)/server_ipu1.xem4: $(OBJS) $(libs) $(CONFIG)/linker.cmd
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(LD) $(LDFLAGS) -o $@ $(OBJS) \
            $(addprefix -l ,$(libs)) $(CONFIG)/linker.cmd $(LDLIBS)

bin/$(BUILD_TYPE)/obj/%.oem4: %.c $(CONFIG)/compiler.opt $(HEADERS)
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) --output_file=$@ -fc $<

src/dla_firmware.c: $(DLA_FIRMWARE)
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

help:
	@$(ECHO) "make                   # build executable"
	@$(ECHO) "make clean             # clean everything"

clean::
	$(RMDIR) bin
	if [ -e monitor_dla/Makefile ]; then \
		$(MAKE) -C monitor_dla CORE=eve clean; \
		$(MAKE) -C monitor_dla CORE=dsp clean; \
	fi

#  ======== toolchain macros ========
CC = $(TI_OCL_M4_CGT_INSTALL)/bin/armcl -c
LD = $(TI_OCL_M4_CGT_INSTALL)/bin/armcl -z

CPPFLAGS =
CFLAGS = -qq -pdsw225 -ppd=$@.dep -ppa $(CCPROFILE_$(BUILD_TYPE)) -@$(CONFIG)/compiler.opt

# entry point is set to an aligned address so that IPC can load the slave
LDFLAGS = -w -x -q -u _c_int00 -c -m $(@D)/obj/$(@F).map
LDLIBS = -l $(TI_OCL_M4_CGT_INSTALL)/lib/libc.a

CCPROFILE_debug = -D_DEBUG_=1 --symdebug:dwarf
CCPROFILE_release = -O2

CP      = cp
ECHO    = echo
MKDIR   = mkdir -p
RM      = rm -f
RMDIR   = rm -rf

#  ======== create output directories ========
ifneq (clean,$(MAKECMDGOALS))
ifneq (,$(BUILD_TYPE))
ifeq (,$(wildcard bin/$(BUILD_TYPE)/obj))
    $(shell $(MKDIR) -p bin/$(BUILD_TYPE)/obj)
endif
endif
endif
