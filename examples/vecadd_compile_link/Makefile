EXE      = vecadd_compile_link
CXXFLAGS = -O3

include ../make.inc

$(EXE): main.o
	@$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(LIBS) -o $@

clean::
	@rm -fr $(EXE)
