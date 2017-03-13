EXE       = mandelbrot_native
CXX_FLAGS = -O3 -fopenmp
LIBSDL    = -lSDL

include ../make.inc 

$(EXE): main.o mandelbrot.o
	@$(CXX) $(CXX_FLAGS) $^ $(LD_FLAGS) $(LIBS) $(LIBSDL) -lgomp -lrt -o $@
