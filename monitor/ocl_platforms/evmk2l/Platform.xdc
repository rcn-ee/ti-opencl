metaonly module Platform inherits xdc.platform.IPlatform 
{
config ti.platforms.generic.Platform.Instance CPU =
  ti.platforms.generic.Platform.create("CPU", {
    clockRate:      1000,                                       
    catalogName:    "ti.catalog.c6000",
    deviceName:     "TMS320TCI6630K2L",

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
      * L2 Related regions
      *----------------------------------------------------------------------*/
      [ "L2SRAM",    { name: "L2SRAM", 
                       base: 0x00800000, 
                       len:     0x20000,    
                       space: "code/data", 
		       access: "RWX", } ],

      [ "OCL_LOCAL", { name: "OCL_LOCAL",
                       base: 0x00820000,
                       len:     0xc0000,  
                       space: "code/data",
                       access: "RWX", } ],

      /*-----------------------------------------------------------------------
      * MSMC related regions
      *----------------------------------------------------------------------*/
      [ "MSMC_NC_PHYS", { name: "MSMC_NC_PHYS", 
                          base: 0x0C000000,
			  len:  0x00080000,
                          space: "code/data", 
			  access: "RWX", } ],

      [ "MSMC_NC_VIRT", { name: "MSMC_NC_VIRT", 
                          base: 0xA1000000, 
			  len:  0x00080000,
                          space: "code/data", 
			  access: "RWX", } ],

      /*-----------------------------------------------------------------------
      * DDR Releated regions
      *----------------------------------------------------------------------*/
      [ "DDR3",         { name: "DDR3", 
                          base: 0xA0000000, 
			  len:  0x01000000,
                          space: "code/data", 
			  access: "RWX", } ],

      /* 0xA1000000 - 0xA1100000 also used for MSMC_NC no caching setting    */
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
