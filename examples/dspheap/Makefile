EXE         = dspheap
CXX_FLAGS   = -O3
CLOCL_FLAGS =

include ../make.inc

$(EXE): main.o
	@$(CXX) $(CXX_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@

main.o: kernels.dsp_h
