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

Global variables in C code
++++++++++++++++++++++++++

This section describes the behavior of global variables in C code called from OpenCL C kernels. Global variables are variables declared at the file scope.

* Global variables must be annotated far or the C code must be compiled with the ``--mem_model:data=far --mem_model:const=data`` options. For details, refer the `C6000 Compiler User's Guide`_, section 7.5.5.1. ::

    int far global_var = 0;

* Without any DATA_SECTION pragmas, global variables in C code are placed in external memory (DDR) that is shared across the compute units (C66x DSPs)in an OpenCL device.
* To give each compute unit its own copy of a global variable, there are two options:

   * Use an array and index using the core id register, DNUM.  ::

        #include <c6x.h> // for DNUM
        int far global_var[MAX_COMPUTE_UNITS]; // 2 for AM572, 8 for K2H
        ...

        void foo()
        {
           ... = global_var[DNUM];
        }

   * Place the global variable in memory local to the compute unit using the DATA_SECTION pragma. However,  any explicit initialization of such variables is ignored and implicit zero initialization required by the C standard is not performed. ::

       #pragma DATA_SECTION(global_var, ".mem_l2")
       int global_var;   // Initialization has no effect

.. _C6000 Compiler User's Guide: http://downloads.ti.com/docs/esd/SPRUI04/index.html#SPRUI04A_HTML/tms320c6000_c_c_language_implementation.html
