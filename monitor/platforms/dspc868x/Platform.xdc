metaonly module Platform inherits xdc.platform.IPlatform 
{
config ti.platforms.generic.Platform.Instance CPU =
  ti.platforms.generic.Platform.create("CPU", {
    clockRate:      1000,                                       
    catalogName:    "ti.catalog.c6000",
    deviceName:     "TMS320C6678",

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
      [ "L2SRAM",       { name: "L2SRAM", 
                          base: 0x00800000, 
                          len:  0x00010000, 
		          space: "code/data", 
			  access: "RWX", } ], 

      [ "OCL_LOCAL",    { name: "OCL_LOCAL", 
                          base: 0x00810000,        
		          len:  0x00050000,
		          space: "code/data", 
		          access: "RWX", } ], 

      /*-----------------------------------------------------------------------
      * MSMC related regions
      *----------------------------------------------------------------------*/
      [ "OCL_MSMC",     { name: "OCL_MSMC", 
                          base: 0x0C000000,
		          len:  0x00380000,
		          space: "code/data", 
		          access: "RWX", } ], 

      [ "MSMC_NC_PHYS", { name: "MSMC_NC_PHYS", 
                          base: 0x0C380000, 
			  len:  0x00080000, 
		          space: "code/data", 
			  access: "RWX", } ], 

      [ "MSMC_NC_VIRT", { name: "MSMC_NC_VIRT", 
                          base: 0xFFF80000, 
			  len:  0x00080000, 
		          space: "code/data", 
			  access: "RWX", } ], 

      /*-----------------------------------------------------------------------
      * DDR Releated regions
      *----------------------------------------------------------------------*/
      [ "DDR3",         { name: "DDR3", 
                          base: 0x80000000, 
			  len:  0x01000000, 
		          space: "code/data", 
			  access: "RWX", } ], 

      [ "OCL_GLOBAL",   { name: "OCL_GLOBAL", 
                          base: 0x81000000,
			  len:  0x3e000000,
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
