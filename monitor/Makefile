include ../host/Makefile.inc

# If not specified, assume the opencl source directory is at same location as
# monitor
OPENCL_SRC_DIR ?= ../host

# If not specified, pick a default location for TI dependent packages
DEFAULT_DEV_INSTALL_DIR ?= $(DEFAULT_PSDK_RTOS)
LINUX_DEVKIT_ROOT ?= $(PSDK_LINUX_DEVKIT_ROOT)

ifeq ($(TI_OCL_CGT_INSTALL),)
  $(error Monitor requires TI_OCL_CGT_INSTALL to point to C6000 Codegen tools)
endif

C6636_PDK_DIR  ?= $(wildcard $(DEFAULT_DEV_INSTALL_DIR)/pdk_k2*)
IPC_DIR        ?= $(wildcard $(DEFAULT_DEV_INSTALL_DIR)/ipc_*)
XDC_DIR        ?= $(wildcard $(DEFAULT_DEV_INSTALL_DIR)/xdctools_*)
BIOS_DIR       ?= $(wildcard $(DEFAULT_DEV_INSTALL_DIR)/bios_*)
FC_DIR         ?= $(wildcard $(DEFAULT_DEV_INSTALL_DIR)/framework_components*)
EDMA3LLD_DIR   ?= $(wildcard $(DEFAULT_DEV_INSTALL_DIR)/edma3_lld_*)
XDAIS_DIR      ?= $(wildcard $(DEFAULT_DEV_INSTALL_DIR)/xdais_*)
ULM_DIR        ?= $(LINUX_DEVKIT_ROOT)/usr/share/ti/ulm
GDB_SERVER_DIR ?= $(LINUX_DEVKIT_ROOT)/usr/share/ti/gdbc6x
SDK_DIR        ?= $(DEFAULT_DLSDK)

export XDC = $(XDC_DIR)
export XDC_DIR
export BIOS_DIR

# Verify that the dependencies exist where stated
ifneq ($(MAKECMDGOALS),clean)
    ifeq ($(BUILD_TARGET),DSPC868x)
      ifeq (,$(wildcard $(SDK_DIR)/.)) 
         $(error Path specified for SDK_DIR(=$(SDK_DIR)) is not valid)
      endif
      ifeq (,$(wildcard $(C6678_PDK_DIR)/.))
       $(error Path specified for C6678_PDK_DIR(=$(C6678_PDK_DIR)) is not valid)
      endif
   else
      ifeq (,$(wildcard $(C6636_PDK_DIR)/.))
       $(error Path specified for C6636_PDK_DIR(=$(C6636_PDK_DIR)) is not valid)
      endif
   endif
   ifeq (,$(wildcard $(BIOS_DIR)/.)) 
      $(error Path specified for BIOS_DIR(=$(BIOS_DIR)) is not valid)
   endif
   ifeq (,$(wildcard $(OMP_DIR)/.))
      $(error Path specified for OMP_DIR(=$(OMP_DIR)) is not valid)
   endif
   ifeq (,$(wildcard $(IPC_DIR)/.))
      $(error Path specified for IPC_DIR(=$(IPC_DIR)) is not valid)
   endif
   ifeq (,$(wildcard $(FC_DIR)/.))
      $(error Path specified for FC_DIR(=$(FC_DIR)) is not valid)
   endif
   ifeq (,$(wildcard $(EDMA3LLD_DIR)/.))
      $(error Path specified for EDMA3LLD_DIR(=$(EDMA3LLD_DIR)) is not valid)
   endif
   ifeq (,$(wildcard $(XDAIS_DIR)/.))
      $(error Path specified for XDAIS_DIR(=$(XDAIS_DIR)) is not valid)
   endif
   ifeq (,$(wildcard $(XDC_DIR)/.))
      $(error Path specified for XDC_DIR(=$(XDC_DIR)) is not valid)
   endif

   ifneq ($(BUILD_TARGET),DSPC868x)
   ifeq (,$(wildcard $(ULM_DIR)/.))
      $(error Path specified for ULM_DIR(=$(ULM_DIR)) is not valid)
   endif
   ifeq (,$(wildcard $(GDB_SERVER_DIR)/.))
     $(error Path specified for GDB_SERVER_DIR(=$(GDB_SERVER_DIR)) is not valid)
   endif
   endif
endif

BUILD_TYPE = release

#INCLUDES= -i $(OPENCL_SRC_DIR)/inc \
#	  -i $(OPENCL_SRC_DIR)/src/core/dsp \
#	  -D MAILBOX_ON_HOST 

INCLUDES=  -i $(OPENCL_SRC_DIR)/src/core/dsp

XDCTARGET= ti.targets.elf.C66
XDCPATH_K2X = ./;$(C6636_PDK_DIR)/packages;$(BIOS_DIR)/packages;$(OMP_DIR)/packages;$(EDMA3LLD_DIR)/packages;$(IPC_DIR)/packages;$(FC_DIR)/packages;$(XDAIS_DIR)/packages;
LIBS_K2X  = monitor.cmd $(ULM_DIR)/libtiulm.ae66 $(GDB_SERVER_DIR)/lib/gdb_server.ae66

ifeq ($(BUILD_TARGET),ARM_K2H)
    PLATFORM = evmk2h
    XDCPATH  = $(XDCPATH_K2X)
    INCLUDES += -DDEVICE_K2H
    LIBS = $(LIBS_K2X)
    DSPS := 0 1 2 3 4 5 6 7

else ifeq ($(BUILD_TARGET),ARM_K2L)
    PLATFORM = evmk2l
    XDCPATH  = $(XDCPATH_K2X)
    INCLUDES += -DDEVICE_K2L
    LIBS = $(LIBS_K2X)
    DSPS := 0 1 2 3

else ifeq ($(BUILD_TARGET),ARM_K2E)
    PLATFORM = evmk2e
    XDCPATH  = $(XDCPATH_K2X)
    INCLUDES += -DDEVICE_K2E
    LIBS = $(LIBS_K2X)
    DSPS := 0

else
    ifeq ($(MAKECMDGOALS),clean)
    else ifeq ($(MAKECMDGOALS),realclean)
    else
        $(error must specify one of: \
        BUILD_TARGET={DSPC868x,ARM_K2H,ARM_K2L,ARM_K2E})
    endif
endif

INCLUDES += -DTI_66AK2X -DK2_PG1_MSMC_STALL_WA
INCLUDES += -DOMP_ENABLED
INCLUDES += -i $(ULM_DIR) -DULM_ENABLED
INCLUDES += -i $(GDB_SERVER_DIR)/include -DGDB_ENABLED

BUILD = monitor_$(PLATFORM)
COMP_OPTS = $(BUILD)/compiler.opt
LINK_CMD = $(BUILD)/linker.cmd

.PHONY: binaries syms


build: binaries syms

binaries:  $(foreach dsp,$(DSPS),$(BUILD)/dsp$(dsp).out)
syms:  $(foreach dsp,$(DSPS),$(BUILD)/dsp$(dsp).syms)




#----------------------------------------------------------------------------
# include libm functions in monitor
#----------------------------------------------------------------------------
LIBM_DIR=../libm
LIBM=$(LIBM_DIR)/libm.lib

$(LIBM): 
	@$(MAKE) -C $(LIBM_DIR) -j4 

#----------------------------------------------------------------------------
# include builtins functions in monitor
#----------------------------------------------------------------------------
BUILTINS_DIR=../builtins
BUILTINS=$(BUILTINS_DIR)/dsp.lib

$(BUILTINS): 
	@$(MAKE) -C $(BUILTINS_DIR) -j4 

#---------------------------------------------------------------------------
# build monitor
#----------------------------------------------------------------------------

SOURCES = monitor.c dsp_rpc.asm util.c touch.asm
SOURCES+= builtins.c
SOURCES+= device_k2x.c
SOURCES+= edma.c edma_config.c


HEADERS = edma.h monitor.h util.h ocl_device_defs.h trace.h

OBJS1=$(patsubst %.c,$(BUILD)/%.obj,$(SOURCES))
OBJS=$(patsubst %.asm,$(BUILD)/%.obj,$(OBJS1))

## !Need -O* to inline CACHE_* functions!
CC= $(TI_OCL_CGT_INSTALL)/bin/cl6x -o3 -mv6600 --abi=eabi --gcc --mem_model:const=data --mem_model:data=far -fr=$(BUILD)
#CC= $(TI_OCL_CGT_INSTALL)/bin/cl6x -o0 -g -mv6600 --abi=eabi --gcc --mem_model:const=data --mem_model:data=far -fr=$(BUILD)


$(BUILD)/%.syms: $(BUILD)/%.out
	$(call create_dsp_syms, $<, $@)

$(BUILD)/dsp0.out: $(OBJS) $(BUILTINS) $(LIBM) $(LINK_CMD) $(LIBS)
	@$(CC) -z --define=DDR3_PER_CORE=DDR3_CORE0 --priority -w -x -m$@.map $^ -I$(TI_OCL_CGT_INSTALL)/lib -llibc.a -o $@

$(BUILD)/dsp1.out: $(OBJS) $(BUILTINS) $(LIBM) $(LINK_CMD) $(LIBS)
	@$(CC) -z --define=DDR3_PER_CORE=DDR3_CORE1 --priority -w -x -m$@.map $^ -I$(TI_OCL_CGT_INSTALL)/lib -llibc.a -o $@

$(BUILD)/dsp2.out: $(OBJS) $(BUILTINS) $(LIBM) $(LINK_CMD) $(LIBS)
	@$(CC) -z --define=DDR3_PER_CORE=DDR3_CORE2 --priority -w -x -m$@.map $^ -I$(TI_OCL_CGT_INSTALL)/lib -llibc.a -o $@

$(BUILD)/dsp3.out: $(OBJS) $(BUILTINS) $(LIBM) $(LINK_CMD) $(LIBS)
	@$(CC) -z --define=DDR3_PER_CORE=DDR3_CORE3 --priority -w -x -m$@.map $^ -I$(TI_OCL_CGT_INSTALL)/lib -llibc.a -o $@

$(BUILD)/dsp4.out: $(OBJS) $(BUILTINS) $(LIBM) $(LINK_CMD) $(LIBS)
	@$(CC) -z --define=DDR3_PER_CORE=DDR3_CORE4 --priority -w -x -m$@.map $^ -I$(TI_OCL_CGT_INSTALL)/lib -llibc.a -o $@

$(BUILD)/dsp5.out: $(OBJS) $(BUILTINS) $(LIBM) $(LINK_CMD) $(LIBS)
	@$(CC) -z --define=DDR3_PER_CORE=DDR3_CORE5 --priority -w -x -m$@.map $^ -I$(TI_OCL_CGT_INSTALL)/lib -llibc.a -o $@

$(BUILD)/dsp6.out: $(OBJS) $(BUILTINS) $(LIBM) $(LINK_CMD) $(LIBS)
	@$(CC) -z --define=DDR3_PER_CORE=DDR3_CORE6 --priority -w -x -m$@.map $^ -I$(TI_OCL_CGT_INSTALL)/lib -llibc.a -o $@

$(BUILD)/dsp7.out: $(OBJS) $(BUILTINS) $(LIBM) $(LINK_CMD) $(LIBS)
	@$(CC) -z --define=DDR3_PER_CORE=DDR3_CORE7 --priority -w -x -m$@.map $^ -I$(TI_OCL_CGT_INSTALL)/lib -llibc.a -o $@


#---------------------------------------------------------------------------
# Function to create the dsp symbols database
#----------------------------------------------------------------------------
define create_dsp_syms
	@echo Creating $(2)
	@$(TI_OCL_CGT_INSTALL)/bin/nm6x -l $(1) | grep PROT | awk -F\| '{ print "\t.weak "$$8"\n"$$8"\t.set "$$2 };' >  $(2).asm
	@$(TI_OCL_CGT_INSTALL)/bin/nm6x -l $(1) | grep FUNC | grep -fexport_syms | awk -F\| '{ print "\t.weak "$$8"\n"$$8"\t.set "$$2 };' >> $(2).asm
	@$(CC) $(2).asm
	@cp dsp_link_commands.$(PLATFORM) $(2)
	@rm -f $(1).syms.tmp* $(2).asm
endef



$(BUILD)/%.obj : %.c $(COMP_OPTS) $(HEADERS)
	@echo Compiling $<
	@$(CC) -@$(COMP_OPTS) $(INCLUDES) -c $< 

$(BUILD)/%.obj : %.asm $(COMP_OPTS)
	@echo Compiling $<
	@$(CC) -@$(COMP_OPTS) $(INCLUDES) -c $< 

$(BUILD)/monitor.obj:  $(OPENCL_SRC_DIR)/src/core/dsp/message.h
$(BUILD)/ocl_qmss.obj: $(OPENCL_SRC_DIR)/src/core/dsp/message.h


#--------------------------------------------------------------------------
# generate and build config packages
#--------------------------------------------------------------------------

$(COMP_OPTS): $(LINK_CMD)

$(LINK_CMD): \
    monitor.cfg \
    ocl_platforms/$(PLATFORM)/package/package_ocl_platforms.$(PLATFORM).c
	@echo Making $(BUILD) files
	@$(XDC_DIR)/xs --xdcpath "$(XDCPATH)" xdc.tools.configuro -c $(TI_OCL_CGT_INSTALL) --cb -t $(XDCTARGET) -p ocl_platforms.$(PLATFORM) -r $(BUILD_TYPE) -o $(BUILD) monitor.cfg
	@sed '/c6xabi\.exidx/d' $(LINK_CMD) > lnk_tmp
	@mv lnk_tmp $(LINK_CMD)

ocl_platforms/$(PLATFORM)/package/package_ocl_platforms.$(PLATFORM).c: \
    ocl_platforms/$(PLATFORM)/Platform.xdc
	 $(MAKE) BIOS_DIR=$(BIOS_DIR) -C ocl_platforms/$(PLATFORM) 


#--------------------------------------------------------------------------
# clean
#--------------------------------------------------------------------------

.PHONY: clean
clean:
	@rm -fr monitor_dspc868x monitor_evmk2h monitor_evmk2l monitor_evmk2e
	@$(MAKE) -C ocl_platforms/dspc868x clean
	@$(MAKE) -C ocl_platforms/evmk2h clean
	@$(MAKE) -C ocl_platforms/evmk2l clean
	@$(MAKE) -C ocl_platforms/evmk2e clean

.PHONY: realclean
realclean: clean
	@rm -fr libm.cmd libm.fcns builtins.cmd builtins.fcns
	@$(MAKE) -C $(LIBM_DIR) clean
	@$(MAKE) -C $(BUILTINS_DIR) clean
