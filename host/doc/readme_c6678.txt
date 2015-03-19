*-----------------------------------------------------------------------------
Open CL(TM) 1.1 Product Version 0.4.0, from Texas Instruments, Inc.
*-----------------------------------------------------------------------------

*-----------------------------------------------------------------------------
* INSTALLATION
*-----------------------------------------------------------------------------
1) This installation is not setup to coexist with other versions of this
   product. This is due to environment variables that point into the
   installation.  Therefore, you should uninstall all previous versions of the
   TI OpenCL product before installation of this version.  

2) The installation program modifies your shell's .rc file (e.g. .bashrc,
   .tcshrc, .cshrc, etc) to create or append to three environment variables,
   PATH, LD_LIBRARY_PATH, TI_OCL_INSTALL.  You will need to re-source that .rc 
   file for the changes to take effect.

3) The OpenCL product or some of the examples in the product are dependent on
   Ubuntu packages that are not typically installed by default.  This step will
   install these packages and will require sudo privileges or an administrator 
   to execute. Execute the following commands:  

   sudo apt-get install libpciaccess-dev binutils-dev ia32-libs libsdl1.2-dev
   sudo apt-get install mesa-common-dev 

Note 1) The OpenCL product is dependent on a kernel module that allows for
   contiguous memory allocation on the Linux host.  Kernel modules can be
   specific to the Linux kernel version you are running.  This package
   contains the source for the module and is custom built for your linux
   version as part of the installation process. The install (and uninstall) 
   package does require sudo privileges for portions of the install process
   and will request an administrator password.  The scripts
   $TI_OCL_INSTALL/scripts/install.sh and $TI_OCL_INSTALL/scripts/uninstall.sh
   can be inspected for details on the commands that are run and require sudo
   privilege.


*-----------------------------------------------------------------------------
* SUMMARY OF DELTAS 0.1.6 from 0.1.5 
*-----------------------------------------------------------------------------

*-----------------------------------------------------------------------------
* SUMMARY OF DELTAS 0.1.5 from 0.1.4 
*-----------------------------------------------------------------------------
- More reliable installation and uninstallation of the cmem module

- Updated the C66 compiler tools to be based on version 7.5.0A13072

*-----------------------------------------------------------------------------
* SUMMARY OF DELTAS 0.1.4 from 0.1.3 
*-----------------------------------------------------------------------------
- Updated to use the TI Desktop Linux SDK version 01.00.00.07

- Updated internal LLVM usage from version 3.0 to 3.2 libraries

- More accurate handling of the DSPC8682

- Increased the OpenCL global buffer area from 992M to 1023M

- Increased the OpenCL local buffer area from 128K to 256K

- The default speed of the DSP was modified from 1.25 Ghz to 1.00 Ghz. 
  This was due to the fact that most of the devices on the Advantech cards
  are qualified for only 1.00 Ghz and some instability was seen running at
  1.25 Ghz.  See below for an environment variable you can set that will
  change the DSP speed back to 1.25 GHz.

- Added logic to reset certain persistent configurations of the DSP device 
  that could cause incorrect behavior when intermixing the run of an opencl
  application with a non opencl application using the DSP devices in 
  a conflicting manner.

- General bug fixes and stability improvements.

*-----------------------------------------------------------------------------
* SUMMARY OF DELTAS 0.1.3 from 0.1.2 
*-----------------------------------------------------------------------------
- Updated to use all DSPs found in the PCIe subsystem.  Previously it was
  fixed to 4 dsps.  This should allow multiple cards to be discovered and
  used. It should also allow all 8 dsps on an octal card to be found. 
  Note: The octal card setup had not been tested. 

- Updated the mandelbrot demo to use an image size of 720 instead of 500.  720
  was chosen because it is divisible by 4, 5, 8, 9, and 16 which allows for a
  simple division of labor across a number of configurations.  It was also
  updated to print the names of all devices being used for pixel generation.

*-----------------------------------------------------------------------------
* SUMMARY OF DELTAS 0.1.2 from 0.1.1 
*-----------------------------------------------------------------------------
- The DSP compiler would sometimes fail to compile OpenCL C code that included 
  vector types, ie float2, int3, etc.  This bug has been resolved.

- Certain OpenCL C kernels would cause a segmentation fault in the dynamic
  loader in the OpenCL library.  The gdb stack dump would list the function
  process_rela_table.isra.7 as the faulting function.  This bug has been
  resolved.

- Version 0.1.1 would not allow local (__local) address qualified variables to
  be defined in function scope.  This bug has been resolved.

- The OpenCL C as_<type> builtins have been added.

- Version 0.1.2 will introduce a 32bit version of the library.

*-----------------------------------------------------------------------------
* SUMMARY OF DELTAS 0.1.1 from 0.1.0 
*-----------------------------------------------------------------------------
- Stability improvements.
- Install and uninstall improvements.  The product is no longer dependent on a
  specific linux kernel version.  It does still depend on a kernel module, but
  the source for that kernel module is shipped with the installation packages
  and is made on the users' machine.  
- Added example simple.

*-----------------------------------------------------------------------------
* PRODUCT DESCRIPTION
*-----------------------------------------------------------------------------
This product is an OpenCL 1.1 implementation.  The OpenCL specification
defines a platform model with a HOST and COMPUTE DEVICES.  For this
implementation the HOST is a 64-bit x86 Linux machine and the COMPUTE DEVICES
are 4 Texas Instruments' TMS320C6678 DSP's resident on a PCIe card installed
in the Linux machine. The x86_64 cpu is also exposed as a fifth compute device 
in this implementation.

*-----------------------------------------------------------------------------
* HARDWARE AND OS REQUIREMENTS
*-----------------------------------------------------------------------------
- Ubuntu 12.04 installation running on an x86 machine.
- An installed Advantech DSPC8681 quad DSP PCIe card configured for little
  endian operation. 

*-----------------------------------------------------------------------------
* OPENCL DOCUMENTATION
*-----------------------------------------------------------------------------
The OpenCL 1.1 specification and the 1.1 C++ bindings specification from
Khronos are included in $(TI_OCL_INSTALL)/doc.

Additional OpenCL resources can be found on the web.  Some links are provided
below.

The OpenCL 1.1 on-line manual pages can be found at:
    http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/

The following page contains links to other OpenCL resources, including books
that may be helpful to you:
    http://www.khronos.org/opencl/resources

*-----------------------------------------------------------------------------
* LIMITATIONS
*-----------------------------------------------------------------------------

- This is an early alpha version of this product.  It is complete enough to be
  useful under some circumstances and as such we would like to get feedback
  from a select number of early adopters.  However, it is by no means a
  complete or compliant implementation.  Taking an existing OpenCL application 
  and linking it against this implementation will not likely produce expected
  results. Some of the major incomplete areas of the implementation are listed
  below. The below list is not a complete list of limitations.

- OpenCL C is not yet fully supported.  In particular, 
    - Kernel arguments less than 32 bits in size cannot be passed to kernels.
    - There is a limit of 10 arguments that may be passed to kernels.
    - Structures may not be passed as arguments to kernels.
    - Only a few OpenCL C built-in functions are supported. 
	- The math fcns that are also part of the std C library are supported.
	- The work group identification functions are supported, i.e.
	  get_global_id(), get_local_id(), get global_size(), etc.
	- The barrier and synchronization functions are not supported.

- OpenCL Out of order Queues (OOOQs) are not yet supported. OOOQs allow 
  enqueued kernels to be serviced before a prior kernel is completed.  This 
  behavior is particularly beneficial if you enqueue tasks rather than
  NDRangekernels.  When OOOQs are supported up to 8 enqueued tasks can be in
  flight simultaneously per DSP device.  OOOQs will also allow overlapped I/O 
  and compute operations allowing a double buffering or pipeline operation.
  For this alpha, only one operation at a time is active within a Queue.

- An OpenCL ICD (Installable Client Driver) is provided with this product, but 
  it will not discover the TI OpenCL implementation. The OpenCL ICD is a
  standard OpenCL library that will discover all installed OpenCL
  implementations on a system and will allow the application to choose a
  platform and dispatch through that platform's implementation. The TI OpenCL
  implementation is not yet ICD compatible and therefore will not be
  discovered.  The ICD library can however be used to discover and dispatch to 
  other vendor implementations.

- The clEnqueueCreateBuffer flags CL_MEM_USE_HOST_MEMORY,
  CL_MEM_ALLOC_HOST_MEMOY, CL_MEM_COPY_HOST_MEMORY are not yet implemented and
  will simply be ignored.

- The OpenCL clEnqueueMapBuffer and clEnqueueUnmapBuffer operations not yet 
  supported.

- OpenCL Images and Samplers are optional features for non GPU devices and are 
  not supported for the DSP devices.  

- The OpenCL api allows for either on-line or off-line compilation of OpenCL C
  kernels. This release only supports the on-line compilation mode for OpenCL C 
  code. As a result, clCreateProgramFromBinaries is not supported yet, nor is 
  querying OpenCL for the binaries associated with a Program object.

	- Even though off-line compilation for OpenCL C code is not yet 
	  supported, OpenCL C code can call standard C code functions and the
	  standard C code functions can be compiled off-line. An example 
	  illustrating this flow is included in the examples sub-directory.
	  The standard C code functions that are called should not include 
	  code that: resets the device, allocates memory blocks that may 
	  conflict with the OpenCL runtime, change the cache configuration, 
	  etc.  OpenCL C code calling C++ code is not supported.

	- Also, compilations of OpenCL C code are cached on the system. If you 
	  run an OpenCL application that on-line compiles some OpenCL C code,
	  the resultant binaries are cached on the system and the next time
	  you run the opencl application, the compilation step is skipped and
	  the cached binaries are used. The caching only uses the OpenCL C
	  code and the compile options as a hash, so an example where the
	  OpenCL C code is calling a C function in a linked object file or
	  library and the object file or library is modified will result in an
	  execution of the OpenCL C linked against the older version of the
	  object.  In this case you will need to clear the OpenCL C compile
	  cache, which can be accomplished with the command 
	  "rm -f /tmp/opencl*".

*-----------------------------------------------------------------------------
* EXAMPLE OPERATION
*-----------------------------------------------------------------------------

There are several OpenCL examples shipped with the product.  I'll explain the
motivation behind each and the steps needed for execution. 

IMPORTANT NOTE: For any of these examples or any OpenCL code you write, 
execution of the code will sometimes appear to hang.  This is due to a known 
issue in the first communication between the Host and the DSP.  It occurs 
intermittently and will be fixed in later releases. There is a decription in 
the LIMITATIONS section of this readme describing workarounds for this problem.

PLATFORM EXAMPLE
----------------
The platform example uses the OpenCL C++ bindings to discover key platform and
device information from the OpenCL implementation and print it to the screen.

To print the information from the TI OpenCL implementation:

    1. cd $TI_OCL_INSTALL/examples/platform
    2. make
    3. ./platform

To print the information from the Any other vendors OpenCL implementation
installed on the system:

    1. cd $TI_OCL_INSTALL/examples/platform
    2. make icd
    3. ./platform_icd

The Makefile in this example directory also illustrates the difference between
linking for the TI implementation of OpenCL and the ICD.

SIMPLE EXAMPLE
-------------
This example simply illustrates the minimum steps needed to dispatch a kernel
to one DSP device and read a buffer of data back.

To run this example:
    1. cd $TI_OCL_INSTALL/examples/simple
    2. make
    3. ./simple 


MANDELBROT EXAMPLE
------------------
The mandelbrot example is a nicely visual OpenCL demo that uses OpenCL to
generate the pixels of a mandelbrot set image.  This example also use the C++
OpenCL binding. The OpenCL kernels are repeatedly called generating images that are zoomed in from the previous image.  This repeats until the zoom factor 
reaches 1E15 or essentially the resolution of a double floating point value. 

This example illustrates several key OpenCL features:
   - It illustrates 4 OpenCL Q's tied to each of the 4 DSPs and a dispatch
     structure  that allows the 4 DSPs to cooperatively generate pixel data.
   - It also illustrates the event wait feature of OpenCL.
   - It illustrates the division of one time setup of OpenCL to the repetitive
     enqueuing of kernels.
   - It also illustrates the ease in which kernels can be shifted from one
     device type to another.

To run this demo:
    1. cd $TI_OCL_INSTALL/examples/mandelbrot
    2. make 
    3. ./mandelbrot dsp
    4. ./mandelbrot cpu
    5. ./mandelbrot all

Step 3 will run the pixel generating kernels on the DSPs.
Step 4 will run the pixel generating kernels on all the CPU cores in the
system.
Step 5 will use both the DSPs and the CPU cores to generate the pixels.

The makefile in this example is also ICD enabled. You can 

    1. cd $TI_OCL_INSTALL/examples/mandelbrot
    2. make icd
    3. ./mandelbrot intel    "If an Intel OpenCL implementation exists"
    4. ./mandelbrot nvidia   "If an Nvidia OpenCL implementation exists"


CCODE EXAMPLE
-------------
This example illustrates the TI extension to OpenCL that allows OpenCL C code
to call standard C code that has been compiled off-line into an object file or
static library. This mechanism can be used to allow optimized C or C callable
assembly routines to be called from OpenCL C code.  It can also be used to
essentially dispatch a standard C function, by wrapping it with an OpenCL C 
wrapper.  Calling C++ routines from OpenCL C is not yet supported.  You should
also ensure that the Standard C function and the call tree resulting from the
standard C function do not allocate device memory, change the cache structure,
or use any resources already being used by the OpenCL runtime. 

To run this example:
    1. cd $TI_OCL_INSTALL/examples/ccode
    2. make
    3. ./ccode 

*-----------------------------------------------------------------------------
* ENVIRONMENT VARIABLES
*-----------------------------------------------------------------------------
TI_OCL_DSP_1_25GHZ:  If this environment variable is set, then the DSPs will be
                     configured to run at 1.25Ghz instead of the standard 1.00
                     Ghz.

TI_OCL_KEEP_FILES:   When OpenCL C kernels are compiled for DSPs, they are
                     compiled to a binary .out file in the /tmp sub-directory.
                     They are then subsequently available for download to the
                     DSPs for running.  The process of compiling generates
                     several intermediate files for each source file.  The
                     OpenCL typically removes these temporary files.  However,
                     it can sometimes be useful to inspect these files.  This
                     environment variable can be set to instruct the runtime to
                     leave the temporary files in /tmp.  This can be useful to
                     inspect the assembly file associated with the out file, to
                     see how well your code was optimized.

TI_OCL_DEBUG_KERNEL: The TI IDE and debugger Code Composer Studio (CCS) is not
		     required for running OpenCL applications with this
		     product, but if you do have CCS installed and and emulator
		     connected to you PCIe card, you can set this environment
		     variable to enable assembly statement level debug of you
		     kernel.  When set, this environment variable will instruct
		     the OpenCL runtime to pause before dispatch of a kernel.
		     While paused the runtime will display data to the user
		     indicating that a kernel dispatch is pending. It will
		     instruct the user to connect to the board through an
		     emulator and will display the appropriate breakpoint
		     address to used for the start of the kernel code.  Having
		     CCS and the emulator insert itself into a running OpenCL
		     application can cause instability in the system in this
		     release and may require a power cycle to the board.  Debug
		     capability has not been a focus for this alpha release and
		     will definitely improve in later releases. Setting up the
		     emulator and CCS is outside the scope of this readme.  If
		     you do have those products, consult the documentation
		     specific to those products.  

*-----------------------------------------------------------------------------
* NOTICES
*-----------------------------------------------------------------------------

* Product is based on a published Khronos Specification, and is expected to 
  pass the Khronos Conformance Testing Process. Current conformance status can 
  be found at www.khronos.org/conformance.
