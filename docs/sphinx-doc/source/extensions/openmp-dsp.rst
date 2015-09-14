******************************************************
Calling Standard C code with OpenMP from OpenCL C code
******************************************************

Standard C code called from OpenCL C code can contain OpenMP pragmas.
When using this feature the OpenCL C kernel containing the call to an
OpenMP enabled C function must be submitted as a task (not an
NDRangeKernel) and it must be submitted to an in-order OpenCL command
queue (i.e. not defined with the
*CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE* flag). In this scenario,
OpenCL will dispatch the kernel to one compute unit of the DSP
accelerator and the OpenMP runtime will manage distribution of tasks
across the compute units. Please see
:ref:`vecadd_openmp-example` or :ref:`vecadd_openmp_t-example`.

.. toctree::
   :maxdepth: 2

   openmp_dsp_dispatch
