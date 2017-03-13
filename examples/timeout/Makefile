EXE       = timeout
CXX_FLAGS = -O3

include ../make.inc

$(EXE): main.o
	@$(CXX) $(CXX_FLAGS) $< $(LD_FLAGS) $(LIBS) -o $@

main.o: kernel.dsp_h
