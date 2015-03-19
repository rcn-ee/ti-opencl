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
      * L2 Related regions
      *----------------------------------------------------------------------*/
      [ "OCL_LOCAL", { name: "OCL_LOCAL", 
                       base: 0x00800000,        
		       len: 0x80000 - 0x20000 - 0x20000,  // 512K - l2Cache(=128K) - size(L2SRAM)
		       space: "code/data", 
		       access: "RWX", } ], 

      [ "L2SRAM",       { name: "L2SRAM", 
                          base: 0x00840000, 
                          len: 0x20000, 
		          space: "code/data", 
			  access: "RWX", } ], 

      /*-----------------------------------------------------------------------
      * MSMC related regions
      *----------------------------------------------------------------------*/
      [ "OCL_MSMC", { name: "OCL_MSMC", 
                      base: 0x0C000000,  // end(MSMC_NC_PHYS)
		      len:  0x00400000 - 0x100000,    // 4M - size(MSMC_NC_PHYS)
		      space: "code/data", 
		      access: "RWX", } ], 

      [ "MSMC_NC_PHYS", { name: "MSMC_NC_PHYS", 
                          base: 0x0C300000, 
			  len:  0x00100000, 
		          space: "code/data", 
			  access: "RWX", } ], 

      [ "MSMC_NC_VIRT", { name: "MSMC_NC_VIRT", 
                          base: 0xFF000000, 
			  len:  0x00100000, 
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

      [ "OCL_GLOBAL", { name: "OCL_GLOBAL", 
                        base: 0x80000000 + 0x01000000,   // end(DDR3)
			len:  0x40000000 - 0x01000000,   // 1G - size(DDR3)
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
