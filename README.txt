TI OpenCL
---------

Building OpenCL
---------------

1. Set the environment variable DESTDIR to the location to install the build
2. To build for a given architecture, run one of:
   * make BUILD_AM57=1
   * make BUILD_K2H=1
   * make BUILD_K2L=1
   * make BUILD_K2E=1
   * make BUILD_DSPC=1
3. Additional build command options:
   * BUILD_OS=SYS_BIOS: only valid with BUILD_AM57=1
   * BUILD_EXAMPLES=1:  only valid with BUILD_OS=SYS_BIOS
