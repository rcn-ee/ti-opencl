********************************************************************
Large OpenCL buffers and Memory Beyond the 32-bit DSP Address Space
********************************************************************

.. Note::

    This information is only applicable to platforms with > 2GB DDR. The
    66AK2G OpenCL implementation does not support extended memory.

The 66AK2x devices will support up to 8GB of DDR3 on the DDR3A bus.  The C66
DSP, however is a 32-bit architecture and cannot access all 8GB at any given
time.  The C66 DSP does have a memory translation capability that will allow it
to access any portion of that memory, but there are constraints on the mapping
that will be described here.

The 8GB of DDR3 exists in the K2x 36-bit physical address space at addresses
8:0000:0000 to 9:FFFF:FFFF.  The K2x device boots with the C66x DSP's mapping
the upper 2GB of its address space 8000:0000 to FFFF:FFFF to the beginning of
that physical range. For the remainder of this section, the physical range from
8:0000:0000 to 8:7FFF:FFFF will be referred to as the low 2GB and the range
from 8:8000:0000 to 9:FFFF:FFFF will be referred to as the upper 6GB.

The figure below illustrates the mapping, using 512M blocks of memory.  The red
blocks are Linux system memory and the green blocks are either CMEM or reserved
memory.  See the section above on DDR partition for definitions of these memory
types. The actual use of the lower 2GB can vary based on memory usage boot
variables, but the use illustrated below is the default and is typical.

.. Image:: ../images/Extended_memory_overlay.png

If the entire upper 6GB of memory are configured as Linux system memory and are
therefore unavailable to OpenCL, then OpenCL will have 1488MB of memory in the
lower 2GB available for OpenCL C programs and Buffers and no further
constraints are necessary and the remainder of this section is not applicable.
Additionally, if the environment variable TI_OCL_DSP_NOMAP is set, then OpenCL
will ignore any CMEM region that is defined in the upper 6GB, and OpenCL
operation will be restricted to the lower 2GB and again the remainder of this
section is not applicable.

If there is memory in the upper 6GB that is given to CMEM to manage, then that
memory will be available to OpenCL as well and understanding how OpenCL will
use that memory is important so an application can maximize   resource
utilization.  The figure below illustrates a potential DDR partition with CMEM
in the upper 6GB.

.. Image:: ../images/Extended_memory_example.png

In the partition above, 1.5GB is partitioned to Linux System memory and 6.5GB
is partitioned for OpenCL use. Note that only the 512M block of memory from
A000:0000 to BFFF:FFFF is indicated as green for OpenCL.  The other 3 512M
blocks are blue indicating that they are available destinations for mapping
from alternate regions of the 36-bit address space.  The one green 512M block
will always be fixed to its corresponding location in physical memory and is
not available for mapping.

Within that 512M fixed block there is 80M of reserved memory and 432M of CMEM
OpenCL memory.  OpenCL will manage allocations using two heaps: a fixed heap
and a mapped heap.  The fixed heap is the 432M of OpenCL memory in the fixed
block of DSP memory from A000:0000 to BFFF:FFFF.  The mapped heap manages all
other OpenCL memory.  In reality, the mapped heap may be more than one heap in
the OpenCL implementation, if the additional OpenCL memory is not contiguous,
as is the case in the above example figure.  The number of actual heaps in the
virtual mapped heap is unimportant to the user, except that a single very
larger buffer may not span all the additional OpenCL memory if it is not
contiguous.

The OpenCL runtime will manage which heap is used for allocation using the
following algorithm:

1.  OpenCL C programs are always allocated from the fixed heap
2.  OpenCL C Buffers <= 16K bytes are allocated from the fixed heap, until it
    is full and then from the mapped heap.  
3.  OpenCL C Buffers <= 16M bytes are allocated from the fixed heap, 
    while fixed heap space available >= 64M, and then from the mapped heap.  
4.  OpenCL C Buffers > 16M are always allocated from the mapped heap.

The mapping of buffers from the mapped heap into the blue (mapping destination)
regions of the 32-bit C66 address space occur at OpenCL C kernel execution
boundaries. Immediately before the launch of a kernel, mapping will occur for
the buffer arguments to the kernel.  Immediately after the kernel completes the
mapping is returned to the default mode.  Mapping will not change during a
single execution of a kernel.  This execution model results in the some
constraints for any single kernel invocation. Most importantly, all buffer
arguments to an OpenCL C kernel that are allocated from the mapped heap, must
cumulatively be mappable to the blue mapping destination region of the 32-bit
C66 DSP address space.  The mapping destination region includes one 512M block
and one 1024M block, so this could support one 1GB buffer and one 512MB buffer,
or three 512M buffers, or two 512M buffers and four 128M buffers, etc.

Additionally, OpenCL can manage only 7 mapped regions and each mapped region
must be sized to a power of 2 and aligned to a power of 2.  this limitation of
7 mapped regions will limit the number of buffers (from the mapped heap) that
can be passed to a kernel.  Subject to the size limits from above, at least 7
such buffers are possible.  Given buffer sizes and locations, it may be
possible to map greater than 7 buffers to a single OpenCL C kernel, because the
OpenCL runtime may be able to map a larger region that covers multiple buffers.
For example, four 128M buffers that are consecutive in the mapped heap, where
the first is aligned to a 512M boundary, could potentially be mapped using only
1 of the 7 map regions.

To maximize OpenCL memory usage in the upper 6GB, and prevent fragmentation, it
is recommended that buffers larger than 16MB be:

1. sized to a power of 2.
2. allocated in decreasing size order.

Smaller buffers allocated in the fixed heap are not subject to the memory
mapping constraints and need not be power of 2 sized.

Large Buffer Use Cases
===================================

For the purposes of this section a requirement for 3 large OpenCL buffers will
be assumed.  From the operational discussion in the previous section, we can
deduce that three 512M buffers can be passed to an OpenCL C kernel.  If the
OpenCL host application requires buffers larger than 512M, for example, it
requires three 2GB buffers, it can allocate three 2GB buffers that are
populated by the host and sub-buffers (See sub-buffer section) can be used to
define 512M subsets of the larger buffers.  The host application can then
enqueue an OpenCL C kernel four times, once for each 512M sub-buffer section of
the larger buffer.

Alternatively, An OpenCL application could base its calculations on a 512M
buffer size and it could define multiple sets of buffers for a ping-pong buffer
implementation.  Set A of three 512M buffers are populated and a kernel is
enqueued to the DSP to process those buffers.  Concurrent with the processing
of buffer set A, and second set B of three buffers is being populated by the
host. Buffer processing and buffer population would then alternate on the two
buffer sets.

A third use case, involves using enqueueTask to enqueue kernels.  In this model
up to 8 independent kernels can be executing concurrently, one on each of the
C66 DSPs in the K2x device.  Each of these tasks can be operating on an
independent set of buffers.  In this case there would be eight sets of three
buffers and they would be limited to 256M each.  A combination of this approach
on ping-pong buffers would require 16 sets of three buffers each limited to
128M.

There are other use-cases for large buffers, but the above briefly describes
some of the common use cases.  The vecadd_mpax example shipped with OpenCL
provides a framework for the sub-buffer use-case.

TBD
