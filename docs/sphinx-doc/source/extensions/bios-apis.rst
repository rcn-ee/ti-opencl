******************************************
Calling TI BIOS APIs from OpenCL C kernels
******************************************

The TI extension to OpenCL to allow general standard C functions to be called
from OpenCL C kernels can be used to dispatch code on the DSP's that make use
of TI's BIOS APIs.  

.. Warning::
   Usage of BIOS APIs from OpenCL is considered an
   advanced technique and should only be used if the application creator is
   knowledgeable of   the BIOS APIs, the BIOS scheduler and the OpenCL
   execution model.  However, with suitable care, this extension expands the use
   cases under which OpenCL is an applicable framework for 
   heterogeneous multicore applications on TI System on Chip (SOC) platforms.  

   This feature is available only on AM572x devices.


Use cases
---------
* Ability to set a timeout for computations dispatched from OpenCL

* Ability to register a task or clock function to persist on the DSP outside of
  a normal OpenCL kernel dispatch

* Ability to start and stop a DSP computation with independent OpenCL kernel
  dispatches

* Ability to create additional BIOS tasks (threads) on the DSP and
  coordinate among them with semaphores

* Ability for in-flight DSP code dispatched through OpenCL to communicate with a
  host thread, without a new OpenCL C kernel to start or finish.


Need to know
------------
* The extension to allow BIOS API calls is currently only supported on TI's
  AM57x family of devices.

* Printf capability from DSP code is only supported from OpenCL C kernels or
  code in the direct call tree from the kernel.  Conversely, printf is not
  supported in DSP code in any user created BIOS Tasks or clock handlers. 

* The OpenCL run-time reserves the BIOS Task priority levels 6-10 for the
  internals of the OpenCL run-time and any OpenCL C kernel code.  Task priority
  levels 0-5 can be used for user created Tasks at lower priority than OpenCL
  and Task priority levels 11-15 can be used for user created Tasks at higher
  priority than OpenCL.

* All OpenCL C kernels dispatched to the DSP run as part of a BIOS Task.
  BIOS is not a time-sharing RTOS.  If a higher priority Task is ready to
  run, it will, until it becomes blocked, at which time the next highest level
  Task ready to run will be scheduled.  If a user created Task is given a
  higher priority than the OpenCL run-time uses for dispatched code, then it 
  must either complete or periodically block.  If it does not, then the
  OpenCL run-time Tasks will never be scheduled again and the system could
  deadlock.

* All BIOS APIs that take a time unit as an argument or return a time unit
  as a value are in terms of BIOS clock ticks, which are configured to occur 
  every 1 millisecond, so a call to Task_sleep(1000) would effectively be a 
  sleep for 1 second.

* Counting the DSP clock ticks around a Task_sleep() call may not give an
  expected value.  This can occur if the DSP has no other Task to execute, in
  which case, it will idle and the CPU clock tick register will not advance. For 
  example, with the DSP running at a frequency of 750Mhz and the following code,
  it could be expected that the variable *elapsed* contained approximately 
  750,000,000.  However, in reality the value of *elapsed* may be far smaller.

.. code-block:: cpp

      unsigned t0      = __clock();  // value of DSP's TSCL register
      Task_sleep(1000);              // equivalent of 1 second
      unsigned t1      = __clock();  // value of DSP's TSCL register
      unsigned elapsed = t1-t0;      // cycles elapsed between __clock() calls



* It is recommended that you do not call BIOS APIs directly from OpenCL C
  code, but instead call BIOS API's from standard C code, that is called
  from OpenCL C code.  This is recommended due to the differing
  interpretation of the long and ulong data type.  OpenCL C defines these types
  as 64 bits, but the standard C compiler for the DSP interprets these types as
  32 bits.  The BIOS APIs use data types that are defined by BIOS and
  these types correspond with the standard C compiler interpretation and not
  the OpenCL C interpretation.

* APIs from the following BIOS and IPC packages are supported:

    * ti.sdo.ipc.MessageQ
    * ti.sysbios.family.c66.Cache
    * ti.sysbios.heaps.HeapBuf
    * ti.sysbios.heaps.HeapMem
    * ti.sysbios.knl.Clock
    * ti.sysbios.knl.Idle
    * ti.sysbios.knl.Intrinsics
    * ti.sysbios.knl.Queue
    * ti.sysbios.knl.Semaphore
    * ti.sysbios.knl.Swi
    * ti.sysbios.knl.Task

Examples
--------
The following examples included in the OpenCL package illustrate using BIOS APIs within C code called from OpenCL C kernels:

* persistent_clock_concurrent
* persistent_clock_spanning
* persistent_common
* persistent_kernel_timeout
* persistent_messageq_concurrent
* persistent_task_concurrent
* persistent_task_spanning

The examples are located in ``/usr/share/ti/examples/opencl``.

References
----------
* `TI BIOS User's Guide <http://www.ti.com/lit/ug/spruex3q/spruex3q.pdf>`_
* `TI IPC User's Guide <http://processors.wiki.ti.com/index.php/IPC_Users_Guide>`_

