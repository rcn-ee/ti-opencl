EXE       = abort_exit
CXXFLAGS = -O3

include ../make.inc

$(EXE): main.o
	@$(CXX) $(CXXFLAGS) $< $(LDFLAGS) $(LIBS) -o $@

main.o: kernel.dsp_h
