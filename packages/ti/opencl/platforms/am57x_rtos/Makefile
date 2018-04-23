# DEPENDS ON THE XDC ENVIRONMENT VARIABLE

# If not specified, pick a default location for TI dependent packages
include ../../../../../host/Makefile.inc

XDC_DIR        ?= $(wildcard $(DEFAULT_PSDK_RTOS)/xdctools_*)
BIOS_DIR       ?= $(wildcard $(DEFAULT_PSDK_RTOS)/bios_*)

all: Platform.xdc
	@$(XDC_DIR)/xdc --xdcpath=$(BIOS_DIR)/packages -PR .

clean: 
	@$(XDC_DIR)/xdc clean --xdcpath=$(BIOS_DIR)/packages -PR .
