.SILENT:

# Determine if cross-compiling and set appropriate CMAKE options
ifneq (,$(findstring 86, $(shell uname -m)))
    CMAKE_DEFINES = -DCMAKE_TOOLCHAIN_FILE=../host/cmake/CMakeARMToolChain.txt
else
    CMAKE_DEFINES = -DDEFAULT_DEV_INSTALL_DIR=/opt/ti
endif

# Default to K2H build. If BUILD_AM57 is set, build for AM57.
ifeq ($(BUILD_AM57),1)
CMAKE_DEFINES += -DBUILD_TARGET=ARM_AM57
OCL_BUILD_DIR  = build/am57
else ifeq ($(BUILD_K2L),1)
CMAKE_DEFINES += -DBUILD_TARGET=ARM_K2L
OCL_BUILD_DIR  = build/k2l
else ifeq ($(BUILD_K2E),1)
CMAKE_DEFINES += -DBUILD_TARGET=ARM_K2E
OCL_BUILD_DIR  = build/k2e
else ifeq ($(BUILD_K2H),1)
CMAKE_DEFINES += -DBUILD_TARGET=ARM_K2H
OCL_BUILD_DIR  = build/k2h
else
$(error must specify one of: \
BUILD_K2H=1 \
BUILD_K2L=1 \
BUILD_K2E=1 \
BUILD_AM57=1)
endif

ifeq (,$(OCL_BUILD_DIR))
	$(error OCL_BUILD_DIR not defined)
endif

CLEAN_DIRS = monitor monitor_vayu builtins examples libm host/clocl

install: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR); cmake $(CMAKE_DEFINES) ../../host; make -j4 install;

build: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR); cmake $(CMAKE_DEFINES) ../../host; make -j4;

package: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR); cmake $(CMAKE_DEFINES) ../../host; make -j4 package;

clean:
	for dir in $(CLEAN_DIRS); do \
	   $(MAKE) -C $$dir clean; \
	done
	rm -rf build

fresh: clean install

$(OCL_BUILD_DIR): 
	mkdir -p $(OCL_BUILD_DIR)

change:
	  git log --pretty=format:"- %s%n%b" $(TAG).. ; \
