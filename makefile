include host/Makefile.inc

# Determine if cross-compiling and set appropriate CMAKE options
CMAKE_DEFINES = -DARM_LLVM_DIR=$(ARM_LLVM_DIR) -DX86_LLVM_DIR=$(X86_LLVM_DIR)
ifneq ($(BUILD_DSPC),1)
ifneq (,$(findstring 86, $(shell uname -m)))
    ifeq ($(BUILD_OS), SYS_BIOS)
        export GCC_ARM_NONE_TOOLCHAIN:=$(GCC_ARM_NONE_TOOLCHAIN)
        export TI_OCL_CGT_INSTALL:=$(TI_OCL_CGT_INSTALL)
        TOOLCHAIN_FILE=../host/cmake/CMakeBiosARMToolChain.txt
    else
        TOOLCHAIN_FILE=../host/cmake/CMakeARMToolChain.txt
    endif
    CMAKE_DEFINES += -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE)
endif
endif

CLEAN_DIRS = builtins examples libm host/clocl monitor monitor_ipu

ifeq ($(BUILD_AM57),1)
    TARGET=am57
    BUILD_TARGET=ARM_AM57
    ifneq ($(BUILD_OS), SYS_BIOS)
        CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(LINUX_DEVKIT_ROOT)
        BUILD_DLA_FIRMWARE ?= 1
        CMAKE_DEFINES += -DBUILD_DLA_FIRMWARE=$(BUILD_DLA_FIRMWARE)
        export PATH:=$(ARM_GCC_DIR)/bin:$(PATH)
    else
        RTOS_PACKAGE_VER=$(shell echo $(OCL_FULL_VER) | sed 's/\<[0-9]\>/0&/g' | sed 's/\./_/g')
        RTOS_PACKAGE_NAME=opencl_rtos_$(TARGET)xx_$(RTOS_PACKAGE_VER)
        export DESTDIR?=$(CURDIR)/install/$(TARGET)$(BUILD_OS)/$(RTOS_PACKAGE_NAME)/packages/ti/opencl
        export GCC_ARM_NONE_TOOLCHAIN
        CMAKE_DEFINES += -DBUILD_OS=SYS_BIOS -DRTOS_INSTALL_DIR=$(RTOS_INSTALL_DIR) -DXDC_INSTALL_PATH=$(XDC_DIR)/packages
        CLEAN_DIRS += packages/ti/opencl
    endif
else ifeq ($(BUILD_K2H),1)
    TARGET=k2h
    BUILD_TARGET=ARM_K2H
    CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(LINUX_DEVKIT_ROOT)
    export PATH:=$(ARM_GCC_DIR)/bin:$(PATH)
else ifeq ($(BUILD_K2L),1)
    TARGET=k2l
    BUILD_TARGET=ARM_K2L
    CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(LINUX_DEVKIT_ROOT)
    export PATH:=$(ARM_GCC_DIR)/bin:$(PATH)
else ifeq ($(BUILD_K2E),1)
    TARGET=k2e
    BUILD_TARGET=ARM_K2E
    CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(LINUX_DEVKIT_ROOT)
    export PATH:=$(ARM_GCC_DIR)/bin:$(PATH)
else ifeq ($(BUILD_K2G),1)
    TARGET=k2g
    BUILD_TARGET=ARM_K2G
    CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(LINUX_DEVKIT_ROOT)
    export PATH:=$(ARM_GCC_DIR)/bin:$(PATH)
else ifeq ($(BUILD_DSPC),1)
    TARGET=dspc
    BUILD_TARGET=DSPC868x
    CMAKE_DEFINES += -DSDK=$(DEFAULT_DLSDK)
else
    ifeq ($(MAKECMDGOALS),clean)
    else ifeq ($(MAKECMDGOALS),realclean)
    else ifeq ($(MAKECMDGOALS),version)
    else
        $(error must specify one of: \
            BUILD_AM57=1 \
            BUILD_K2H=1 \
            BUILD_K2L=1 \
            BUILD_K2E=1 \
            BUILD_DSPC=1)
    endif
endif

ifeq ($(BUILD_EXAMPLES),1)
	export XDC_INSTALL_DIR=$(XDC_DIR)
	export IPC_INSTALL_DIR=$(RTOS_INSTALL_DIR)/ti-ipc-tree
	export BIOS_INSTALL_DIR=$(RTOS_INSTALL_DIR)/ti-sysbios-tree
    CMAKE_DEFINES += -DBUILD_EXAMPLES=1
endif

CMAKE_DEFINES += -DBUILD_TARGET=$(BUILD_TARGET)
CMAKE_DEFINES += -DOCL_VERSION=$(OCL_FULL_VER)
OCL_BUILD_DIR = build/$(TARGET)$(BUILD_OS)
OCL_INSTALL_DIR = install/$(TARGET)$(BUILD_OS)
ifeq ($(BUILD_DLA_FIRMWARE),1)
    DLA_SUBMODULE = dla_submodule
endif
export DESTDIR?=$(CURDIR)/$(OCL_INSTALL_DIR)


install: $(OCL_BUILD_DIR) $(DESTDIR) $(DLA_SUBMODULE)
	cd $(OCL_BUILD_DIR) && cmake $(CMAKE_DEFINES) ../../host && $(MAKE) install

.PHONY: build
build: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR) && cmake $(CMAKE_DEFINES) ../../host && $(MAKE)

package: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR) && cmake $(CMAKE_DEFINES) ../../host && $(MAKE) package

clean:
	if [ -z "$(TARGET)" ]; then \
         	for trg in K2H K2L K2E K2G; do \
             		$(MAKE) -f makefile BUILD_$$trg=1 clean; \
         	done; \
             	$(MAKE) -f makefile BUILD_AM57=1 clean; \
             	$(MAKE) -f makefile BUILD_AM57=1 BUILD_OS=SYS_BIOS clean; \
    	else \
        	for dir in $(CLEAN_DIRS); do \
            		echo $(MAKE) -C $$dir BUILD_TARGET=$(BUILD_TARGET) BUILD_OS=$(BUILD_OS) clean; \
            		$(MAKE) -C $$dir BUILD_TARGET=$(BUILD_TARGET) BUILD_OS=$(BUILD_OS) clean; \
         	done; \
         	rm -rf "$(OCL_BUILD_DIR)"; \
         	rm -rf "$(OCL_INSTALL_DIR)"; \
    	fi

fresh: clean install

$(OCL_BUILD_DIR):
	mkdir -p $(OCL_BUILD_DIR)

$(DESTDIR):
	mkdir -p $(DESTDIR)

.PHONY: dla_submodule
dla_submodule:
	git submodule update --init

change:
	git log --pretty=format:"- %s%n%b" $(TAG).. ; \

version:
	@echo $(OCL_VER)
