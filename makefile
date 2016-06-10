include host/Makefile.inc

# Determine if cross-compiling and set appropriate CMAKE options
CMAKE_DEFINES = -DARM_LLVM_DIR=$(ARM_LLVM_DIR) -DX86_LLVM_DIR=$(X86_LLVM_DIR)
ifneq ($(BUILD_DSPC),1)
ifneq (,$(findstring 86, $(shell uname -m)))
    ifeq ($(BUILD_OS), SYS_BIOS)
        export GCC_ARM_NONE_TOOLCHAIN:=$(GCC_ARM_NONE_TOOLCHAIN)
        TOOLCHAIN_FILE=../host/cmake/CMakeBiosARMToolChain.txt
    else
        TOOLCHAIN_FILE=../host/cmake/CMakeARMToolChain.txt
    endif
    CMAKE_DEFINES += -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE)
endif
endif

CLEAN_DIRS = builtins examples libm host/clocl

ifeq ($(BUILD_AM57),1)
    TARGET=am57
    BUILD_TARGET=ARM_AM57
    ifneq ($(BUILD_OS), SYS_BIOS)
        CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(PSDK_LINUX_DEVKIT_ROOT)
        export PATH:=$(ARM_GCC49_DIR)/bin:$(PATH)
    else
	export DESTDIR?=$(CURDIR)/install/ti-opencl-rtos-$(TARGET)-$(OCL_FULL_VER)/packages/ti/opencl
	export GCC_ARM_NONE_TOOLCHAIN
        CMAKE_DEFINES += -DBUILD_OS=SYS_BIOS -DPSDK_RTOS=$(DEFAULT_PSDK_RTOS)
    endif
    CLEAN_DIRS += monitor_vayu
else ifeq ($(BUILD_K2H),1)
    TARGET=k2h
    BUILD_TARGET=ARM_K2H
    CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(PSDK_LINUX_DEVKIT_ROOT_K2X)
    export PATH:=$(ARM_GCC49_DIR)/bin:$(PATH)
    CLEAN_DIRS += monitor
else ifeq ($(BUILD_K2L),1)
    TARGET=k2l
    BUILD_TARGET=ARM_K2L
    CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(PSDK_LINUX_DEVKIT_ROOT_K2X)
    export PATH:=$(ARM_GCC49_DIR)/bin:$(PATH)
    CLEAN_DIRS += monitor
else ifeq ($(BUILD_K2E),1)
    TARGET=k2e
    BUILD_TARGET=ARM_K2E
    CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(PSDK_LINUX_DEVKIT_ROOT_K2X)
    export PATH:=$(ARM_GCC49_DIR)/bin:$(PATH)
    CLEAN_DIRS += monitor
else ifeq ($(BUILD_K2G),1)
    TARGET=k2g
    BUILD_TARGET=ARM_K2G
    CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(PSDK_LINUX_DEVKIT_ROOT_K2X)
    export PATH:=$(ARM_GCC49_DIR)/bin:$(PATH)
    CLEAN_DIRS += monitor_vayu
else ifeq ($(BUILD_DSPC),1)
    TARGET=dspc
    BUILD_TARGET=DSPC868x
    CMAKE_DEFINES += -DSDK=$(DEFAULT_DLSDK)
    CLEAN_DIRS += monitor
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

CMAKE_DEFINES += -DBUILD_TARGET=$(BUILD_TARGET)
CMAKE_DEFINES += -DOCL_VERSION=$(OCL_FULL_VER)
OCL_BUILD_DIR = build/$(TARGET)$(BUILD_OS)
OCL_INSTALL_DIR = install/$(TARGET)$(BUILD_OS)
export DESTDIR?=$(CURDIR)/$(OCL_INSTALL_DIR)


install: $(OCL_BUILD_DIR) $(DESTDIR)
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

change:
	git log --pretty=format:"- %s%n%b" $(TAG).. ; \

version:
	@echo $(OCL_VER)
