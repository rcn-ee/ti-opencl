EXE        = dsplib_fft
CXXFLAGS   = -O3

include ../make.inc

DSPLIB_LIB ?= dsplib_3_4_0_0.ae66

$(EXE): fft_ocl.o
	@$(CXX) $(CXXFLAGS) $< $(LDFLAGS) $(LIBS) -o $@

fft_ocl.o: kernel.dsp_h

kernel.dsp_h: kernel.cl $(DSPLIB_LIB)

