include ../Makefile.inc

SDK_DIR?=$(DEFAULT_DLSDK)

ifeq ($(TI_OCL_CGT_INSTALL),)
  $(error init requires TI_OCL_CGT_INSTALL to point to C6000 Codegen tools)
endif

ifneq ($(MAKECMDGOALS),clean)
    ifeq (,$(wildcard $(SDK_DIR)/.)) 
        $(error Path specified for SDK_DIR(=$(SDK_DIR)) is not valid)
    endif
    ifeq (,$(wildcard $(C6678_PDK1_DIR)/.))
        $(error Path specified for C6678_PDK1_DIR(=$(C6678_PDK1_DIR)) is not valid)
    endif
endif

CC=$(TI_OCL_CGT_INSTALL)/bin/cl6x -g -mv6600 --abi=eabi --mem_model:const=data 

INCLUDES= -i $(C6678_PDK1_DIR)/packages/ti/platform \
          -i $(C6678_PDK1_DIR)/packages \
          -i $(TI_OCL_CGT_INSTALL)/include \
	  -i $(SDK_DIR)

LIBS = $(TI_OCL_CGT_INSTALL)/lib/libc.a init.cmd

all: init_dspc8681.out init_dspc8682.out

init_dspc8681.out: main.obj
	@$(CC) -z -w -x --rom_model $^ $(C6678_PDK1_DIR)/packages/ti/platform/dspc8681/platform_lib/lib/debug/ti.platform.dspc8681.lite.lib $(LIBS) -o $@ -m init_dspc8681.map

init_dspc8682.out: main.obj
	@$(CC) -z -w -x --rom_model $^ $(C6678_PDK1_DIR)/packages/ti/platform/dspc8682/platform_lib/lib/debug/ti.platform.dspc8682.lite.lib $(LIBS) -o $@ -m init_dspc8682.map

%.obj : %.c 
	@$(CC) $(INCLUDES) -c $< 

.PHONY: clean
clean:
	@rm -fr *.obj *.out *.map
