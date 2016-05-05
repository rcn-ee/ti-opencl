#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = /cgnas/ti-processor-sdk-rtos-am57xx-evm-02.00.01.07-release/bios_6_45_00_19/packages
override XDCROOT = /cgnas/ti-processor-sdk-rtos-am57xx-evm-02.00.01.07-release/xdctools_3_31_02_38_core
override XDCBUILDCFG = ../../config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = /cgnas/ti-processor-sdk-rtos-am57xx-evm-02.00.01.07-release/bios_6_45_00_19/packages;/cgnas/ti-processor-sdk-rtos-am57xx-evm-02.00.01.07-release/xdctools_3_31_02_38_core/packages;../..
HOSTOS = Linux
endif
