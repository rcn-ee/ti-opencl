EXE        = persistent_clock_concurrent
CXXFLAGS   = -O3 -I../persistent_common
CL6XFLAGS  = -o2 -I../persistent_common --mem_model:data=far --mem_model:const=data
CLOCLFLAGS = 
USE_RTOS   = 1


VPATH = ../persistent_common

include ../make.inc

$(EXE): host.o host_assist.o
	@echo Linking $@
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LIBS) -o $@

host.o: kernel.dsp_h

kernel.dsp_h: dsp.obj
