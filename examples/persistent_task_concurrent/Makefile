EXE         = persistent_task_concurrent
CXX_FLAGS   = -O3 -I../persistent_common
CL6X_FLAGS  = -o2 -I../persistent_common --mem_model:data=far --mem_model:const=data
CLOCL_FLAGS = 

VPATH = ../persistent_common

include ../make.inc
DSP_INCLUDE += -I$(BIOS_DIR) -I$(XDC_DIR) -I$(IPC_DIR)

$(EXE): host.o host_assist.o
	@echo Linking $@
	$(CXX) $(CXX_FLAGS) $^ $(LD_FLAGS) $(LIBS) -o $@

host.o: kernel.dsp_h

kernel.dsp_h: dsp.obj
