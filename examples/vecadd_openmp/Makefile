EXE        = vecadd_openmp
CXXFLAGS  = -O3
CL6XFLAGS = -O3 --omp

include ../make.inc

$(EXE): main.o vadd_openmp.obj
	@$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(LIBS) -o $@
