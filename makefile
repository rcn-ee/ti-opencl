.SILENT:

# Determine if cross-compiling and set appropriate CMAKE options
ifneq (,$(findstring 86, $(shell uname -m)))
    CMAKE_DEFINES = -DCMAKE_TOOLCHAIN_FILE=../host/cmake/CMakeARMToolChain.txt
else
    CMAKE_DEFINES = -DDEFAULT_DEV_INSTALL_DIR=/opt/ti
endif

CLEAN_DIRS = host monitor builtins examples libm host/clocl

install:
	cd buildh; cmake $(CMAKE_DEFINES) ../host; make -j4 install; 

package:
	cd buildh; cmake $(CMAKE_DEFINES) ../host; make -j4 package; 

clean:
	for dir in $(CLEAN_DIRS); do \
	   $(MAKE) -C $$dir clean; \
	done
	rm -rf buildh_*
	rm -rf buildh/*

fresh: clean install

change:
	  git log --pretty=format:"- %s%n%b" $(TAG).. ; \
