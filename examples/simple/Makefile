EXE       = simple
CXXFLAGS = -O3

include ../make.inc

$(EXE): simple.o 
	@$(CXX) $(CXXFLAGS) $< $(LDFLAGS) $(LIBS) -o $@
