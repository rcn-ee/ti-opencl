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
host side, printed out in the stdout, for example, the linux window/terminal
where you launch your OpenCL application.

Not only can you put ``printf`` in the OpenCL C kernel code, as described in
OpenCL version 1.2 specification, you can also put ``printf`` in the standard
C code that you link into the OpenCL C kernel, as shown in TI extension
(:doc:`../extensions/standard-c-code`).
They will all be printed out on the host side.  When using ``printf`` in
OpenCL C kernel, you do not need to include any header files, when using in
standard C code that gets linked in, you need to include ``stdio.h`` as you
normally do.

Note that in the format string of ``printf``, TI's implementation does not
support all flag characters described in the OpenCL specification, for example,
``%v`` for vector.  But you can work around it by printing each individual
vector element out.  For example, instead of
``int2 v; ...; printf("v = <%v>\n", v);``,
you can write ``printf("v = <%d,%d>\n", v.s0, v.s1);``.

Output of ``printf`` from the DSP side is automatically prepended with the dsp
core number where the kernel code runs, for example, ``[core 0] v = <1,5>``.
Sometimes, knowing which core your kernel lands on can also help debugging.

