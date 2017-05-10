CXXFLAGS  = -O3 -Wall -Werror -fpic 
CL6XFLAGS = -o3 --omp -mv6600 --abi=eabi -I$(TARGET_ROOTDIR)/usr/share/ti/opencl -I$(TI_OCL_CGT_INSTALL)/include -I$(TARGET_ROOTDIR)/usr/share/ti/cgt-c6x/include

all: libcblas_dgemm_dsp.a

include ../../make.inc

libcblas_dgemm_dsp.a: cblas_dgemm.o init.o
	ar r $@ $^

init.o : init.cpp kernel.dsp_h

kernel.dsp_h: kernel.cl dgemm.obj dgemm_kernel.obj data_move.obj dgemm_omp.obj
	$(CLOCL) --txt $^

%.obj: %.c
	$(CL6X) -c $(CL6XFLAGS) $<

clean::
	rm -f *.o*  *.dsp_h *.a
