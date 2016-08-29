#
#  Copyright (c) 2012-2015 Texas Instruments Incorporated - http://www.ti.com
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
#  ======== make_rtos.inc ========
#
ifeq ($(OS), Windows_NT)
DEFAULT_PSDK_RTOS      ?= C:\ti\ti-processor-sdk-rtos-am57xx-evm-03.00.00.02
GCC_ARM_NONE_TOOLCHAIN ?= C:\ti\ccsv6\tools\compiler\gcc-arm-none-eabi-4_9-2015q3

TI_OCL_CGT_INSTALL     ?= C:\ti\ccsv6\tools\compiler\ti-cgt-c6000_8.1.1
TI_OCL_INSTALL         ?= C:\ti\opencl_rtos_am57xx_01_01_09_01\packages\ti\opencl

else
DEFAULT_PSDK_RTOS      ?= /cgnas/ti-processor-sdk-rtos-am57xx-evm-03.00.00.04-release
GCC_ARM_NONE_TOOLCHAIN ?= /cgnas/gcc-arm-none-eabi-4_9-2015q3

TI_OCL_CGT_INSTALL     ?= /cgnas/ti-cgt-c6000-8.1.0-release-linux
TI_OCL_INSTALL         ?= /cgnas/opencl_rtos_am57xx_01_01_10_00-20160829/packages/ti/opencl
endif

BIOS_INSTALL_DIR       ?= $(wildcard $(DEFAULT_PSDK_RTOS)/bios_*)
IPC_INSTALL_DIR        ?= $(wildcard $(DEFAULT_PSDK_RTOS)/ipc_*)
XDC_INSTALL_DIR        ?= $(wildcard $(DEFAULT_PSDK_RTOS)/xdctools_*)

PROFILE                ?= release
HOSTCONFIG             ?= $(OCL_INSTALL_DIR)/packages/ti/opencl/Host.cfg

export TI_OCL_INSTALL
export TI_OCL_CGT_INSTALL
export PATH:=$(TI_OCL_CGT_INSTALL)/bin:$(PATH)


CC                = $(GCC_ARM_NONE_TOOLCHAIN)/bin/arm-none-eabi-gcc
CXX               = $(GCC_ARM_NONE_TOOLCHAIN)/bin/arm-none-eabi-g++
LD                = $(GCC_ARM_NONE_TOOLCHAIN)/bin/arm-none-eabi-gcc
OCL_INSTALL_DIR   = $(TI_OCL_INSTALL)/../../..
ifeq ($(OS), Windows_NT)
CLOCL             = $(TI_OCL_INSTALL)/usr/share/ti/opencl/bin/pc/clocl
else
CLOCL             = $(TI_OCL_INSTALL)/usr/share/ti/opencl/bin/x86/clocl
endif
DSP_INCLUDE       = -I$(TI_OCL_CGT_INSTALL)/include
DSP_INCLUDE      += -I$(TI_OCL_INSTALL)/usr/share/ti/opencl
CL6X              = $(TI_OCL_CGT_INSTALL)/bin/cl6x -mv6600 --abi=eabi $(DSP_INCLUDE)


# Use this goal to print your product variables.
.show:
	@echo "BIOS_INSTALL_DIR       = $(BIOS_INSTALL_DIR)"
	@echo "IPC_INSTALL_DIR        = $(IPC_INSTALL_DIR)"
	@echo "XDC_INSTALL_DIR        = $(XDC_INSTALL_DIR)"
	@echo "OCL_INSTALL_DIR        = $(OCL_INSTALL_DIR)"
	@echo "GCC_ARM_NONE_TOOLCHAIN = $(GCC_ARM_NONE_TOOLCHAIN)"
	@echo "TI_OCL_CGT_INSTALL     = $(TI_OCL_CGT_INSTALL)"
	@echo "TI_OCL_INSTALL         = $(TI_OCL_INSTALL)"


PROCLIST    = host
CONFIG      = bin/$(PROFILE)/cfg


objs = $(addprefix bin/$(PROFILE)/,$(patsubst %.cpp,%.oa15fg,$(srcs)))
libs = -Wl,-T,$(CONFIG)/linker.cmd -lrdimon -lgcc -lc -lm -lnosys -lstdc++ \
       -Wl,--start-group -Wl,--end-group

.PRECIOUS: %/compiler.opt %/linker.cmd

$(EXE).x: bin/$(PROFILE)/$(EXE).xa15fg
bin/$(PROFILE)/$(EXE).xa15fg: $(objs) $(CONFIG)/linker.cmd
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(LD) $(LDFLAGS) -Wl,-Map,"$(EXE).map" -o $@ $(objs) $(libs) $(LDLIBS)
	$(RM) *.map
	$(RMDIR) $(CONFIG)

bin/$(PROFILE)/%.oa15fg: %.cpp $(CONFIG)/compiler.opt
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(CXX) $(CPPFLAGS) $(CFLAGS) -o $@ $<

bin/$(PROFILE)/%.oa15fg: %.c $(CONFIG)/compiler.opt
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(CXX) $(CPPFLAGS) $(CFLAGS) -o $@ $<

%.obj: %.c
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(CL6X) -c $(CL6X_FLAGS) $<

%.dsp_h: %.cl
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(CLOCL) --txt $(CLOCL_FLAGS) $^

%/compiler.opt: %/linker.cmd ;
$(CONFIG)/compiler.opt: $(CONFIG)/linker.cmd ;

$(CONFIG)/linker.cmd: $(HOSTCONFIG)
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(XDC_INSTALL_DIR)/xs --xdcpath="$(subst +,;,$(PKGPATH))" \
            xdc.tools.configuro -o $(CONFIG) \
            -t gnu.targets.arm.A15F \
            -c $(GCC_ARM_NONE_TOOLCHAIN) \
            -p ti.opencl.platforms.am57x_rtos:HOST \
            -r $(PROFILE) \
	    $(HOSTCONFIG)

help:
	@$(ECHO) "make                   # build executable"
	@$(ECHO) "make clean             # clean everything"

clean::
	$(RMDIR) bin *.out *.dsp_h *.obj
	$(RM) *.map


PKGPATH := $(BIOS_INSTALL_DIR)/packages
PKGPATH := $(PKGPATH)+$(IPC_INSTALL_DIR)/packages
PKGPATH := $(PKGPATH)+$(OCL_INSTALL_DIR)/packages

#  ======== install validation ========
ifeq (install,$(MAKECMDGOALS))
ifeq (,$(EXEC_DIR))
$(error must specify EXEC_DIR)
endif
endif

#  ======== toolchain macros ========

CPPFLAGS += -Dfar= -D__DYNAMIC_REENT__ -D_TI_RTOS -fpermissive -std=c++0x
CFLAGS   += -c -MD -MF $@.dep -MMD -MP -mcpu=cortex-a15 -mtune=cortex-a15 -mfloat-abi=hard -ffunction-sections -fdata-sections $(CCPROFILE_$(PROFILE)) @$(CONFIG)/compiler.opt -I"$(GCC_ARM_NONE_TOOLCHAIN)/arm-none-eabi/include" -I. -I$(TI_OCL_INSTALL)/usr/include

LDFLAGS += $(LDPROFILE_$(PROFILE)) -mfloat-abi=hard -nostartfiles -Wl,-static -Wl,--gc-sections -Wl,-Map=$(@D)/$(@F).map
LDLIBS += -L$(XDC_INSTALL_DIR)/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/lib/fpu

CCPROFILE_debug = -g -ggdb -D_DEBUG_=1
CCPROFILE_release = -O3

LDPROFILE_debug = -g -ggdb
LDPROFILE_release =

#  ======== standard macros ========
ifneq (,$(wildcard $(XDC_INSTALL_DIR)/bin/echo.exe))
    # use these on Windows
    CP      = $(XDC_INSTALL_DIR)/bin/cp
    ECHO    = $(XDC_INSTALL_DIR)/bin/echo
    MKDIR   = $(XDC_INSTALL_DIR)/bin/mkdir -p
    RM      = $(XDC_INSTALL_DIR)/bin/rm -f
    RMDIR   = $(XDC_INSTALL_DIR)/bin/rm -rf
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
ifneq (,$(PROFILE))
ifeq (,$(wildcard bin/$(PROFILE)))
    $(shell $(MKDIR) -p bin/$(PROFILE))
endif
endif
endif
