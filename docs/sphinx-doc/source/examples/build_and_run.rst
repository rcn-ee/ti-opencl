********************
Building and Running
********************

There are several OpenCL examples that are part of the OpenCL package
installation. They are located in the /usr/share/ti/examples/opencl[+openmp]
directories on the target file system.

The examples can be cross-compiled in an X86 development environment, or
compiled native on the ARM A15, depending on the availability of native g++ or
cross-compiled arm-linux-gnueabihf-g++ tool sets.

On an X86 development environment the example makefiles are setup to
cross-compile by default and assume an ARM cross-compile environment has been
installed. If the cross compiler is not installed, execute the following
command to install it:

    :command:`sudo apt-get install g++-4.6-arm-linux-gnueabihf`


All the examples can be built at one time by invoking 'make' from the top-level
directory where the OpenCL examples are installed/copied to. Individual
examples can be built by navigating to the desired directory and also issuing
'make'.
