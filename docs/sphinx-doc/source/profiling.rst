********************************
Profiling 
********************************

You can profile OpenCL application as any other application with generic
profiling tools such as "gprof".  Here we explain how to profile commands
in the OpenCL command queue.

Host Side Profiling
=======================================================

To profile commands on the host side, you will need to specify
"CL_QUEUE_PROFILING_ENABLE" property when creating the command queue.
OpenCL runtime will then record the host-side timestamp in nano-seconds
when the command is enqueued, is submitted, starts execution and finishes
execution.  User code can query these timestamps using
"clGetEventProfilingInfo" API on the corresponding event that is returned
at command enqueue time.

DSP Side Profiling
=======================================================

OpenCL runtime also records timestamps for predefined OpenCL activities
on the DSP side and uses lightweight ULM (Usage and Load Monitor) to
communicate those data back to host.  These predefined activities include
start and finish of a workgroup execution, and dsp cache coherency operations.
On the host side, users will need to use dsptop utility to retrieve the
information.  You probably will need two windows/consoles and run your
OpenCL application after dsptop is run, for example,

#. In window 1, run ``dsptop -l last``
#. In window 2, launch your OpenCL application, wait for it to finish
#. Back in window 1, type "q" to quit dsptop.  dsptop should print out
   the information sent back from the DSP side.

Details about usage of dsptop can be found by running ``dsptop -h`` and by
this `dsptop wikipage`_ .

.. _dsptop wikipage: http://processors.wiki.ti.com/index.php/Dsptop

