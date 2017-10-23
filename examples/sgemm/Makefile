ATLAS_INSTALL=libcblas_atlas_arm
ATLAS_LIB    = $(ATLAS_INSTALL)/libcblas.a $(ATLAS_INSTALL)/libatlas.a

EXE         = sgemm
CXXFLAGS   = -O3 $(ATLAS_INC)
CL6XFLAGS  = -o3 
CLOCLFLAGS =

include ../make.inc

LIBS += $(ATLAS_LIB)

main.o: kernel.dsp_h

$(EXE): main.o
	$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(LIBS) -lrt -lpthread -lm -o $@

kernel.dsp_h: kernel.cl sgemm.obj sgemm_kernel.obj data_move.obj
	$(CLOCL) $(CLOCLFLAGS) --txt $^
