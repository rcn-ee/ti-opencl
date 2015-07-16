.SILENT:

# Determine if cross-compiling and set appropriate CMAKE options
ifneq (,$(findstring 86, $(shell uname -m)))
    CMAKE_DEFINES = -DCMAKE_TOOLCHAIN_FILE=../host/cmake/CMakeARMToolChain.txt
else
    CMAKE_DEFINES = -DDEFAULT_DEV_INSTALL_DIR=/opt/ti
endif

# Default to K2X build. If BUILD_AM57 is set, build for AM57.
ifeq ($(BUILD_AM57),1)
CMAKE_DEFINES += -DBUILD_TARGET=ARM_AM57
OCL_BUILD_DIR  = builda
else
CMAKE_DEFINES += -DBUILD_TARGET=ARM_K2X
OCL_BUILD_DIR  = buildh
endif

ifeq (,$(OCL_BUILD_DIR))
	$(error OCL_BUILD_DIR not defined)
endif

CLEAN_DIRS = monitor monitor_vayu builtins examples libm host/clocl

install: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR); cmake $(CMAKE_DEFINES) ../host; make -j4 install;

package: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR); cmake $(CMAKE_DEFINES) ../host; make -j4 package;

clean:
	for dir in $(CLEAN_DIRS); do \
	   $(MAKE) -C $$dir clean; \
	done
	rm -rf $(OCL_BUILD_DIR)/*

fresh: clean install

$(OCL_BUILD_DIR): 
	mkdir -p $(OCL_BUILD_DIR)

change:
	  git log --pretty=format:"- %s%n%b" $(TAG).. ; \
