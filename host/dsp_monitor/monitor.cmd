-heap  0x1000 -stack 0x1000

#define K(x) (x*1024)
#define M(x) (x*1024*1024)
#define G(x) (x*1024*1024*1024)

MEMORY
{
    L2SRAM0 	(RWX) : org = 0x00800000,    len = 128   
    L2SRAM    	(RWX) : org = END(L2SRAM0),  len = K(256) - 128
    L2SRAM_RESV (RWX) : org = 0x00840000,    len = K(128)
    L1PSRAM 	(RWX) : org = 0x00E00000,    len = K(32)
    L1DSRAM 	(RW)  : org = 0x00F00000,    len = K(32)
    MSMCSRAM 	(RWX) : org = 0x0C100000,    len = M(3)
    PDSP1D      (RWX) : org = 0x340B8000,    len = K(16)
    PDSP2D      (RWX) : org = 0x340BC000,    len = K(8)
    MAIL_D2H    (RW)  : org = 0x80000000,    len = K(512)
    MAIL_H2D    (RW)  : org = END(MAIL_D2H), len = K(512)
    MSMC_NC     (RWX) : org = 0x80100000,    len = M(1)
    DDR3_NC     (RWX) : org = 0x80200000,    len = M(14)
    DDR3        (RWX) : org = 0x81000000,    len = M(16)
    DDR3_RESV   (RWX) : org = 0x82000000,    len = G(1) - M(32)
}

SECTIONS
{
    .workgroup_config  >  L2SRAM0

    GROUP:             > L2SRAM
    {
        .neardata:
        .rodata:
        .bss:
    }

    platform_lib      > L2SRAM
    .stack:           > L2SRAM
    .pinit:           > L2SRAM
    .init_array:      > L2SRAM
    .data:            > L2SRAM
    .fardata:         > L2SRAM
    .far:             > L2SRAM
    .cio:             > L2SRAM
    .sysmem:          > L2SRAM

    GROUP:            > DDR3
    {
	.text:_c_int00
	.text:
	.cinit:
	.const:
	.switch:
    }

    .dsp2hostMailBox: > MAIL_D2H
    .host2dspMailBox: > MAIL_H2D
}
