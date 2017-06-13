CL6X_FLAGS  = -o2 -I../../persistent_common --mem_model:data=far --mem_model:const=data
CLOCL_FLAGS = 

include ../../make.inc
DSP_INCLUDE += -I$(BIOS_DIR) -I$(XDC_DIR) -I$(IPC_DIR)

kernel.dsp_h: dsp.obj

dsp.obj: heapbuf.h
dsp.obj: messageq.h
