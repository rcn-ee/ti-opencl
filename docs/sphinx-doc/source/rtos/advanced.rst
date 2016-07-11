******************************************************
Advanced OpenCL RTOS Application Development
******************************************************
Advanced OpenCL RTOS application development requires much deeper RTOS
knowledge and more RTOS/RTSC application development experience.  Essentially,
user can treat OpenCL runtime on host and DSP as libraries that get
invoked and linked into their host and DSP programs.  They can customize
the platform, memory map, IPC, both the OpenCL host runtime and the OpenCL
DSP runtime, etc.  We reserve this mode for expert TI-RTOS users only
and let them handle their own application development.

.. Warning:: 

  If programs running on DSP1 and DSP2 are different and both DSPs are
  participating as OpenCL compute units, user needs to make sure that OpenCL
  exported symbols are laid out exactly the same in each DSP's memory.
  The reason is that once-compiled OpenCL kernel only links against symbols
  on one DSP.  Current OpenCL implementation relies on the fact that symbols
  are similarly laid out in memory on both DSPs and programs the other DSP's
  MMU to ensure that OpenCL exported symbols point to the correct locations.

