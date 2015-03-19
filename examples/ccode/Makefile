EXE         = ccode
CPP_FLAGS   = -g
CL6X_FLAGS  = -g
CLOCL_FLAGS = 

include ../make.inc

$(EXE): main.o ccode.obj
	@$(CPP) $(CPP_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@
