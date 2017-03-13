******************************************
Setting Timeout Limit on OpenCL Kernels
******************************************

This TI-extended OpenCL implementation supports the ability to set a timeout
limit on a OpenCL kernel.  When the timeout limit has been reached before
the kernel execution finishes on the device, the kernel execution will be
terminated on the device and a negative status will be returned to the the
kernel event.  After that, the device will be ready for running the next
kernel.

Semantics of supported timeouts
===============================

We currently only support timeout locally on a OpenCL compute unit.  That is,
each compute unit has its own clock for timeout and compares against the
specified timeout limit independently.  With regards to multiple workgroups
and multiple compute units, the following rules apply:

#. If a kernel has multiple workgroups dispatched to the same compute
   unit, the clock starts when the first such workgroup starts execution,
   and keeps running during all workgroups.  If the timeout limit is reached
   during the execution of any workgroups, the kernel is terminated.
#. If timeout happens on any compute unit, kernel event status will be set
   to the negative value, ``CL_ERROR_KERNEL_TIMEOUT_TI``.

If a timeout limit is set on a kernel and the timeout happens during the
execution, user can query the corresponding kernel event to query the
timeout error status.  All subsequent kernel events that have the timed out
kernel event in their wait lists will have their execution status set to
``CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST``, per the OpenCL spec for
handling kernel errors.

.. Warning::
  If user set timeout on a kernel but choose not to create a kernel event
  when enqueuing the kernel, the timeout will be silent to the user
  application, if it does happen.

OpenCL extensions and APIs
==========================

OpenCL platform extension: ``cl_ti_kernel_timeout``

OpenCL DSP device extension: ``cl_ti_kernel_timeout_compute_unit``

OpenCL DSP device queue property: ``CL_QUEUE_KERNEL_TIMEOUT_COMPUTE_UNIT_TI``

OpenCL command queue property: ``CL_QUEUE_KERNEL_TIMEOUT_COMPUTE_UNIT_TI``

OpenCL kernel event status: ``CL_ERROR_KERNEL_TIMEOUT_TI``

OpenCL host API:
``cl_int __ti_set_kernel_timeout_ms(cl_kernel d_kernel, cl_uint timeout_in_ms)``

Example of querying, setting and checking timeout
=================================================

The :ref:`timeout-example`  illustrates how the timeout extension works.
The involved steps are:

#. Check device queue property to see if timeout extension is supported

   .. code-block:: cpp

     devices[0].getInfo(CL_DEVICE_QUEUE_PROPERTIES, &devq_prop);
     if ((devq_prop & CL_QUEUE_KERNEL_TIMEOUT_COMPUTE_UNIT_TI) != 0)

#. Create a CommandQueue with timeout property

   .. code-block:: cpp

     new CommandQueue(context, devices[0],
                                    CL_QUEUE_KERNEL_TIMEOUT_COMPUTE_UNIT_TI);

#. Set a timeout limit in milliseconds on the kernel

   .. code-block:: cpp

     __ti_set_kernel_timeout_ms(K(), 100);

#. Create a kernel event when enqueuing the kernel

   .. code-block:: cpp

     ev = kernel_functor(...);
     //or: enqueueNDRangeKernel(kernel, Range, Range, Range, wait_evs, &ev);
     //or: enqueueTask(kernel, wait_evs, &ev);

#. Check the kernel event status to see if timeout happened

   .. code-block:: cpp

     ev.getInfo(CL_EVENT_COMMAND_EXECUTION_STATUS, &status);
     if (status == CL_ERROR_KERNEL_TIMEOUT_TI)
