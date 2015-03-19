EXE        = edmamgr
CPP_FLAGS  = -O3 
include ../make.inc

$(EXE): main.o 
	@$(CPP) $(CPP_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@

main.o: kernel.dsp_h
