*****************************************************************************
Guidelines for porting Stand-alone DSP applications to OpenCL
*****************************************************************************
For C66 DSP developers moving to OpenCL from a standalone DSP
application environment, the following guidelines will be helpful for the
transition.  

Heap Memory Management
======================================
Within an OpenCL application, memory management for the C66 DSP is most
naturally accomplished through the use of OpenCL buffers defined in the host
application.  OpenCL buffers can be defined in DDR, MSMC, and L2 memory
regions. See :doc:`buffers` for details.  If OpenCL buffers are use, this
eliminates the need for heap management on the DSP, i.e. calls to malloc, call,
free, etc...  

However, when porting existing code to run under an OpenCL kernel, it is
sometimes convienient to allow existing heap management calls to continue to
exist. To support this there is a small (<= 8MB) heap available on the DSP's
that can be used to service malloc type calls. This heap will be in DDR and
will be shared across the DSP cores.  If multiple workgroups (DSPs) are
accessing the same location, then it is up to the application to ensure
synchronization to preent race conditions.

If the size of the small heap is insufficient for your needs or you would like
a heap in on-chip shared memory, We have added some additional built-in
functions that allow you to create your own dynamically size heaps using a
Buffer passed into your kernel.  Please see :doc:`dsp-malloc-extension` for
details.

.. Note::
    The size of the small heap may vary from platform to platform and possibly
    from release to release on the same platform.

Stack Usage 
======================================
OpenCL C kernels and any call tree originating from a kernel inherit a stack
from the OpenCL runtime executing on the DSPs.  To limit the amount of on-chip
L2 memory that is reserved on the DSP, this stack is small, currently 10K
bytes.  Auto (function scope) variables and private data should be therefore
kept to a minimum.  Additionally, call tree depth must be limited.  Recursion
is not allowed in OpenCL C code.  If OpenCL C code calls standard C code, then
recursion should not be used in the standard C code.

.. Note::
    The size of the stakc may vary from platform to platform and possibly
    from release to release on the same platform.

Boot Routine Dependencies
======================================
OpenCL C does not run a boot setup for dispatch of kernels, therefore
dependencies on items that typically run before main is called or after main
returns in a standalone DSP application are not supported in a OpenCL C
environment. This would include:

   - C++ constructors and destructors
   - atexit registered functions
   - SysBios Tasks 
   - Some XDC constructs

Linker Command Files
======================================
The OpenCL C compiler does perform a link of OpenCL C code, any called C code,
and built-in function libraries.  However, the addresses bound into the resultant
.out file are not explicitly honored when the .out file is dynamically loaded
for execution.  This decouples the compilation of OpenCL C code from the underlying 
memory map of the platform.  

In this environment, a user linker command file is typically not needed.  The
exception would be the case where you have a named section and would like to
force allocation of that section to either DDR, MSMC, or L2 SRAM. In this case
you have two options.

#. Rather than use your own section names, map your data to one of these pre-existing sections:

    ============ =========================
    Section Name Memory Range Mapped To
    ============ =========================
    .mem_ddr     DDR3
    .mem_msm     MSMC
    .mem_l2      L2SRAM
    ============ =========================

#. Use your own linker command file to place your section names and pass the
   linker command file name as a build option to the OpenCL C compiler clocl.  ::

    SECTIONS
    {
        my_ddr_section  > DDR
        my_msmc_section > MSMC
        my_l2_section   > L2SRAM
    }

OpenCL dynamically loads OpenCL C code (including any called standard C code) so 
any attempt to bind to specific addresses in your linker command file will be ignored.
