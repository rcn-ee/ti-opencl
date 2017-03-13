EXE       = dsplib_fft
CXX_FLAGS = -O3

include ../make.inc

$(EXE): fft_ocl.o 
	@$(CXX) $(CXX_FLAGS) $< $(LD_FLAGS) $(LIBS) -o $@
