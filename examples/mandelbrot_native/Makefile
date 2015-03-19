EXE       = mandelbrot_native
CPP_FLAGS = -O3 -fopenmp
LIBSDL    = -lSDL

include ../make.inc 

$(EXE): main.o mandelbrot.o
	@$(CPP) $(CPP_FLAGS) $^ $(LD_FLAGS) $(LIBS) $(LIBSDL) -lgomp -lrt -o $@
