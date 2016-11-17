metaonly module Platform inherits xdc.platform.IPlatform 
{
config ti.platforms.generic.Platform.Instance CPU =
  ti.platforms.generic.Platform.create("CPU", {
    clockRate:      1000,                                       
    catalogName:    "ti.catalog.c6000",
    deviceName:     "TMS320TCI6638",

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
                       len:     0x10000,
                       space: "code/data", 
               access: "RWX", } ],

      [ "OCL_LOCAL", { name: "OCL_LOCAL",
                       base: 0x00810000,
                       len:     0xd0000,
                       space: "code/data",
                       access: "RWX", } ],

      /*-----------------------------------------------------------------------
      * MSMC related regions
      *----------------------------------------------------------------------*/
      [ "MSMC_NC_PHYS", { name: "MSMC_NC_PHYS", 
                          base: 0x0C580000,
              len:  0x00080000,
                          space: "code/data", 
              access: "RWX", } ],

      [ "MSMC_NC_VIRT", { name: "MSMC_NC_VIRT", 
                          base: 0xA1000000, 
              len:  0x00080000,
                          space: "code/data", 
              access: "RWX", } ],

      /*-----------------------------------------------------------------------
      * DDR Related regions
      *----------------------------------------------------------------------*/
      [ "DDR3_HEAP",  { name: "DDR3_HEAP",
                        base: 0xA0000000,
                        len:  0x00880000,
                        space: "code/data",
                        access: "RWX", } ],

      /* Stack for ocl_service_omp task - 0x10000 for each core */
      [ "DDR3_STACK", { name: "DDR3_STACK",
                        base: 0xA0880000,
                        len:  0x00080000,
                        space: "code/data",
                        access: "RWX", } ],

      [ "DDR3",       { name: "DDR3",
                        base: 0xA0900000,
                        len:  0x00300000,
                        space: "code/data",
                        access: "RWX", } ],

      /* Core specific DDR regions mapped to the same "virtual" address using MPAX
       * The length must be consistent with MPAX configuration on device_k2x.c
       */
      [ "DDR3_CORE0", { name: "DDR3_CORE0", 
                        base: 0xA0c00000,
                        len:  0x00040000,
                        space: "code/data",
                        access: "RWX", } ],

      [ "DDR3_CORE1", { name: "DDR3_CORE1", 
                        base: 0xA0c40000,
                        len:  0x00040000,
                        space: "code/data",
                        access: "RWX", } ],

      [ "DDR3_CORE2", { name: "DDR3_CORE2", 
                        base: 0xA0c80000,
                        len:  0x00040000,
                        space: "code/data",
                        access: "RWX", } ],

      [ "DDR3_CORE3", { name: "DDR3_CORE3", 
                        base: 0xA0cc0000,
                        len:  0x00040000,
                        space: "code/data",
                        access: "RWX", } ],

      [ "DDR3_CORE4", { name: "DDR3_CORE4", 
                        base: 0xA0d00000,
                        len:  0x00040000,
                        space: "code/data",
                        access: "RWX", } ],

      [ "DDR3_CORE5", { name: "DDR3_CORE5", 
                        base: 0xA0d40000,
                        len:  0x00040000,
                        space: "code/data",
                        access: "RWX", } ],

      [ "DDR3_CORE6", { name: "DDR3_CORE6", 
                        base: 0xA0d80000,
                        len:  0x00040000,
                        space: "code/data",
                        access: "RWX", } ],

      [ "DDR3_CORE7", { name: "DDR3_CORE7", 
                        base: 0xA0dc0000,
                        len:  0x00040000,
                        space: "code/data",
                        access: "RWX", } ],

      /* 0xA1000000 - 0xA1100000 also used for MSMC_NC no caching setting    */


      [ "DDR3_VIRT", { name: "DDR3_VIRT", 
                       base: 0xA2000000,
                       len:  0x00040000,
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
