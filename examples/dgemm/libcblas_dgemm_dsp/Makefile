CPPFLAGS   = -O3 -Wall -Werror -fpic 
CL6X_FLAGS = -o3 --omp -mv6600 --abi=eabi -I$(TARGET_ROOTDIR)/usr/share/ti/opencl -I$(TI_OCL_CGT_INSTALL)/include -I$(TARGET_ROOTDIR)/usr/share/ti/cgt-c6x/include

include ../../make.inc


all: libcblas_dgemm_dsp.a
cross: all

libcblas_dgemm_dsp.a: cblas_dgemm.o init.o
	ar r $@ $^

init.o : init.cpp kernel.dsp_h

kernel.dsp_h: kernel.cl dgemm.obj dgemm_kernel.obj data_move.obj dgemm_omp.obj
	clocl --txt $^

%.obj: %.c
	cl6x -c $(CL6X_FLAGS) $<

clean::
	rm -f *.o*  *.dsp_h *.a
