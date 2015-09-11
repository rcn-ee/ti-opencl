********************************
Compilation 
********************************

A discussion of OpenCL compilation could describe either; compiling the host
OpenCL application, or compiling OpenCL C programs containing code that is
en-queued to OpenCL devices.  OpenCL was designed to make both easy. However,
there are some options for compilation of OpenCL C programs.  This chapter 
describes the host compilation process and then enumerates and describes the
OpenCL C program compilation process.

Compile Host OpenCL Applications
=======================================================

From the host or CPU application side, OpenCL was designed as a pure library,
meaning that it allows a user of OpenCL to simply include the appropriate
header file in their source and to link the application against an OpenCL
library.  For example, if a file program.cpp already exists and is compiled
with the command :command:`g++ -O3 program.c`, that file could be OpenCL enabled by simply
adding ``#include <CL/cl.h>`` to the file and compiling with the command :command:`g++
-O3 program.c -lOpenCL`.  The ``CL/cl.h`` header file is used to compile host OpenCL
applications using the C API.  If using the C++ bindings instead, then add
``#include <CL/cl.hpp>`` to your file.  Additionally, if using C++ and you would 
like to let the C++ exception handler catch OpenCL API errors, then you
should also add ``#define __CL_ENABLE_EXCEPTIONS`` before you include ``CL/cl.hpp``. 

Compiling OpenCL C Programs
=======================================================

OpenCL provides a generic mechanism for compiling OpenCL C programs for all 
the devices in an OpenCL platform.  The general flow is:

1. Create an OpenCL C program
2. Build the OpenCL C program.

There are two options for how an OpenCL C program is created.  They can be
created from sources or binaries.  If the OpenCL C program is created from
sources, then a textual string is used to create the program and the build step
will invoke an OpenCL C compiler to parse, optimize and generate object from
the code in the string.  If the OpenCL C program is created from binaries, then
binary data that already represents the object code for an OpenCL C program is
used to create the program and the build step can bypass a full compile and
perhaps only link or perform other book-keeping. 

Additionally, you have the choice to embed the content for your program in the
application or to read it from a file.  This is applicable for both programs
created from source and programs created from binary.  The cross product of
these two options give you four cases for OpenCL C program creation:

1. Create an OpenCL program from source, with embedded source,
2. Create an OpenCL program from source, with source in a file,
3. Create an OpenCL program from binary, with embedded binary,
4. Create an OpenCL program from binary, with binary in a file.

Alternatively, we call cases 1 and 2, where the program is created from source,
on-line compilation, because the compiler is called during the run time of your
application.  We call cases 3 and 4, where the program is created from binary,
off-line compilation, because the compiler is called as part of the build of the
application and not during the run time of the application. Clearly on-line
compilation will entail some run time delay as the compilation process
completes, but it does provide for portability of the application, because the
on-line compile is encapsulated in the OpenCL program build step and therefore
the application can run on any OpenCL platform.  Conversely, off-line
compilation will eliminate the run time delay of compilation and will also
provide IP protection since the source for the OpenCL C programs will not need
to be delivered as part of a running application.  When creating programs from
binary, the application must provide a list of devices and a list of
corresponding binaries.  The following examples will assume that only one
device exists and will provide for only one binary.  Please refer to the OpenCL
specification and the C++ binding users guide for assistance in using binaries
for multiple devices.  The below code examples will use the OpenCL C++ APIs.

For all four OpenCL program build scenarios an OpenCL Context and vector of
Devices will be required.  The code to create these items is invariant across
the four build scenarios and is included here once as a reference.  ::

    Context             context(CL_DEVICE_TYPE_ACCELERATOR);
    std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

Create an OpenCL program from source, with embedded source 
-----------------------------------------------------------

This is the most common method used OpenCL examples downloaded from the web.  
It has the benefit of portability, because the compilation of the OpenCL C code is
completely encapsulated in the OpenCL API calls.  Also, since the OpenCL C code
i.e. embedded in the application, the resultant executable is standalone in that
there is no dependency on another file containing OpenCL C code.

.. code-block:: cpp
  :linenos:

    const char * kernStr = "kernel void devset(global char* buf) {}"

    Program::Sources    source(1, std::make_pair(kernStr, strlen(kernStr)));
    Program             program = Program(context, source);
    program.build(devices);

Line 1 defines an embedded string representing an OpenCL C program.  This
example shows an empty kernel for brevity, but they can be arbitrarily long.
Line 3 defines a Program::Sources object.  In this case, there is one source
and it requires a pointer to the source and its length as a pair.  Line 4
creates the Program object.  The Program object will contain a copy of the
sources and the original string containing the sources could be destroyed or
reused at this point.  
Line 5 compiles the Program source for all devices in
the devices vector.  Once build has been called, Kernel objects can be created
for any function in the OpenCL C code adorned with the kernel keyword.  These
kernels can then be repeatedly en-queued to any device in the devices vector
until the Program object is deleted. Note that the Program object will be
deleted when it goes out of scope.

Create an OpenCL program from source, with source in a file
-----------------------------------------------------------

This build scenario also builds a program from source and will incur an on-line
compilation of the source.  It only differs from the previous build scenario in
regard to where the OpenCL C code exists.  In this case the OpenCL C code
exists in a separate file, named kernel.cl in the same directory in which the
application is run.  This OpenCL program build option still requires no
knowledge of standalone OpenCL C program compilation and is therefore still
portable.  Since the OpenCL C code is in a file, however, this build scenario
can be useful when developing OpenCL kernels.  Because the OpenCL C
file is read and compiled for the devices at host application run time, the
OpenCL C code can be edited and re-run without requiring the host application
to be re-compiled.

.. code-block:: cpp
  :linenos:

    ifstream t("./kernel.cl");
    if (!t) { cout << "Error Opening Kernel Source file\n"; exit(-1); }

    std::string kSrc((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
    Program::Sources source(1, make_pair(kSrc.c_str(), kSrc.length()));
    Program          program = Program(context, source);
    program.build(devices);

Lines 1 and 2 open a stream to the file kernel.cl in the current directory.
Line 4 creates a C++ string KSrc and reads the contents of the kernel.cl file
into KSrc. Line 5 creates a Program::Sources object with a pointer to the
source and the length of the source code.  Lines 6 and 7 are the same as the
previous build scenario.

Create an OpenCL program from binary, with binary in a file
-----------------------------------------------------------

For this build scenario, off-line compilation is used to create a binary file
from an OpenCL C source file.  An off-line compiler called :command:`clocl` is
shipped with the TI OpenCL product. To take a file named kernel.cl containing
OpenCL C source and create a binary called kernel.out, simply invoke clocl with
the input file name: :command:`clocl kernel.cl`.  This step would typically be
part of a makefile and is completed at application build time.  Refer to the
section on clocl for more details on clocl options.  This build scenario
protects IP better than the on-line compilation models, since the OpenCL C
source is not required for running the application, only for building the
application.  It also results in faster runtime, since the time delay for
on-line compilation will not be experienced.  It does, however, expose
implementation specific details of off-line compilation and therefore impacts
portability. 

.. code-block:: cpp
  :linenos:

    #include "ocl_util.h"

    char *bin;
    int bin_length = ocl_read_binary("./kernel.out", bin);

    Program::Binaries   binary(1, std::make_pair(bin, bin_length));
    Program             program = Program(context, devices, binary);
    program.build(devices);

    delete [] bin;

Line 4 calls a helper function ocl_read_binary to read a binary file into a
char array.  This helper function is provided with the TI OpenCL product.  To
use this function, you must include ocl_util.h to see the function prototype.
This can bee seen in line 1 of the above example.  Also, you will additionally
need to link the host application with the library containing the helper
function. Add -locl_util as a linker option.

Line 3 defines a pointer to a char array.  The ocl_read_binary function will
inspect the specified file to determine the number of bytes to allocate, it
will allocate the bytes, and it will read the contents of the file into those
bytes.  After a Program object is created using the binary data, the bytes
allocated by ocl_read_binary should be deleted.  This can be seen on line 10
of the example code.  

Line 6 creates a Program:Binaries object.  It requires a pair consisting of a
pointer to the binary data and a length of the data.  This example illustrates
one device and one binary, but it is possible to specify multiple devices and
multiple binaries. Line 6 creates a Program object from the binaries.  Note
that creating a program from binaries requires an additional argument, a vector
of devices.  This allows the application to create a program for a subset of
devices.  Perhaps an OpenCL context contains 3 devices, but the application
only has a binary for one of the devices.  In that case, the vector of devices
passed to the Program object constructor should contain just the one device.
The binaries vector should then correspondingly have one pair representing the
pointer and length of the binary data for that device. 

For reference, the implementation of the ocl_read_binary function is given
below.

.. code-block:: cpp
  :linenos:


    #include <iostream>
    #include <fstream>

    int ocl_read_binary(const char *filename, char* &buffer)
    {
        try
        {
            std::ifstream is;
            is.open (filename, std::ios::binary );
            is.seekg (0, std::ios::end);
            int length = is.tellg();
            is.seekg (0, std::ios::beg);
            buffer = new char [length];
            is.read (buffer, length);
            is.close();
            return length;
        }
        catch(...) { std::cout << "Binary read function failure" << std::endl; }
    }

.. _offline-embedded:

Create an OpenCL program from binary, with embedded binary
-----------------------------------------------------------

For this OpenCL program build scenario, off-line compilation is again used, but
an option is given to the off-line compiler :command:`clocl` to instruct it to
create a text based file that can be used as a header file rather than a binary
out file.  The text file is simply the binary data in an initialized char
array. Invoking clocl like this: :command:`clocl -t kernel.cl` will compile
``kernel.cl`` into ``kernel.out`` and the create ``kernel.dsp_h`` that will be
a file containing the initialized array ``kernel_dsp_bin`` which can be used
directly to create an OpenCL Program::Binaries object.  This build method is
the fastest of the four because neither on-line compilation nor reading the file
are required.

.. code-block:: cpp
  :linenos:

    #include "kernel.dsp_h"

    Program::Binaries binary(1, make_pair(kernel_dsp_bin,sizeof(kernel_dsp_bin)));
    Program           program = Program(context, devices, binary);
    program.build(devices);

Line 1 includes the file created by :command:`clocl -t`. Line 2 creates the
``Program::Binaries`` object from the array defined in ``kernel.dsp_h``.  Line 4
creates the OpenCL C program from the binary and Line 5 builds the program.

An example of the first few lines of a kernel.dsp_h file are provided below for
illustration purposes.  ::

    unsigned int kernel_dsp_bin_len = 3656;
    char kernel_dsp_bin[] = { 0x7f
    , 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    , 0x00, 0x00, 0x02, 0x00, 0x8c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    ...

Caching on-line compilation results
=======================================================

On-line compilation of OpenCL C requires invocation of a compiler for the
devices specified in the build API call.  These compiles will entail some delay
and in some cases can result in significant delay.  The TI OpenCL
implementation does provide a mechanism where the result of an on-line compile
can be cached on the system and the time delay for compilation is paid once for
the first invocation of a compile but subsequent invocations are short
circuited and the cached result is used instead.  This behavior is controlled
through the environment variable :envvar:`TI_OCL_CACHE_KERNELS`. 


The TI off-line OpenCL C compiler: clocl
=======================================================

Executing :command:`clocl -h` will print the help screen.  Clocl
contains two sets of options to control behavior. The first set of options is
clocl and TI OpenCL specific.  They include the option -t which is used to
generate an embeddable OpenCL C program binary array.  The second set of
options are generic OpenCL options as specified by the OpenCL 1.1
specification.  I refer the reader to the specification for more details on
those options.  

    Usage: clocl [options] <OpenCL C file> [<link files>]

    Options passed to clocl are either options to control
    clocl behavior or they are documented OpenCL 1.1 build
    options.

    The clocl behavior options are:
       =============== =========================================
       -h, --help      Print this help screen
       -v, --verbose   Print verbose messages
       -k, --keep      Do not delete temp compilation files
       -g, --debug     Generate debug symbols
       -t, --txt       Generate object in header form
       -l, --lib       Do not link. Stop after compilation
       -a              Allow kernel buffer arguments to overlap
       --version       Print OpenCL product
       =============== =========================================

    The OpenCL 1.1 build options. Refer to 1.1 spec for desc:
       ===============================  ========================================================
       -D<name>                         Create a preprocessor symbol <name>
       -D<name>=<val>                   Assign <val> to preprocessor symbol <name>
       -I<dir>                          Add <dir> to the list of paths to search for headers
       -w                               Inhibit all warning messages
       -Werror                          Make all warnings into errors
       -cl-single-precision-constant    Treat double FP constant as single FP constant
       -cl-denorms-are-zero             Enable flush to zero FP behavior
       -cl-opt-disable                  Disables all optimizations
       -cl-mad-enable                   Allow a * b + c to be replaced by a mad
       -cl-no-signed-zeros              Allow opts for FP math that ignore sign of zero
       -cl-unsafe-math-optimizations    Allow opts for FP math that may violate standards
       -cl-finite-math-only             Allow opts for FP math that assumes operands are finite
       -cl-fast-relaxed-math            Choose fast FP operations over compliant FP operations
       -cl-std=<val>                    Determine the OpenCL C language version to use 
       ===============================  ========================================================
