EXE       = ooo_map
CXX_FLAGS = -g -O0 -fno-inline -std=c++0x

include ../make.inc

$(EXE): ooo.o 
	@$(CXX) $(CXX_FLAGS) $< $(LD_FLAGS) $(LIBS) -lrt -o $@
