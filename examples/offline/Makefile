EXE         = offline
CXX_FLAGS   = -O3
CLOCL_FLAGS = 

include ../make.inc

$(EXE): main.o vadd.out
	@$(CXX) $(CXX_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@
