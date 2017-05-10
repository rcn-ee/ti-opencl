EXE         = float_compute
CXXFLAGS   = -O3 -ftree-vectorize -fopenmp -march=armv7-a -mfpu=neon -mfloat-abi=hard
CLOCLFLAGS =

include ../make.inc

$(EXE): main.o dsp_compute.out
	@$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(LIBS) -o $@ -lrt -ldl
