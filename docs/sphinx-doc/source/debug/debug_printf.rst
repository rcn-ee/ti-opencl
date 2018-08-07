****************************
Debug with printf
****************************

Though it has to be manually inserted into the code, a simple ``printf``
function can help you debug program progress, values of interested data
and so on.  You can use ``printf`` to debug your OpenCL application as well.

Host side OpenCL application code
=================================
Obviously, you can put ``printf`` in your host side OpenCL application code,
as long as your host compiler supports it (gcc does).

DSP side OpenCL kernel code
===========================
Though TI's OpenCL implementation is currently at most OpenCL version 1.1
conformant on the SoCs that we support, we do support this OpenCL version
1.2 feature, ``printf``, as described in section 6.12.13 in OpenCL v1.2
specification.  The output of ``printf`` from DSP side is redirected to the
host side, printed out in the stdout, for example, the Linux window/terminal
where you launch your OpenCL application.

Not only can you put ``printf`` in the OpenCL C kernel code, as described in
OpenCL version 1.2 specification, you can also put ``printf`` in the standard
C code that you link into the OpenCL C kernel, as shown in TI extension
(:doc:`../extensions/standard-c-code`).
They will all be printed out on the host side.  When using ``printf`` in
OpenCL C kernel, you do not need to include any header files, when using in
standard C code that gets linked in, you need to include ``stdio.h`` as you
normally do.

Note that in the format string of ``printf``, TI's implementation now supports
all features described in the OpenCL 1.2 specification. For example ``%v``
representing an OpenCL vector type is now supported. A known issue is the use
of ``printf("%s\n", "string");`` leads to a clocl/clang assertion failure. You
can avoid this by simply using ``printf("string\n");``

Note that the output of ``printf`` from a DSP kernel remains prepended with the
DSP core number by default. The OpenCL 1.2 specification does not require it.
Therefore a new environment variable, ``TI_OCL_PRINTF_COREID`` can now be used
to toggle between showing the DSP core number or not. If
``TI_OCL_PRINTF_COREID=0`` is used, the DSP core number will not be displayed.
