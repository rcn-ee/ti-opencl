EXE         = dspheap
CXXFLAGS   = -O3
CLOCLFLAGS =

include ../make.inc

$(EXE): main.o
	@$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(LIBS) -o $@

main.o: kernels.dsp_h
