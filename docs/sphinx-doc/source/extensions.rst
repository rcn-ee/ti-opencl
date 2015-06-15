TI Specific OpenCL extensions
*****************************

TI's OpenCL implementation has been extended with a set of features beyond the OpenCL 1.1 specification. These features were added in order to better support the execution of code on the C66 DSP, to enable existing DSP libraries, and to better map to TI's devices.  

1. This OpenCL implementation supports the ability to call standard C
   code from OpenCL C kernels. This includes calling functions from
   existing C66 DSP libraries, such as the dsplib or mathlib. For
   examples of this capability please refer to the `ccode
   example <OpenCL_Examples#ccode_example>`__ for calling a C function
   you define, or the `dsplib\_fft
   example <OpenCL_Examples#dsplib_fft_example>`__ for calling a
   function in a library.

2. Additionally the called standard C code can contain OpenMP pragmas.
   When using this feature the OpenCL C kernel containing the call to an
   OpenMP enabled C function must be submitted as a task (not an
   NDRangeKernel) and it must be submitted to an in-order OpenCL command
   queue (i.e not defined with the
   ``CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE`` flag). In this scenario,
   OpenCL will dispatch the kernel to one compute unit of the DSP
   accelerator and the OpenMP runtime will manage distribution of tasks
   across the compute units. Please see examples
   `vecadd\_openmp <OpenCL_Examples#vecadd_openmp_example>`__,
   `vecadd\_openmp\_t <OpenCL_Examples#vecadd_openmp_t_example>`__ or
   `openmpbench\_C\_v3 <OpenCL_Examples#openmpbench_C_v3_example>`__.

3. A printf capability has been added for kernels running on the DSP.
   OpenCL C kernels or standard C code called from an OpenCL C kernel
   can call printf. The string resulting from the printf will be
   transmitted to the host ARM and displayed using a printf on the ARM
   side. This feature can be used to assist in debug of your OpenCL
   kernels. Note that there is a performance penalty in using printf
   from the DSPs, so it is not a feature that should be used when
   evaluating DSP performance. This feature is not the OpenCL 1.2 printf
   which contains additional formatting codes for printing vector types.

4. A TI extension for allocating buffers out of MSMC memory has been
   added. Other than the location on the DSP where the buffer will
   reside, this MSMC defined buffer will act as a standard global buffer
   in all other ways. Example:
   ::

       Buffer bufMsmc(context, CL_MEM_READ_ONLY|CL_MEM_USE_MSMC_TI, size);
       Buffer bufDdr (context, CL_MEM_READ_ONLY, size); }

   The `matmpy example <OpenCL_Examples#matmpy_example>`__ illustrates
   the use of MSMC buffers. The `platform
   example <OpenCL_Examples#platform_example>`__ will query the DSP
   device and report the amount of MSMC memory available for OpenCL use.

.. note::
   MSMC stands for Multicore Shared Memory Controller. It contains on-chip 
   memory shared across all ARM and DSP cores on the 66AK2H. 
   CL_MEM_USE_MSMC_TI is available only on 66AK2H.

5. The OpenCL C compiler for the C66 DSP supports the C66x standard C
   compiler set of intrinsic functions, with the exception of those
   intrinsics that accept or result in a 40 bit value. Please refer to
   the C6000 Compiler User's Guide for a list of these intrinsic
   functions.

6. Additionally these non standard OpenCL C built-in functions are
   supported:
   ::

       uint32_t __core_num     (void);
       uint32_t __clock        (void);
       uint64_t __clock64      (void);
       void     __cycle_delay  (uint64_t cyclesToDelay);
       void     __mfence       (void);

   ``__core_num`` returns [0-7] depending on which DSP core executes the
   function.

   ``__clock`` return a 32-bit time-stamp value, subtracting two values
   returned by ``__clock`` gives the number of elapsed DSP cycles
   between the two. This equates to the C66 device's TSCL register.

   ``__clock64`` return a 64-bit time-stamp value similar to ``__clock``
   but with more granularity to avoid potential overflow of a 32 bit
   counter. This equates to the C66 device's TSCH:TSCL register pair.

   ``__cycle_delay`` takes a specified number of cycles to delay and
   will busy loop for that many cycles (approximately) before returning.

   ``__mfence`` is a memory fence for the C66x dsp. Under typical OpenCL
   use, this will not be needed. However, when incorporating EDMA usage
   into OpenCL C kernels, it may be needed.

.. note::
   In standard C for C66 a uint32\_t is an unsigned int and a
   uint64\_t is an unsigned long long. In OpenCL C for C66 a uint32\_t
   is an unsigned int and a uint64\_t is an unsigned long.


7. This OpenCL implementation also supports direct access to the EDMA
   system from the DSP and OpenCL C kernels. A wide range of EDMA
   constructs are supported. These include 1D to 1D, 1D to 2D, 2D to 1D,
   and chained transfers. The `edmamgr
   example <OpenCL_Examples#edmamgr_example>`__ illustrates how this
   feature is used.

8. An extended memory feature is supported on the 66AK2H implementation
   of OpenCL. The C66 DSP is a 32-bit architecture and has a limit of
   2GB of DDR that it can access at any given time. The 66AK2H platforms
   can support up to 8GB of DDR3. To enable usage of DDRs greater than
   2GB, this OpenCL implementation can use a hardware mapping feature to
   move windows over the 8GB DDR into the 32-bit DSP address space.
   Movement of these windows will occur at kernel start boundaries so
   two sequential kernels dispatched to the DSP device may actually
   operate on different 2GB areas within the 8GB DDR. The windows are
   not moved within a kernel. As a result of this feature, large buffers
   may be created and subsequently populated on the ARM side. However, a
   dispatched kernel may not access the entire buffer in one dispatch.
   Any given OpenCL Kernel will be limited to a total of 2GB of DDR
   access. The `vecadd\_mpax
   example <OpenCL_Examples#vecadd_mpax_example>`__ illustrates a
   process of defining a large buffer and then defining sub-buffers
   within the larger buffer and dispatching multiple OpenCL kernels to
   the DSP on these sub-buffers, cumulatively resulting in the entire
   large buffer being processed.


