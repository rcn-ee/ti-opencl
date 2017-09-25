EXE         = ccode
CXXFLAGS   = -O3
CL6XFLAGS  = -O3
CLOCLFLAGS = 

include ../make.inc

$(EXE): main.o ccode.obj
	@$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(LIBS) -o $@
