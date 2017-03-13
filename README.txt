TI DSP OpenCL Runtime
---------------------

OpenCL user guide available at:
http://software-dl.ti.com/mctools/esd/docs/opencl/index.html

Building OpenCL
---------------

1. Download and build Clang/LLVM using instructions available at:
   http://git.ti.com/opencl/llvm/blobs/release_36_ti/README.ti
2. Update variables in host/Makefile.inc based on the location of the
   Processor SDK.
3. Set environment variable DESTDIR to the location to install the OpenCL build
4. To build for a given SoC, run one of:
   * make BUILD_AM57=1
   * make BUILD_K2H=1
   * make BUILD_K2L=1
   * make BUILD_K2E=1
   * make BUILD_K2G=1
5. Additional build command options:
   * BUILD_OS=SYS_BIOS: only valid with BUILD_AM57=1
   * BUILD_EXAMPLES=1:  only valid with BUILD_OS=SYS_BIOS
