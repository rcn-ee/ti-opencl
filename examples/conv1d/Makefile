EXE       = conv1d
CXX_FLAGS = -O3
CL6X_FLAGS = -o3 -mw --symdebug:none

include ../make.inc

$(EXE): main.o
	@$(CXX) $(CXX_FLAGS) main.o $(LD_FLAGS) $(LIBS) -lrt -o $@

main.o: ti_kernels.dsp_h

ti_kernels.dsp_h: k_extc.obj

