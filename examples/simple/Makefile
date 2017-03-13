EXE       = simple
CXX_FLAGS = -O3

include ../make.inc

$(EXE): simple.o 
	@$(CXX) $(CXX_FLAGS) $< $(LD_FLAGS) $(LIBS) -o $@
