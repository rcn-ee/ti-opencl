********************************************
C66 standard C compiler intrinsic functions
********************************************

The OpenCL C compiler for the C66 DSP supports the C66x standard C
compiler set of intrinsic functions, with the exception of those
intrinsics that accept or result in a 40 bit value or a long long type. 
Please refer to the subsection 7.5.5. Using Intrinsics to Access Assembly
Language Statements in the `C6000 Compiler User's Guide`_ for a list of these
intrinsic functions.

.. Important:: 

    In OpenCL C code the long type is 64 bits wide, and a long long type does
    not exist. 
    
    In standard C66 C code the long type is 32 bits wide and the long long type
    is 64 bits wide.

.. _C6000 Compiler User's Guide: http://www.ti.com/lit/ug/spru187u/spru187u.pdf
