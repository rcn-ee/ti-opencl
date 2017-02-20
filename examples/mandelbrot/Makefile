EXE       = mandelbrot
CXX_FLAGS = -O3
LIBSDL    = -lSDL

include ../make.inc

$(EXE): main.o 
	@$(CXX) $(CXX_FLAGS) main.o $(LD_FLAGS) $(LIBS) $(LIBSDL) -lrt -o $@
