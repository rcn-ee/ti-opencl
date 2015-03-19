EXE       = mandelbrot
CPP_FLAGS = -O3
LIBSDL    = -lSDL

include ../make.inc

$(EXE): main.o 
	@$(CPP) $(CPP_FLAGS) main.o $(LD_FLAGS) $(LIBS) $(LIBSDL) -lrt -o $@
