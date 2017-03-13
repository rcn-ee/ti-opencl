EXE          = monte_carlo
CXX_FLAGS    = -O3  -mfpu=neon -ftree-vectorize -funsafe-math-optimizations  

include ../make.inc

CL6X_FLAGS   = -O3


cpu_main.o: 	   dsp_kernels.dsp_h initial.h 
dsp_ccode.obj: 	   initial.h
dsp_kernels.dsp_h: dsp_ccode.obj

$(EXE): cpu_main.o dsp_ccode.obj
	@$(CXX) $(CXX_FLAGS) cpu_main.o $(LD_FLAGS) $(LIBS) -o $@

