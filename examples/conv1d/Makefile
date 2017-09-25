EXE       = conv1d
CXXFLAGS = -O3
CL6XFLAGS = -o3 -mw --symdebug:none

include ../make.inc

$(EXE): main.o
	@$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(LIBS) -lrt -o $@

main.o: ti_kernels.dsp_h

ti_kernels.dsp_h: k_extc.obj

