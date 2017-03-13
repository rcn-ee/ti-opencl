EXE       = ooo_callback
CXX_FLAGS = -g -O0 -fno-inline -std=c++0x

include ../make.inc

$(EXE): ooo_callback.o
	@$(CXX) $(CXX_FLAGS) $< $(LD_FLAGS) $(LIBS) -lrt -o $@

