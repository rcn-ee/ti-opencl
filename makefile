#.SILENT:

# Determine if cross-compiling and set appropriate CMAKE options
ifeq ($(TARGET_OS),SYS_BIOS)
    CMAKE_DEFINES = -DCMAKE_TOOLCHAIN_FILE=../host/cmake/CMakeBiosARMToolChain.txt -G "MinGW Makefiles"
else
ifneq (,$(findstring 86, $(shell uname -m)))
    CMAKE_DEFINES = -DCMAKE_TOOLCHAIN_FILE=../host/cmake/CMakeARMToolChain.txt
else
    CMAKE_DEFINES = -DDEFAULT_DEV_INSTALL_DIR=/opt/ti
endif
endif
# Default to K2H build. If BUILD_AM57 is set, build for AM57.
ifeq ($(TARGET_OS),SYS_BIOS)
CMAKE_DEFINES += -DBUILD_TARGET=ARM_AM57
OCL_BUILD_DIR  = buildbios
else
ifeq ($(BUILD_AM57),1)
CMAKE_DEFINES += -DBUILD_TARGET=ARM_AM57
OCL_BUILD_DIR  = builda
else
CMAKE_DEFINES += -DBUILD_TARGET=ARM_K2H
OCL_BUILD_DIR  = buildh
endif
endif
ifeq (,$(OCL_BUILD_DIR))
	$(error OCL_BUILD_DIR not defined)
endif

CLEAN_DIRS = host monitor monitor_vayu builtins examples libm host/clocl
ifeq ($(TARGET_OS),SYS_BIOS)

install: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR)/
	cd /d $(OCL_BUILD_DIR) && cmake $(CMAKE_DEFINES) ../host/src 
	cd /d $(OCL_BUILD_DIR) && gmake -f ../monitor_vayu/makefile.bios all
	cd /d $(OCL_BUILD_DIR) && gmake install 
else
install: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR); cmake $(CMAKE_DEFINES) ../host; make -j4 install;

package: $(OCL_BUILD_DIR)
	cd $(OCL_BUILD_DIR); cmake $(CMAKE_DEFINES) ../host; make -j4 package;

clean:
	for dir in $(CLEAN_DIRS); do \
	   $(MAKE) -C $$dir clean; \
	done
	rm -rf $(OCL_BUILD_DIR)/*
endif
fresh: clean install

$(OCL_BUILD_DIR): 
	mkdir -p $(OCL_BUILD_DIR)

change:
	  git log --pretty=format:"- %s%n%b" $(TAG).. ; \
