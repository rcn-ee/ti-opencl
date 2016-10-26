# DEPENDS ON THE XDC ENVIRONMENT VARIABLE

# If not specified, pick a default location for TI dependent packages
DEFAULT_DEV_INSTALL_DIR ?= $(DEFAULT_PSDK_RTOS)
XDC_DIR        ?= $(wildcard $(DEFAULT_DEV_INSTALL_DIR)/xdctools_*)
BIOS_DIR       ?= $(wildcard $(DEFAULT_DEV_INSTALL_DIR)/bios_*)

all: Platform.xdc
	@$(XDC_DIR)/xdc --xdcpath=$(BIOS_DIR)/packages -PR .

clean: 
	@$(XDC_DIR)/xdc clean --xdcpath=$(BIOS_DIR)/packages -PR .
