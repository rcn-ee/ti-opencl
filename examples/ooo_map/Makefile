EXE       = ooo_map
CPP_FLAGS = -g -O0 -fno-inline -std=c++0x

include ../make.inc

$(EXE): ooo.o 
	@$(CPP) $(CPP_FLAGS) $< $(LD_FLAGS) $(LIBS) -lrt -o $@
