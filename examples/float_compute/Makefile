EXE         = float_compute
CXX_FLAGS   = -O3 -ftree-vectorize -fopenmp -march=armv7-a -mfpu=neon -mfloat-abi=hard
CLOCL_FLAGS =

include ../make.inc

$(EXE): main.o dsp_compute.out
	@$(CXX) $(CXX_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@ -lrt -ldl
