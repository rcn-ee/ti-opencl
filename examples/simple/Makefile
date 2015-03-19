EXE       = simple
CPP_FLAGS = -O3

include ../make.inc

$(EXE): simple.o 
	@$(CPP) $(CPP_FLAGS) $< $(LD_FLAGS) $(LIBS) -o $@
