EXE       = null
CXX_FLAGS = -O3

include ../make.inc

$(EXE): main.o 
	@$(CXX) $(CXX_FLAGS) main.o $(LD_FLAGS) $(LIBS) -lrt -o $@
