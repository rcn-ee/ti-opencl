******************************
OpenCL C code using printf
******************************

A printf capability has been added for kernels running on the DSP.
OpenCL C kernels or standard C code called from an OpenCL C kernel
can call printf. The string resulting from the printf will be
transmitted to the host ARM and displayed using a printf on the ARM
side. This feature can be used to assist in debug of your OpenCL
kernels. Note that there is a performance penalty in using printf
from the DSPs, so it is not a feature that should be used when
evaluating DSP performance. This feature is not the OpenCL 1.2 printf,
which contains additional formatting codes for printing vector types.

