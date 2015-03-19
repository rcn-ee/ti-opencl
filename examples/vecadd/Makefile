EXE       = vecadd
EXE_MD    = vecadd_md
CPP_FLAGS = -O3

include ../make.inc

$(EXE): main.o  $(EXE_MD)
	@$(CPP) $(CPP_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@

$(EXE_MD): main_md.o 
	@$(CPP) $(CPP_FLAGS) main_md.o $(LD_FLAGS) $(LIBS) -lrt -o $@

clean::
	@rm -fr $(EXE_MD)
