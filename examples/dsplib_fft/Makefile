EXE       = dsplib_fft
CXXFLAGS = -O3

include ../make.inc

$(EXE): fft_ocl.o 
	@$(CXX) $(CXXFLAGS) $< $(LDFLAGS) $(LIBS) -o $@
