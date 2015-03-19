EXE       = ooo_callback
CPP_FLAGS = -g -O0 -fno-inline -std=c++0x

include ../make.inc

$(EXE): ooo_callback.o
	@$(CPP) $(CPP_FLAGS) $< $(LD_FLAGS) $(LIBS) -lrt -o $@

