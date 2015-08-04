include ../host/Makefile.inc

# If not specified, assume the opencl source directory is at same location as
# monitor
OPENCL_SRC_DIR ?= ../host

# If not specified, pick a default location for TI dependent packages
DEFAULT_DEV_INSTALL_DIR ?= $(DEFAULT_PSDK_RTOS)
LINUX_DEVKIT_ROOT       ?= $(PSDK_LINUX_DEVKIT_ROOT)

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

AM57_PDK_DIR   ?= $(DEFAULT_DEV_INSTALL_DIR)/pdk_am57xx_1_0_0
IPC_DIR        ?= $(DEFAULT_DEV_INSTALL_DIR)/ipc_3_40_01_08
XDC_DIR        ?= $(DEFAULT_DEV_INSTALL_DIR)/xdctools_3_31_02_38_core
BIOS_DIR       ?= $(DEFAULT_DEV_INSTALL_DIR)/bios_6_42_02_29
OMP_DIR        ?= $(DEFAULT_DEV_INSTALL_DIR)/openmp_dsp_2_01_16_03
FC_DIR         ?= $(DEFAULT_DEV_INSTALL_DIR)/framework_components_3_40_01_04
EDMA3LLD_DIR   ?= $(DEFAULT_DEV_INSTALL_DIR)/edma3_lld_02_12_01_22
XDAIS_DIR      ?= $(DEFAULT_DEV_INSTALL_DIR)/xdais_7_24_00_04
ULM_DIR        ?= $(LINUX_DEVKIT_ROOT)/usr/share/ti/ulm
GDB_SERVER_DIR ?= $(LINUX_DEVKIT_ROOT)/usr/share/ti/gdbc6x

export XDC_DIR
export BIOS_DIR

PLATFORM  = am57x
PROJECT   = monitor
DEVICE    = DEVICE_AM572x
SOC       = SOC_AM572x

## !Need -O* to inline CACHE_* functions!
CC= $(TI_OCL_CGT_INSTALL)/bin/cl6x -o3 -mv6600 --abi=eabi --gcc --mem_model:const=data --mem_model:data=far

INCLUDES  = -i. -i $(OPENCL_SRC_DIR)/src/core/dsp
INCLUDES += -i $(AM57_PDK_DIR)/packages -i $(TI_OCL_CGT_INSTALL)/include \
	    -i $(IPC_DIR)/packages -i $(XDC_DIR)/packages           \
	    -i $(BIOS_DIR)/packages -i $(EDMA3LLD_DIR)/packages     \
	    -i $(FC_DIR)/packages -i $(XDAIS_DIR)/packages
INCLUDES += -D$(DEVICE)
INCLUDES += -D$(SOC)
INCLUDES += -i $(ULM_DIR) -DULM_ENABLED
INCLUDES += -i $(GDB_SERVER_DIR)/include -DGDB_ENABLED

SOURCES = monitor.c util.c dsp_rpc.asm touch.asm builtins.c edma.c \
          edma3_config.c
HEADERS = edma.h monitor.h util.h trace.h custom_rsc_table_vayu_dsp.h
LIBS    = monitor.cmd
LIBS   += $(ULM_DIR)/libtiulm.ae66
LIBS   += $(GDB_SERVER_DIR)/lib/gdb_server.ae66
OBJS1   = $(patsubst %.c,%.obj,$(SOURCES))
OBJS    = $(patsubst %.asm,%.obj,$(OBJS1))

LINK_CMD  = $(PROJECT)/linker.cmd


#----------------------------------------------------------------------------
# XDC stuff
#----------------------------------------------------------------------------
XDCPATH = ./;$(IPC_DIR)/packages;$(BIOS_DIR)/packages;$(EDMA3LLD_DIR)/packages;$(FC_DIR)/packages;$(XDAIS_DIR)/packages;$(OMP_DIR)/packages;$(AM57_PDK_DIR)/packages;

XDCTARGET = ti.targets.elf.C66

COMP_OPTS  = $(PROJECT)/compiler.opt
BUILD_TYPE = release


#----------------------------------------------------------------------------
# LIBM
#----------------------------------------------------------------------------
LIBM_DIR=../libm
LIBM=$(LIBM_DIR)/libm.lib


#----------------------------------------------------------------------------
# BUILTINS
#----------------------------------------------------------------------------
BUILTINS_DIR=../builtins
BUILTINS=$(BUILTINS_DIR)/dsp.lib


#---------------------------------------------------------------------------
# Function to create the dsp symbols database
#----------------------------------------------------------------------------
define create_dsp_syms
	@echo Creating $(2)
	@$(TI_OCL_CGT_INSTALL)/bin/nm6x -l $(1) | grep PROT | awk -F\| '{ print "\t.weak "$$8"\n"$$8"\t.set "$$2 };' >  $(2).asm
	@$(TI_OCL_CGT_INSTALL)/bin/nm6x -l $(1) | grep FUNC | grep -fexport_syms | awk -F\| '{ print "\t.weak "$$8"\n"$$8"\t.set "$$2 };' >> $(2).asm
	@$(CC) $(2).asm
	@cp dsp_link_commands $(2)
	@rm -f $(1).syms.tmp* $(2).asm
endef



.PHONY: all dsp0 dsp1
all: dsp0 dsp1
	@diff -q dsp0.syms dsp1.syms

dsp0 : $(PROJECT)/linker.cmd dsp0.out dsp0.syms
dsp1 : $(PROJECT)/linker.cmd dsp1.out dsp1.syms


#---------------------------------------------------------------------------
# Build monitors
#----------------------------------------------------------------------------
dsp0.out: custom_rsc_table_vayu_dsp0.obj $(OBJS) $(BUILTINS) $(LIBM) $(LINK_CMD) $(LIBS)
	@echo Linking $@
	@echo $(LINK_CMD)
	$(CC) -z --priority -w -x -m$@.map --define=DDR3_FCSECTION=DDR3_FC0 $^ -I$(TI_OCL_CGT_INSTALL)/lib -llibc.a -o $@

dsp1.out: custom_rsc_table_vayu_dsp1.obj $(OBJS) $(BUILTINS) $(LIBM) $(LINK_CMD) $(LIBS)
	@echo Linking $@
	@echo $(LINK_CMD)
	$(CC) -z --priority -w -x -m$@.map --define=DDR3_FCSECTION=DDR3_FC1 $^ -I$(TI_OCL_CGT_INSTALL)/lib -llibc.a -o $@

%.syms: %.out
	$(call create_dsp_syms, $^, $@)

$(LIBM): 
	$(MAKE) -C $(LIBM_DIR) -j4 

$(BUILTINS): 
	$(MAKE) -C $(BUILTINS_DIR) -j4 


ocl_platforms/am57x/package/package_ocl_platforms.am57x.c: ocl_platforms/am57x/Platform.xdc
	 $(MAKE) -C ocl_platforms/am57x


monitor/linker.cmd : monitor.cfg ocl_platforms/am57x/package/package_ocl_platforms.am57x.c
	$(XDC_DIR)/xs --xdcpath "$(XDCPATH)" xdc.tools.configuro -c $(TI_OCL_CGT_INSTALL) --cb -t $(XDCTARGET) -p ocl_platforms.$(PLATFORM) -r $(BUILD_TYPE) $(PROJECT).cfg
	@sed '/c6xabi\.exidx/d' $(LINK_CMD) > lnk_tmp
	@mv lnk_tmp $(LINK_CMD)


#---------------------------------------------------------------------------
# Default build rules
#----------------------------------------------------------------------------
%.obj : %.c $(HEADERS)
	@echo Compiling $<
	$(CC) $(INCLUDES) -c $< 

%.obj : %.asm 
	@echo Compiling $<
	$(CC) $(INCLUDES) -c $< 

.PHONY: clean
clean:
	@rm -fr *.out *.map *.syms *.obj libm.cmd libm.fcns builtins.cmd builtins.fcns $(PROJECT)

.PHONY: realclean
realclean: clean
	@$(MAKE) -C $(LIBM_DIR) clean
	@$(MAKE) -C $(BUILTINS_DIR) clean

