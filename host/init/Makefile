PDK= /opt/ti/pdk_C6678_1_1_2_5
SDK= /opt/ti/desktop-linux-sdk_01_00_03_00
CG=${TI_OCL_CGT_INSTALL}
CC= $(CG)/bin/cl6x -g -mv6600 --abi=eabi --mem_model:const=data 

INCLUDES= -i $(PDK)/packages/ti/platform \
          -i $(PDK)/packages \
          -i $(CG)/include \
	  -i $(SDK)

LIBS = $(CG)/lib/libc.a init.cmd

all: init_dspc8681.out init_dspc8682.out

init_dspc8681.out: main.obj
	@$(CC) -z -w -x --rom_model $^ $(PDK)/packages/ti/platform/dspc8681/platform_lib/lib/debug/ti.platform.dspc8681.lite.lib $(LIBS) -o $@ -m init_dspc8681.map

init_dspc8682.out: main.obj
	@$(CC) -z -w -x --rom_model $^ $(PDK)/packages/ti/platform/dspc8682/platform_lib/lib/debug/ti.platform.dspc8682.lite.lib $(LIBS) -o $@ -m init_dspc8682.map

%.obj : %.c 
	@$(CC) $(INCLUDES) -c $< 

.PHONY: clean
clean:
	@rm -fr *.obj *.out *.map
