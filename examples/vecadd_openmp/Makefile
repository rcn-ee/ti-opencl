EXE        = vecadd_openmp
CPP_FLAGS  = -O3
CL6X_FLAGS = -O1 --omp --use_g3

include ../make.inc

$(EXE): main.o vadd_openmp.obj
	@$(CPP) $(CPP_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@
