******************************************************
The OpenCL Memory Model
******************************************************

The OpenCL 1.1 specification
http://www.khronos.org/registry/cl/specs/opencl-1.1.pdf available from Khronos
defines a memory model in Section 3.3.  Please refer to the specification for
details on these memory regions and how they relate to work-items, work-groups,
and kernels. This document will focus on the mapping of the OpenCL memory model
to TI devices. There are four virtual memory regions defined.

Global Memory 
  This memory region contains global buffers and is the primary conduit for
  data transfers from the host A15 CPUs to/from the C66 DSPs. This region will
  also contain OpenCL C program code that will be executed on the C66 DSPs.
  For this OpenCL implementation, global memory by default maps to the portion
  of DDR3 partitioned as CMEM contiguous memory.  

  On K2x devices, MSMC memory is also available as global memory and buffers
  can be defined to reside in this memory instead of DDR3 through an OpenCL API
  extension specific to TI. This mechanism will be described in a later section
  that details handling of the OpenCL buffer creation flags.  

Constant Memory
  This memory region contains content that remains constant during the
  execution of a kernel.  OpenCL C program code and constant data defined in
  that code would be placed in this region.  For this implementation, constant
  memory is mapped to the portion of DDR3 partitioned as CMEM contiguous
  memory.  

Local Memory 
  The local memory region is not defined by the spec to be accessible from the
  host (ARM A15 cores). This memory is local to a work group.  It can be viewed
  as a core local scratchpad memory and in fact for this implementation it is
  mapped to L2 that is reserved for this purpose.  The use case for local
  memory is for an OpenCL work-group to migrate a portion of a global buffer
  to/from a local buffer for performance reasons.  This use case is optional
  for users as access to global buffers in DDR will be cached in both the L2
  cache and the L1D cache on the C66 DSPs.  However, performance can often be
  improved by taking the extra step in OpenCL C programs to manage local memory
  as a scratchpad.  

Private Memory 
  This memory region is for values that are private to a work-item. These
  values are typically allocated to registers in the C66 DSP core.  Sometimes
  it may be necessary for these values to exist in memory.  In these cases the
  values are stored on the C66 DSP stack, which resides in the reserved portion
  of the L2 memory.
