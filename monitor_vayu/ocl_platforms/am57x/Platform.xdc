metaonly module Platform inherits xdc.platform.IPlatform 
{
config ti.platforms.generic.Platform.Instance CPU =
  ti.platforms.generic.Platform.create("CPU", {
    clockRate:      600,                                       
    catalogName:    "ti.catalog.c6000",
    deviceName:     "DRA7XX",

    customMemoryMap: 
    [ 
      /*-----------------------------------------------------------------------
      * L1D regions, both l1d cache and l1d scratchpad will reside within this
      *----------------------------------------------------------------------*/
      [ "L1DSRAM",       { name: "L1DSRAM",
                          base: 0x00F00000,
                          len:  0x00008000,
                          space: "code/data",
                          access: "RWX", } ],

      /*-----------------------------------------------------------------------
      * L2SRAM : Memory range used by the runtime to store its state (32KB) 
      * OCL_GLOBAL : Memory range used for OpenCL local buffers (128KB)
      *   Note: base, len must be a multiple of Linux page size (shmem.cpp)
      *----------------------------------------------------------------------*/
      [ "L2SRAM",    { name: "L2SRAM", 
                       base: 0x00800000, 
                       len:     0x08000,
                       space: "code/data", 
		       access: "RWX", } ],

      [ "OCL_LOCAL", { name: "OCL_LOCAL",
                       base: 0x00808000,
                       len:     0x20000,
                       space: "code/data",
                       access: "RWX", } ],

      /*-----------------------------------------------------------------------
      * DDR Related regions
      *----------------------------------------------------------------------*/
      [ "DDR3",         { name: "DDR3", 
                          base: 0xFE800000, 
			  len:  0x00400000,
                          space: "code/data", 
			  access: "RWX", } ],

      /* Each DSP core uses its own framework components data structure */
      [ "DDR3_FC0",     { name: "DDR3_FC0",
                          base: 0xFEC00000,
			  len:  0x00010000,
                          space: "code/data",
			  access: "RWX", } ],
      [ "DDR3_FC1",     { name: "DDR3_FC1",
                          base: 0xFEC10000,
			  len:  0x00010000,
                          space: "code/data",
			  access: "RWX", } ],

      /* Non-cached DDR */
      [ "SR_0",         { name: "SR_0",
                          base: 0xFF000000,
                          len:  0x00100000,
                          space: "data",
                          access: "RWX", } ],

      [ "DDR3_NC",   { name: "DDR3_NC",
                          base: 0xFF100000,
                          len:  0x00F00000,
                          space: "code/data",
                          access: "RWX", } ],

    ],

    l2Mode:"128k", 
    l1PMode:"32k", 
    l1DMode:"32k",
});

instance :
  override config string codeMemory  = "DDR3";
  override config string stackMemory = "L2SRAM";
  override config string dataMemory  = "L2SRAM";
}
