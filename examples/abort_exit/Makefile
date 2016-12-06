EXE       = abort_exit
CPP_FLAGS = -O3

include ../make.inc

$(EXE): main.o
	@$(CPP) $(CPP_FLAGS) $< $(LD_FLAGS) $(LIBS) -o $@

