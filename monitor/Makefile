# If not specified, assume the opencl source directory is at same location as
# monitor
OPENCL_SRC_DIR ?= ../host

# If not specified, pick a default location for TI dependent packages
DEFAULT_DEV_INSTALL_DIR ?= /opt/ti

# And for the devkit, when doing an x86 build ...
UNAME_M :=$(shell uname -m)

ifneq (,$(findstring 86, $(UNAME_M)))
   LINUX_DEVKIT_ROOT ?= $(DEFAULT_DEV_INSTALL_DIR)/mcsdk_linux_3_01_04_07/linux-devkit/arago-2015.05/sysroots/cortexa15t2hf-vfp-neon-linux-gnueabi
endif

ifeq ($(TI_OCL_CGT_INSTALL),)
  $(error Monitor requires TI_OCL_CGT_INSTALL to point to C6000 Codegen tools)
endif

SDK_DIR        ?= $(DEFAULT_DEV_INSTALL_DIR)/desktop-linux-sdk_01_00_03_00
MPM_DIR        ?= $(DEFAULT_DEV_INSTALL_DIR)/mpm_2_00_01_10
BIOS_DIR       ?= $(DEFAULT_DEV_INSTALL_DIR)/bios_6_41_04_54
OMP_DIR        ?= $(DEFAULT_DEV_INSTALL_DIR)/openmp_dsp_2_01_17_01
IPC_DIR        ?= $(DEFAULT_DEV_INSTALL_DIR)/ipc_3_36_02_13
FC_DIR         ?= $(DEFAULT_DEV_INSTALL_DIR)/framework_components_3_40_00_02
EDMA3LLD_DIR   ?= $(DEFAULT_DEV_INSTALL_DIR)/edma3_lld_02_11_13_17
XDAIS_DIR      ?= $(DEFAULT_DEV_INSTALL_DIR)/xdais_7_24_00_04
C6678_PDK_DIR  ?= $(DEFAULT_DEV_INSTALL_DIR)/pdk_C6678_2_1_3_7
C6636_PDK_DIR  ?= $(DEFAULT_DEV_INSTALL_DIR)/pdk_keystone2_3_01_04_07
XDC_DIR        ?= $(DEFAULT_DEV_INSTALL_DIR)/xdctools_3_31_02_38_core
ULM_DIR        ?= $(LINUX_DEVKIT_ROOT)/usr/share/ti/ulm
GDB_SERVER_DIR ?= $(LINUX_DEVKIT_ROOT)/usr/share/ti/gdbc6x

export XDC = $(XDC_DIR)
export BIOS_DIR = $(BIOS_DIR)

# Verify that the dependencies exist where stated
ifneq ($(MAKECMDGOALS),clean)
   ifneq ($(MAKECMDGOALS),k2h)
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
   ifeq (,$(wildcard $(MPM_DIR)/.)) 
      $(error Path specified for MPM_DIR(=$(MPM_DIR)) is not valid)
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
   ifeq (,$(wildcard $(ULM_DIR)/.))
      $(error Path specified for ULM_DIR(=$(ULM_DIR)) is not valid)
   endif
   ifeq (,$(wildcard $(GDB_SERVER_DIR)/.))
     $(error Path specified for GDB_SERVER_DIR(=$(GDB_SERVER_DIR)) is not valid)
   endif
endif

PROJECT    = monitor
COMP_OPTS  = $(PROJECT)/compiler.opt
LINK_CMD   = $(PROJECT)/linker.cmd
BUILD_TYPE = release

## !Need -O* to inline CACHE_* functions!
CC= cl6x -o3 -mv6600 --abi=eabi --gcc --mem_model:const=data --mem_model:data=far
#CC= cl6x -o0 -g -mv6600 --abi=eabi --gcc --mem_model:const=data --mem_model:data=far

#INCLUDES= -i $(OPENCL_SRC_DIR)/inc \
#	  -i $(OPENCL_SRC_DIR)/src/core/dsp \
#	  -D MAILBOX_ON_HOST 

INCLUDES=  -i $(OPENCL_SRC_DIR)/src/core/dsp \
           -D MAILBOX_ON_HOST 

SOURCES = monitor.c dsp_rpc.asm util.c touch.asm sem.c 
SOURCES+= osal.c qmss_device.c  
SOURCES+= omp_config_api.c
SOURCES+= builtins.c
SOURCES+= ocl_qmss.c
SOURCES+= edma.c edma_config.c
#SOURCES+= em_pdk_hal.c ocl_em_init.c ti_em_init.c cppi_device.c 

HEADERS = edma.h monitor.h util.h ocl_device_defs.h trace.h ocl_qmss.h

OBJS1  =$(patsubst %.c,%.obj,$(SOURCES))
OBJS   =$(patsubst %.asm,%.obj,$(OBJS1))
XDCPATH=

dspc: override PDK_DIR = $(C6678_PDK_DIR)
dspc: override PLATFORM = dspc868x
dspc: override XDCPATH = ./;$(PDK_DIR)/packages;$(BIOS_DIR)/packages;$(OMP_DIR)/packages;$(IPC_DIR)/packages
dspc: INCLUDES += -DTI_C6678 \
	          -i $(SDK_DIR)/sdk/pciedrv \
	          -i $(SDK_DIR)/sdk/mailBox \
	          -i $(SDK_DIR)/sdk/mailBox/inc \
	          -i $(SDK_DIR)/sdk/buffmgr \
	          -i $(SDK_DIR)/sdk/config \
                  -i $(PDK_DIR)/packages/ti/drv/qmss
dspc: $(COMP_OPTS) $(LINK_CMD)  dsp_k1

k2h: override PDK_DIR = $(C6636_PDK_DIR)
k2h: override PLATFORM = evmk2h
k2h: override XDCPATH  = ./;$(PDK_DIR)/packages;$(BIOS_DIR)/packages;$(OMP_DIR)/packages;$(EDMA3LLD_DIR)/packages;$(IPC_DIR)/packages;$(FC_DIR)/packages;$(XDAIS_DIR)/packages;
k2h: INCLUDES += -DDEVICE_K2K -DTI_66AK2H -DK2_PG1_MSMC_STALL_WA \
	         -i $(MPM_DIR)/include -i$(ULM_DIR) -DULM_ENABLED -i $(GDB_SERVER_DIR)/include -DGDB_ENABLED
k2h: $(COMP_OPTS) $(LINK_CMD)  dsp_k2h

K1LIBS      = monitor_k1.cmd 
K2LIBS      = monitor.cmd 
DSPC_LIBS = $(K1LIBS) $(SDK_DIR)/sdk/mailBox/c66x/lib/mailBox.a
K2H_LIBS  = $(K2LIBS) $(MPM_DIR)/lib_c66x/mpmmailbox.ae66 $(ULM_DIR)/libtiulm.ae66 $(GDB_SERVER_DIR)/lib/gdb_server.ae66

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
# Function to create the dsp symbols database
#----------------------------------------------------------------------------
define create_dsp_syms
	@echo Creating dsp.syms
	@nm6x -l dsp.out | grep PROT | awk -F\| '{ print "\t.weak "$$8"\n"$$8"\t.set "$$2 };' >  dsp_syms.asm
	@nm6x -l dsp.out | grep FUNC | grep -fexport_syms | awk -F\| '{ print "\t.weak "$$8"\n"$$8"\t.set "$$2 };' >> dsp_syms.asm
	@$(CC) dsp_syms.asm
	@cp dsp_link_commands dsp.syms
	@rm -f $1.syms.tmp* dsp_syms.asm
endef

#---------------------------------------------------------------------------
# Build k1 monitor
#----------------------------------------------------------------------------
dsp_k1: $(OBJS) $(BUILTINS) $(LIBM) $(LINK_CMD) $(DSPC_LIBS)
	@echo Linking $@
	@$(CC) -z --priority -w -x -m$@.map $^ -I$(TI_OCL_CGT_INSTALL)/lib -llibc.a -o dsp.out
	$(call create_dsp_syms, $@)

#---------------------------------------------------------------------------
# Build k2h monitor
#----------------------------------------------------------------------------
dsp_k2h: $(OBJS) $(BUILTINS) $(LIBM) $(LINK_CMD) $(K2H_LIBS)
	@echo Linking $@
	@$(CC) -z --priority -w -x -m$@.map $^ -I$(TI_OCL_CGT_INSTALL)/lib -llibc.a -o dsp.out
	$(call create_dsp_syms, $@)

%.obj : %.c $(COMP_OPTS) $(HEADERS)
	@echo Compiling $<
	@$(CC) -@$(COMP_OPTS) $(INCLUDES) -c $< 

%.obj : %.asm $(COMP_OPTS)
	@echo Compiling $<
	@$(CC) -@$(COMP_OPTS) $(INCLUDES) -c $< 

.PHONY: clean
clean:
	@rm -fr *.out $(OBJS) *.map $(PROJECT) $(BUILD_TYPE) *.syms *.obj
	@$(MAKE) -C ocl_platforms/dspc868x clean
	@$(MAKE) -C ocl_platforms/evmk2h clean

.PHONY: realclean
realclean: clean
	@rm -fr libm.cmd libm.fcns builtins.cmd builtins.fcns
	@$(MAKE) -C $(LIBM_DIR) clean
	@$(MAKE) -C $(BUILTINS_DIR) clean

#--------------------------------------------------------------------------
# Make sure the platform is built before we try to build the monitor.
#--------------------------------------------------------------------------
ocl_platforms/dspc868x/package/package_ocl_platforms.dspc868x.c: ocl_platforms/dspc868x/Platform.xdc
	 $(MAKE) BIOS_DIR=$(BIOS_DIR) -C ocl_platforms/dspc868x 

ocl_platforms/evmk2h/package/package_ocl_platforms.evmk2h.c: ocl_platforms/evmk2h/Platform.xdc
	 $(MAKE) BIOS_DIR=$(BIOS_DIR) -C ocl_platforms/evmk2h

#--------------------------------------------------------------------------
# generate and build config packages
#--------------------------------------------------------------------------
XDCTARGET= ti.targets.elf.C66

$(COMP_OPTS) : $(LINK_CMD)

$(LINK_CMD): $(PROJECT).cfg ocl_platforms/dspc868x/package/package_ocl_platforms.dspc868x.c ocl_platforms/evmk2h/package/package_ocl_platforms.evmk2h.c
	@echo Making $(PROJECT) files
	@mkdir -p $(BUILD_TYPE)
	@$(XDC)/xs --xdcpath "$(XDCPATH)" xdc.tools.configuro -c $(TI_OCL_CGT_INSTALL) --cb -t $(XDCTARGET) -p ocl_platforms.$(PLATFORM) -r $(BUILD_TYPE) $(PROJECT).cfg
	@sed '/c6xabi\.exidx/d' $(LINK_CMD) > lnk_tmp
	@mv lnk_tmp $(LINK_CMD)


monitor.obj:  $(OPENCL_SRC_DIR)/src/core/dsp/message.h
ocl_qmss.obj: $(OPENCL_SRC_DIR)/src/core/dsp/message.h
