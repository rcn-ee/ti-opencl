EXE         = matmpy
CXXFLAGS   = -O3 -fopenmp -I/usr/include
CL6XFLAGS  = -o3
CLOCLFLAGS = 

include ../make.inc

$(EXE): main.o kernel.out
	@$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(LIBS) -lrt -ldl -lgomp -o $@

kernel.out: ccode.obj 
