EXE         = ccode
CXX_FLAGS   = -O3
CL6X_FLAGS  = -O3
CLOCL_FLAGS = 

include ../make.inc

$(EXE): main.o ccode.obj
	@$(CXX) $(CXX_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@
