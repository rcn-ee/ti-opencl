metaonly module Platform inherits xdc.platform.IPlatform 
{
config ti.platforms.generic.Platform.Instance CPU =
  ti.platforms.generic.Platform.create("CPU", {
    clockRate:      1000,                                       
    catalogName:    "ti.catalog.c6000",
    deviceName:     "DRA7XX",

    customMemoryMap: 
    [ 
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
                       len:     0x08000,  
                       space: "code/data",
                       access: "RWX", } ],


      // L2CACHE  008E:0000 = 008F:FFFF

      /*-----------------------------------------------------------------------
      * MSMC related regions
      *----------------------------------------------------------------------*/
      // 0C00:0000 to 0C03:FFFF reserved for MPI
/* No MSMC...
      [ "OCL_MSMC", { name: "OCL_MSMC" ,
                      base: 0x0C040000,          
                      len:  0x00500000 - 0x40000, // 5M-256K
                      space: "code/data",
                      access: "RWX", } ],

      [ "MSMC_NC_PHYS", { name: "MSMC_NC_PHYS", 
                          base: 0x0C500000, 
			  len:  0x00100000,
                          space: "code/data", 
			  access: "RWX", } ],

      [ "MSMC_NC_VIRT", { name: "MSMC_NC_VIRT", 
                          base: 0xA1000000, 
			  len:  0x00100000,
                          space: "code/data", 
			  access: "RWX", } ],
*/
      /*-----------------------------------------------------------------------
      * DDR Releated regions
      *----------------------------------------------------------------------*/
      [ "DDR3",         { name: "DDR3", 
                          base: 0xFE800000, 
			  len:  0x00600000,
                          space: "code/data", 
			  access: "RWX", } ],

      /* 0xA1000000 - 0xA6000000 reserved for Hyperlink                      */
      /* 0xA1000000 - 0xA1100000 also used for MSMC_NC no caching setting    */

      [ "OCL_GLOBAL", { name: "OCL_GLOBAL",
                        base: 0xFEE00000, 
			len:  0x00100000,
                        space: "code/data",
                        access: "RWX", } ],

      /* Non-cached DDR */
      [ "DDR3_NC",   { name: "DDR3_NC",
                          base: 0xFF000000,
                          len:  0x01000000,
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
