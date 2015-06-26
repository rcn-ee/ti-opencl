*****************************
Extensions
*****************************

TI's OpenCL implementation has been extended with a set of features beyond the OpenCL 1.1 specification. These features were added in order to better support the execution of code on the C66 DSP, to enable existing DSP libraries, and to better map to TI's devices.  

#. This OpenCL implementation supports the ability to call standard C
   code from OpenCL C kernels. This includes calling functions from
   existing C66 DSP libraries, such as the dsplib or mathlib. For
   examples of this capability please refer to the :ref:`ccode-example`
   for calling a C function you define, or the :ref:`dsplib_fft-example`
   for calling a function in a library.

#. Additionally the called standard C code can contain OpenMP pragmas.
   When using this feature the OpenCL C kernel containing the call to an
   OpenMP enabled C function must be submitted as a task (not an
   NDRangeKernel) and it must be submitted to an in-order OpenCL command
   queue (i.e not defined with the
   *CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE* flag). In this scenario,
   OpenCL will dispatch the kernel to one compute unit of the DSP
   accelerator and the OpenMP runtime will manage distribution of tasks
   across the compute units. Please see
   :ref:`vecadd_openmp-example` and :ref:`vecadd_openmp_t-example`.

#. A printf capability has been added for kernels running on the DSP.
   OpenCL C kernels or standard C code called from an OpenCL C kernel
   can call printf. The string resulting from the printf will be
   transmitted to the host ARM and displayed using a printf on the ARM
   side. This feature can be used to assist in debug of your OpenCL
   kernels. Note that there is a performance penalty in using printf
   from the DSPs, so it is not a feature that should be used when
   evaluating DSP performance. This feature is not the OpenCL 1.2 printf
   which contains additional formatting codes for printing vector types.

#. A TI extension for allocating buffers out of MSMC memory has been
   added. Other than the location on the DSP where the buffer will
   reside, this MSMC defined buffer will act as a standard global buffer
   in all other ways. Example:
   ::

       Buffer bufMsmc(context, CL_MEM_READ_ONLY|CL_MEM_USE_MSMC_TI, size);
       Buffer bufDdr (context, CL_MEM_READ_ONLY, size); }

   The :ref:`matmpy-example` illustrates the use of MSMC buffers. 
   The :ref:`platforms-example` will query the DSP device and report 
   the amount of MSMC memory available for OpenCL use.

#. The OpenCL C compiler for the C66 DSP supports the C66x standard C
   compiler set of intrinsic functions, with the exception of those
   intrinsics that accept or result in a 40 bit value. Please refer to
   the C6000 Compiler User's Guide for a list of these intrinsic
   functions.

#. This OpenCL implementation also supports direct access to the EDMA
   system from the DSP and OpenCL C kernels. A wide range of EDMA
   constructs are supported. These include 1D to 1D, 1D to 2D, 2D to 1D,
   and chained transfers. Refer to :doc:`memory/edmamgr` and the 
   :ref:`edmamgr-example` for more information.

#. An extended memory feature is supported on the 66AK2H implementation
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
   access. The :ref:`vecadd_mpax-example`  illustrates a
   process of defining a large buffer and then defining sub-buffers
   within the larger buffer and dispatching multiple OpenCL kernels to
   the DSP on these sub-buffers, cumulatively resulting in the entire
   large buffer being processed. Also see :doc:`memory/extended-memory`

#. :doc:`opencl-c-builtin-function-extensions`

#. :doc:`memory/host-malloc-extension`

#. :doc:`memory/dsp-malloc-extension`


.. note::
   MSMC stands for Multicore Shared Memory Controller. It contains on-chip 
   memory shared across all ARM and DSP cores on the 66AK2H. 
   CL_MEM_USE_MSMC_TI is available only on 66AK2H.

