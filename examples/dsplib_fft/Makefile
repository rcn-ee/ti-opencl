EXE       = dsplib_fft
CPP_FLAGS = -O3

include ../make.inc

$(EXE): fft_ocl.o 
	@$(CPP) $(CPP_FLAGS) $< $(LD_FLAGS) $(LIBS) -o $@
