ATLAS_INSTALL=libcblas_atlas_arm
ATLAS_LIB    = $(ATLAS_INSTALL)/libcblas.a $(ATLAS_INSTALL)/libatlas.a

EXE         = sgemm
CPP_FLAGS   = -O3 -I/usr/include $(ATLAS_INC)
CL6X_FLAGS  = -o3 
CLOCL_FLAGS =

include ../make.inc

LIBS += $(ATLAS_LIB)

main.o: kernel.dsp_h

$(EXE): main.o
	$(CPP) $(CPP_FLAGS) main.o $(LD_FLAGS) $(LIBS) -lrt -lpthread -lm -o $@

kernel.dsp_h: kernel.cl sgemm.obj sgemm_kernel.obj data_move.obj
	$(CLOCL) $(CLOCL_FLAGS) --txt $^
