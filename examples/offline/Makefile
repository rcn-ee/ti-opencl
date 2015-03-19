EXE         = offline
CPP_FLAGS   = -O3
CLOCL_FLAGS = 

include ../make.inc

$(EXE): main.o vadd.out
	@$(CPP) $(CPP_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@
