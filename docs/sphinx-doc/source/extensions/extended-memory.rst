********************************************
Using Extended Memory on the 66AK2x device
********************************************

An extended memory feature is supported on the 66AK2H implementation
of OpenCL. The C66x DSP is a 32-bit architecture and has a limit of
2GB of DDR that it can access at any given time. The 66AK2H platforms
can support up to 8GB of DDR3. To enable usage of DDRs greater than
2GB, this OpenCL implementation can use a hardware mapping feature to
move windows over the 8GB DDR into the 32-bit DSP address space.
Movement of these windows will occur at kernel start boundaries so
two sequential kernels dispatched to the DSP device may actually
operate on different 2GB areas within the 8GB DDR. The windows are
not moved within a kernel. As a result of this feature, large buffers
may be created and subsequently populated on the ARM side. However, a
dispatched kernel may not access the entire buffer in one dispatch.
Any given OpenCL Kernel will be limited to a total of 2GB of DDR
access. The :ref:`vecadd_mpax-example`  illustrates a
process of defining a large buffer and then defining sub-buffers
within the larger buffer and dispatching multiple OpenCL kernels to
the DSP on these sub-buffers, cumulatively resulting in the entire
large buffer being processed. Also see :doc:`../memory/extended-memory`
