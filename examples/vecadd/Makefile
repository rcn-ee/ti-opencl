EXE       = vecadd
EXE_MD    = vecadd_md
CXX_FLAGS = -O3

include ../make.inc

$(EXE): main.o  $(EXE_MD)
	@$(CXX) $(CXX_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@

$(EXE_MD): main_md.o 
	@$(CXX) $(CXX_FLAGS) main_md.o $(LD_FLAGS) $(LIBS) -lrt -o $@

clean::
	@rm -fr $(EXE_MD)
