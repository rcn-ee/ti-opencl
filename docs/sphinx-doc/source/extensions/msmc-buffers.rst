******************************************
Fast Global buffers in on-chip MSMC memory
******************************************
A TI extension for allocating buffers out of MSMC memory has been
added. Other than the location on the DSP where the buffer will
reside, this MSMC defined buffer will act as a standard global buffer
in all other ways. Example:
::

   Buffer bufMsmc(context, CL_MEM_READ_ONLY|CL_MEM_USE_MSMC_TI, size);
   Buffer bufDdr (context, CL_MEM_READ_ONLY, size); }

The :ref:`matmpy-example` illustrates the use of MSMC buffers. 
The :ref:`platforms-example` will query the DSP device and report 
the amount of MSMC memory available for OpenCL use.

.. note::
   MSMC stands for Multicore Shared Memory Controller. It contains on-chip 
   memory shared across all ARM and DSP cores on the 66AK2H. 
   CL_MEM_USE_MSMC_TI is available only on 66AK2H.

