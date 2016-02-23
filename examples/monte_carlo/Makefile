EXE          = monte_carlo
CPP_FLAGS    = -O3  -mfpu=neon -ftree-vectorize -funsafe-math-optimizations  

include ../make.inc

OCL_VER := $(shell clocl --version | awk '/ver/ {print gensub(/\./, "", "G", $$6) }')
CL6X_FLAGS   = -O3 -D__TI_OCL_VERSION=0x$(OCL_VER)


cpu_main.o: 	   dsp_kernels.dsp_h initial.h 
dsp_ccode.obj: 	   initial.h
dsp_kernels.dsp_h: dsp_ccode.obj

$(EXE): cpu_main.o dsp_ccode.obj
	@$(CPP) $(CPP_FLAGS) cpu_main.o $(LD_FLAGS) $(LIBS) -o $@

