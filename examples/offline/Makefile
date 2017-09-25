EXE         = offline
CXXFLAGS   = -O3
CLOCLFLAGS = 

include ../make.inc

$(EXE): main.o vadd.out
	@$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(LIBS) -o $@
