******************************************
Calling Standard C Code From OpenCL C Code
******************************************

This OpenCL implementation supports the ability to call standard C
code from OpenCL C code. This includes calling functions in
existing C66 DSP libraries, such as the dsplib, mathlib or imglib. For
examples of this capability please refer to the :ref:`ccode-example`
for calling a C function you define, or the :ref:`dsplib_fft-example`
for calling a function in a library.

The standard C code may also contain OpenMP pragmas to control parallel
execution. Refer to :doc:`openmp-dsp` for a description of this use case.
