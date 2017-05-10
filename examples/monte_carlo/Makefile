EXE          = monte_carlo
CXXFLAGS    = -O3  -mfpu=neon -ftree-vectorize -funsafe-math-optimizations  
CL6XFLAGS   = -O3

include ../make.inc

cpu_main.o: 	   dsp_kernels.dsp_h initial.h 
dsp_ccode.obj: 	   initial.h
dsp_kernels.dsp_h: dsp_ccode.obj

$(EXE): cpu_main.o dsp_ccode.obj
	@$(CXX) $(CXXFLAGS) cpu_main.o $(LDFLAGS) $(LIBS) -o $@

