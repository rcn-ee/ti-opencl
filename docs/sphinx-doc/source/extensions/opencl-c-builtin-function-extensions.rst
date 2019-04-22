****************************************************************
OpenCL C Builtin Function Extensions
****************************************************************

The following built-in functions are available from OpenCL C code on the DSP
device.  They are also available from standard C code that may be called from
OpenCL C code. If using these functions in your standard C code, please add
``#include <dsp_c.h>`` to your source.  This header defines the function
prototypes.

.. c:function::  uint32_t __core_num(void)

    Returns the core ID of the DSP issuing the call. This will be a value in
    the range [0..N-1] where N is the number of C66 DSP cores in the device.

.. c:function::  uint64_t __clock64(void)

    Returns a 64-bit unsigned time-stamp representing the cycle count counter
    in the DSP. Calling this function twice and subtracting the results is a
    valid mechanism for determining the number of elapsed cycles between two
    points in your DSP OpenCL C or Standard C code. This function equates to
    the TSCH:TSCL register pair on the C66 DSP.

.. c:function::   uint32_t __clock(void)

    Returns a 32-bit unsigned time-stamp representing the cycle count counter
    in the DSP. Calling this function twice and subtracting the results is a
    valid mechanism for determining the number of elapsed cycles between two
    points in your DSP OpenCL C or Standard C code. This function equates to
    the TSCL register pair on the C66 DSP.

.. c:function::  void __cycle_delay(uint64_t cyclesToDelay)

    Given a number of cycles to delay, this function will busy
    loop for approximately that many cycles before returning.

.. c:function::  void __mfence(void)

    Creates a memory fence for the C66x DSP.  This function is equivalent to
    the OpenCL C builtin function mem_fence(), but this version can also be
    called from standard C code called from OpenCL C code.

.. c:function::  uint dot(uchar4 a, uchar4 b)

    Compute the dot product of two uchar4 vectors and return the result as a uint.
    The C66x DSP can support this as a single instruction.

.. c:function::  int  dot(char4  a, uchar4 b)

    Compute the dot product of a char4 vector and a uchar4 vector and return the
    result as an int.  The C66x DSP can support this as a single instruction.

.. c:function::  int  dot(short2 a, short2 b)

    Compute the dot product of two short2 vectors and return the result as an int.
    The C66x DSP can support this as a single instruction.

.. c:function::  uint32_t __dsp_frequency(void)

    Returns the clock frequency (Mhz) the DSP cores are running at.

.. Important::
   In OpenCL C for C66 a uint32_t is an unsigned int and a uint64_t is an unsigned long.
   In standard C for C66 a uint32_t is an unsigned int and a uint64_t is an unsigned long long.
