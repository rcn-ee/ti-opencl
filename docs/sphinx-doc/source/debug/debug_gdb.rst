****************************
Debug with gdb
****************************

Host side gdb
======================
You can debug your host side OpenCL application with gdb the same way as
you debug other host side applications.  You will need to use flag "-g" 
during compilation.  You can refer to `gdb documentation`_ for details.

If gdb does not come pre-installed on your host file system, you will need
to download a package and install it, or you will need to download `gdb
release`_, build and install on your file system.

.. _gdb documentation: http://www.gnu.org/software/gdb/documentation/
.. _gdb release: http://www.gnu.org/software/gdb/download/


DSP side debug with host side client gdbc6x
===========================================
DSP side kernel code can be debugged with hosted debugger, gdbc6x.  The
process to debug kernel code in an OpenCL application is as follows.  You will
need two windows/consoles, one window to run OpenCL application, the other
to debug DSP side kernel.

1. In window 1, set environment variable ``TI_OCL_DEBUG`` before running
   application, for example, ``TI_OCL_DEBUG=1 ./your_ocl_app`` if you use bash
2. Once the application is running, before launching your kernel to DSP,
   OpenCL runtime will print out a gdbc6x command in window 1, for example,
   gdbc6x -q -iex "target remote /dev/gdbtty0" -iex "set confirm off" -iex "symbol-file /usr/share/ti/opencl/dsp.out" -iex "add-symbol-file /tmp/opencl7mNBld.out 0x86000000" -iex "b exit" -iex "b VectorAdd"
3. Copy and paste the gdbc6x command into window 2, run it
4. Hit any key in window 1
5. Start debugging the kernel in window 2

The following are the sample output of window 1::

    root@am57xx-evm:~/oclexamples/vecadd# TI_OCL_DEBUG=1 ./vecadd
    DEVICE: TI Multicore C66 DSP
    
    Offloading vector addition of 8192K elements...
    
    gdbc6x -q -iex "target remote /dev/gdbtty0" -iex "set confirm off" -iex "symbol-file /usr/share/ti/opencl/dsp.out" -iex "add-symbol-file /tmp/openclXmObdu.out 0x86000000" -iex "b exit" -iex "b VectorAdd" 
    Press any key, then enter to continue
    c
    Kernel Exec : Queue  to Submit: 4 us
    Kernel Exec : Submit to Start : 45 us
    Kernel Exec : Start  to End   : 83717229 us
    
    Success!

and window 2::

    root@am57xx-evm:~# gdbc6x -q -iex "target remote /dev/gdbtty0" -iex "set confirm off" -iex "symbol-file /usr/share/ti/opencl/dsp.out" -iex "add-symbol-file /tmp/openclXmObdu.out 0x86000000" -iex "b exit" -iex "b VectorAdd" 
    Remote debugging using /dev/gdbtty0
    0xfeabec64 in ?? ()
    Reading symbols from /usr/share/ti/opencl/dsp.out...done.
    add symbol table from file "/tmp/openclXmObdu.out" at
    	.text_addr = 0x86000000
    Reading symbols from /tmp/openclXmObdu.out...done.
    Breakpoint 1 at 0xfea53254: file exit.c, line 64.
    Breakpoint 2 at 0x8600000c: file /tmp/openclXmObdu.cl, line 4.
    (gdb) continue
    Continuing.
    
    Breakpoint 2, VectorAdd () at /tmp/openclXmObdu.cl:4
    4	{
    (gdb) list
    1	kernel void VectorAdd(global const short4* a, 
    2	                      global const short4* b, 
    3	                      global short4* c) 
    4	{
    5	    int id = get_global_id(0);
    6	    c[id] = a[id] + b[id];
    7	}
    (gdb) break 6
    Breakpoint 3 at 0x8600008a: file /tmp/openclXmObdu.cl, line 6.
    (gdb) cont
    Continuing.
    
    Breakpoint 3, $C$L6 () at /tmp/openclXmObdu.cl:6
    6	    c[id] = a[id] + b[id];
    (gdb) print a[0]
    $1 = {0, 4, 8, 12}
    (gdb) print b[0]
    $2 = {0, 4, 8, 12}
    (gdb) print c[0]
    $3 = {0, 0, 0, 0}
    (gdb) next
    7	}
    (gdb) print c[0]
    $4 = {0, 8, 16, 24}
    (gdb) info locals
    dim = 0
    dim = 0
    a = 0x80000000
    b = 0x82000000
    c = 0x84000000
    id = 0
    (gdb) delete 3
    (gdb) delete 2
    (gdb) cont
    Continuing.
    ^C
    Program received signal SIGTRAP, Trace/breakpoint trap.
    0xfea7ec04 in $C$RL54 ()
        at /home/gtbldadm/processor-sdk-linux-daisy-build/build-CORTEX_1/arago-tmp-external-linaro-toolchain/sysroots/am57xx-evm/usr/share/ti/ti-sysbios-tree/packages/ti/sysbios/knl/Idle.c:72
    72	/home/gtbldadm/processor-sdk-linux-daisy-build/build-CORTEX_1/arago-tmp-external-linaro-toolchain/sysroots/am57xx-evm/usr/share/ti/ti-sysbios-tree/packages/ti/sysbios/knl/Idle.c: No such file or directory.
    (gdb) quit
    Detaching from program: , Remote target
    Ending remote debugging.
    root@am57xx-evm:~# 

