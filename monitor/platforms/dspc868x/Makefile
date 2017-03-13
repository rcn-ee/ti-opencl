# DEPENDS ON THE XDC ENVIRONMENT VARIABLE

all: Platform.xdc
	@$(XDC)/xdc --xdcpath=$(BIOS_DIR)/packages -PR .

clean: 
	@$(XDC)/xdc clean -PR .
	@rm -f package/*.ccs
	@rm -f package/*.class
	@rm -f package/*.java
	@rm -f package/package.defs.h
	@rm -f package/package.xdc.dep
	@rm -f package/package.xdc.inc
	@rm -f package/*.c
	@rm -f package/.xdc-y23
	@rm -f package/.vers_r170
	@rm -f package/.vers_g180
	@rm -f package/.vers_b150
	@rm -f package/*.sch
	@rm -f -r package/cfg
	@rm -f -r package/lib
	@rm -f -r package/internal
	@rm -f -r package/external
