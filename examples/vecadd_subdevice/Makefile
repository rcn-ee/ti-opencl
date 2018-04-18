EXE       = vecadd_subdevice
CXXFLAGS = -O3

include ../make.inc

$(EXE): main.o
	@$(CXX) $(CXXFLAGS) $< $(LD_FLAGS) $(LIBS) -o $@
