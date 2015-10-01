****************************
Debug with dsptop
****************************

**dsptop**, a TI utility that is similar to Linux utility **top**, can be used
to debug which DSP cores participate in computation, memory usage of OpenCL
buffers, and kernel activities with timestamps such as workgroup start,
workgroup complete, and cache operations.  

Like gdbc6x, use of dsptop requires two windows/consoles as well:
start dsptop in window 1 first, launch your OpenCL application in window 2
next.  Details about the usage of dsptop can be found by running
``dsptop -h`` and by this `dsptop wikipage`_ .

.. _dsptop wikipage: http://processors.wiki.ti.com/index.php/Dsptop


.. Note::
  The first time dsptop runs on AM57, the following message will be
  printed out in the console.  However, this message is harmless and won't
  impact functionality of dsptop and your application.  We are working on a fix
  to remove this message in the future releases.

::

    [  242.398541] ------------[ cut here ]------------
    [  242.403205] WARNING: CPU: 0 PID: 0 at drivers/bus/omap_l3_noc.c:147 l3_interrupt_handler+0x234/0x35c()
    [  242.412554] 44000000.ocp:L3 Custom Error: MASTER MPU TARGET L3_INSTR (Idle): Data Access in User mode during Functional access
    [  242.423992] Modules linked in: rpmsg_proto gdbserverproxy(O) cryptodev(O) cmemk(O) usb_f_ss_lb g_zero libcomposite configfs xhci_plat_hcd xhci_hcd rpmsg_rpc dwc3 virtio_rpmsg_bus ti_vip ti_vpe pixcir_i2c_ts videobuf2_dma_contig ti_vpdma v4l2_mem2mem videobuf2_memops videobuf2_core mt9t11x v4l2_common omapdrm_pvr(O) videodev btwilink media dwc3_omap omap_remoteproc remoteproc virtio debugss_kmodule(O) virtio_ring bluetooth 6lowpan_iphc
    [  242.462939] CPU: 0 PID: 0 Comm: swapper/0 Tainted: G           O 3.14.43-ge859996 #1
    [  242.470714] Backtrace:
    [  242.473189] [<c0011d20>] (dump_backtrace) from [<c0011ebc>] (show_stack+0x18/0x1c)
    [  242.480788]  r6:00000093 r5:00000009 r4:00000000 r3:00000000
    [  242.486511] [<c0011ea4>] (show_stack) from [<c05fc814>] (dump_stack+0x78/0x94)
    [  242.493768] [<c05fc79c>] (dump_stack) from [<c00380ac>] (warn_slowpath_common+0x6c/0x90)
    [  242.501891]  r4:c0889da8 r3:c0884558
    [  242.505498] [<c0038040>] (warn_slowpath_common) from [<c0038174>] (warn_slowpath_fmt+0x38/0x40)
    [  242.514230]  r8:c062b474 r7:c079d1f8 r6:c079d264 r5:80080003 r4:ec964a90
    [  242.521004] [<c0038140>] (warn_slowpath_fmt) from [<c02b62d4>] (l3_interrupt_handler+0x234/0x35c)
    [  242.529911]  r3:ec964d80 r2:c079d324
    [  242.533527] [<c02b60a0>] (l3_interrupt_handler) from [<c0074dc4>] (handle_irq_event_percpu+0x54/0x1a0)
    [  242.542869]  r10:ec930680 r9:c08de4bb r8:00000017 r7:00000000 r6:00000000 r5:ec9306dc
    [  242.550769]  r4:ec964700
    [  242.553323] [<c0074d70>] (handle_irq_event_percpu) from [<c0074f5c>] (handle_irq_event+0x4c/0x6c)
    [  242.562229]  r10:c060994c r9:c0888000 r8:00000001 r7:fa212000 r6:00000000 r5:ec9306dc
    [  242.570128]  r4:ec930680
    [  242.572683] [<c0074f10>] (handle_irq_event) from [<c0078060>] (handle_fasteoi_irq+0x84/0x150)
    [  242.581241]  r6:00000000 r5:00000017 r4:ec930680 r3:00000000
    [  242.586958] [<c0077fdc>] (handle_fasteoi_irq) from [<c00746fc>] (generic_handle_irq+0x28/0x38)
    [  242.595603]  r4:00000017 r3:c0077fdc
    [  242.599213] [<c00746d4>] (generic_handle_irq) from [<c000f0f8>] (handle_IRQ+0x40/0x9c)
    [  242.607159]  r4:c0884ea0 r3:000001c3
    [  242.610766] [<c000f0b8>] (handle_IRQ) from [<c0008520>] (gic_handle_irq+0x30/0x64)
    [  242.618364]  r6:c0889f40 r5:c0890978 r4:fa21200c r3:000000c0
    [  242.624080] [<c00084f0>] (gic_handle_irq) from [<c0601b80>] (__irq_svc+0x40/0x50)
    [  242.631593] Exception stack(0xc0889f40 to 0xc0889f88)
    [  242.636667] 9f40: ffffffed 2c6d5000 c0891264 c00288d8 c089051c c08904b8 c08de4b9 c08de4b9
    [  242.644879] 9f60: 00000001 c0888000 c060994c c0889f94 c0889f88 c0889f88 c002851c c000f47c
    [  242.653089] 9f80: a00f0013 ffffffff
    [  242.656588]  r7:c0889f74 r6:ffffffff r5:a00f0013 r4:c000f47c
    [  242.662307] [<c000f44c>] (arch_cpu_idle) from [<c0074418>] (cpu_startup_entry+0x68/0x138)
    [  242.670523] [<c00743b0>] (cpu_startup_entry) from [<c05f7488>] (rest_init+0x68/0x80)
    [  242.678296]  r7:edfff680 r3:00000000
    [  242.681906] [<c05f7420>] (rest_init) from [<c0834b4c>] (start_kernel+0x318/0x374)
    [  242.689426] [<c0834834>] (start_kernel) from [<80008074>] (0x80008074)
    [  242.695980] ---[ end trace a6b5c2bd38a1acd7 ]---

