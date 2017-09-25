EXE       = mandelbrot_native
CXXFLAGS = -O3 -fopenmp
LIBSDL    = -lSDL

include ../make.inc 

$(EXE): main.o mandelbrot.o
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LIBS) $(LIBSDL) -lgomp -lrt -o $@
