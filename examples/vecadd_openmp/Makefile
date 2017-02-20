EXE        = vecadd_openmp
CXX_FLAGS  = -O3
CL6X_FLAGS = -O3 --omp

include ../make.inc

$(EXE): main.o vadd_openmp.obj
	@$(CXX) $(CXX_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@
