EXE       = mandelbrot
CXXFLAGS = -O3
LIBSDL    = -lSDL

include ../make.inc

$(EXE): main.o 
	@$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(LIBS) $(LIBSDL) -lrt -o $@
