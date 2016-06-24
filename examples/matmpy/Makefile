EXE         = matmpy
CPP_FLAGS   = -O3 -fopenmp -I/usr/include
CL6X_FLAGS  = -o3
CLOCL_FLAGS = 

include ../make.inc

$(EXE): main.o kernel.out
	@$(CPP) $(CPP_FLAGS) main.o $(LD_FLAGS) $(LIBS) -lrt -ldl -lgomp -o $@

kernel.out: ccode.obj 
