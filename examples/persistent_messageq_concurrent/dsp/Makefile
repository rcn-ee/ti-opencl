CL6XFLAGS  = -o2 -I../../persistent_common --mem_model:data=far --mem_model:const=data
CLOCLLAGS  = 
USE_RTOS   = 1

include ../../make.inc

kernel.dsp_h: dsp.obj

dsp.obj: heapbuf.h
dsp.obj: messageq.h
