EXE       = vecadd
EXE_MD    = vecadd_md
CXXFLAGS = -O3

include ../make.inc

$(EXE): main.o  $(EXE_MD)
	@$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(LIBS) -o $@

$(EXE_MD): main_md.o 
	@$(CXX) $(CXXFLAGS) main_md.o $(LDFLAGS) $(LIBS) -lrt -o $@

clean::
	@rm -fr $(EXE_MD)
