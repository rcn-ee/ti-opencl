EXE       = conv1d
CPP_FLAGS = -O3
CL6X_FLAGS = -o3 -mw --symdebug:none

include ../make.inc

$(EXE): main.o
	@$(CPP) $(CPP_FLAGS) main.o $(LD_FLAGS) $(LIBS) -lrt -o $@

main.o: ti_kernels.dsp_h

ti_kernels.dsp_h: k_extc.obj

