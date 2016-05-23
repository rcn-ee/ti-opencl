include host/Makefile.inc

# Determine if cross-compiling and set appropriate CMAKE options
CMAKE_DEFINES = -DARM_LLVM_DIR=$(ARM_LLVM_DIR) -DX86_LLVM_DIR=$(X86_LLVM_DIR)
ifneq ($(BUILD_DSPC),1)
ifneq (,$(findstring 86, $(shell uname -m)))
    ifeq ($(BUILD_OS), SYS_BIOS)
        TOOLCHAIN_FILE=../host/cmake/CMakeBiosARMToolChain.txt
    else
        TOOLCHAIN_FILE=../host/cmake/CMakeARMToolChain.txt
    endif
    CMAKE_DEFINES += -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE)
endif
endif

ifeq ($(BUILD_AM57),1)
    TARGET=am57
    BUILD_TARGET=ARM_AM57
    ifneq ($(BUILD_OS), SYS_BIOS)
        CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(PSDK_LINUX_DEVKIT_ROOT)
        export PATH:=$(ARM_GCC49_DIR)/bin:$(PATH)
    else
        CMAKE_DEFINES += -DBUILD_OS=SYS_BIOS -DPSDK_RTOS=$(DEFAULT_PSDK_RTOS)
        CMAKE_DEFINES += -DARM_GCC_DIR=$(ARM_GCC48NONE_DIR)
        export PATH:=$(ARM_GCC48NONE_DIR)/bin:$(PATH)
	export PACKAGE_ROOT_DIR=$(CURDIR)/install/ti-opencl-rtos-$(TARGET)-$(OCL_FULL_VER)
	export DESTDIR?=$(PACKAGE_ROOT_DIR)/packages/ti/opencl
    endif
else ifeq ($(BUILD_K2H),1)
    TARGET=k2h
    BUILD_TARGET=ARM_K2H
    CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(PSDK_LINUX_DEVKIT_ROOT_K2X)
    export PATH:=$(ARM_GCC49_DIR)/bin:$(PATH)
else ifeq ($(BUILD_K2L),1)
    TARGET=k2l
    BUILD_TARGET=ARM_K2L
    CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(PSDK_LINUX_DEVKIT_ROOT_K2X)
    export PATH:=$(ARM_GCC49_DIR)/bin:$(PATH)
else ifeq ($(BUILD_K2E),1)
    TARGET=k2e
    BUILD_TARGET=ARM_K2E
    CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(PSDK_LINUX_DEVKIT_ROOT_K2X)
    export PATH:=$(ARM_GCC49_DIR)/bin:$(PATH)
else ifeq ($(BUILD_K2G),1)
    TARGET=k2g
    BUILD_TARGET=ARM_K2G
    CMAKE_DEFINES += -DLINUX_DEVKIT_ROOT=$(PSDK_LINUX_DEVKIT_ROOT_K2X)
    export PATH:=$(ARM_GCC49_DIR)/bin:$(PATH)
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

CMAKE_DEFINES += -DBUILD_TARGET=$(BUILD_TARGET)
CMAKE_DEFINES += -DOCL_VERSION=$(OCL_FULL_VER)
OCL_BUILD_DIR = build/$(TARGET)
OCL_INSTALL_DIR = install/$(TARGET)
export DESTDIR?=$(CURDIR)/$(OCL_INSTALL_DIR)

CLEAN_DIRS = monitor monitor_vayu builtins examples libm host/clocl

install: $(OCL_BUILD_DIR) $(DESTDIR)
	cd $(OCL_BUILD_DIR) && cmake $(CMAKE_DEFINES) ../../host && $(MAKE) install

bios_install: install
	echo $(DESTDIR)
	make ECLIPSE_INSTALL_DIR=$(PACKAGE_ROOT_DIR) -C eclipse;
	cp -r ./packages/ti/opencl/* $(DESTDIR)/

build: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR) && cmake $(CMAKE_DEFINES) ../../host && $(MAKE)

package: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR) && cmake $(CMAKE_DEFINES) ../../host && $(MAKE) package

prebuild: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR) && cmake $(CMAKE_DEFINES) ../../host
	$(MAKE) -C monitor BUILD_TARGET=$(BUILD_TARGET)
	ln -s . $(OCL_VERSIONED_NAME)
	tar czf $(OCL_VERSIONED_NAME).tar.gz --exclude='.git' --exclude='init_global_shared_mem' --transform='s/makefile.arm/makefile/g' $(OCL_VERSIONED_NAME)/host $(OCL_VERSIONED_NAME)/debian $(OCL_VERSIONED_NAME)/examples $(OCL_VERSIONED_NAME)/monitor $(OCL_VERSIONED_NAME)/makefile.arm $(OCL_VERSIONED_NAME)/builtins
	rm $(OCL_VERSIONED_NAME)

clean:
	for dir in $(CLEAN_DIRS); do \
	   $(MAKE) -C $$dir clean; \
	done; \
	if [ -z "$(OCL_BUILD_DIR)" ]; then \
	    rm -rf build; \
	else \
	    rm -rf "$(OCL_BUILD_DIR)"; \
	fi; \
	if [ -z "$(OCL_INSTALL_DIR)" ]; then \
	    rm -rf install; \
	else \
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
