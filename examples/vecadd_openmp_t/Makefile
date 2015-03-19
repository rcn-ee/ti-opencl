EXE        = vecadd_openmp_t
CPP_FLAGS  = -O3
CL6X_FLAGS = --omp --use_g3 -O1 

include ../make.inc

$(EXE): main.o vadd_openmp.obj
	@$(CPP) $(CPP_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@

