Optimization Techniques for Device (DSP) Code
********************************************* 

Prefer Kernels with 1 work-item per work-group
==============================================
For better performance, create work-groups with a single work-item and use iteration within the work-group. Kernels structured in this manner take advantage of the C66x DSP's ability to execute loops efficiently.


Use Local Buffers
=================
Local buffers cannot be used to directly communicate between host and device,
but they are very good for storing temporary intermediate values in device code.
On TI SoCs, local buffers are located in L2 SRAM memory, whereas global buffers
are located in DDR3 memory.  The access time to L2 is greater than 10x faster
than to DDR.  The impact of local rather than global is further magnified when
writing values.  For algorithms, where values are written to a
buffer, and the buffer is subsequently used by another kernel or by the CPU host, it is almost
always better to write the values to a local buffer and then copy that local
buffer back to a global buffer using the OpenCL async_work_group_copy
function.  

The below two kernels perform the same simple vector addition operation. The
difference is that the first reads two inputs from ddr and writes a result back
to ddr, where the second reads two inputs from ddr and writes a result to local
L2 and then uses an async_work_group_copy to bulk move the local buffer back to
the global buffer.  The second version is almost 3 times faster than the first
version.

The first version of vector addition ::

    kernel void VectorAdd(global const short4* a, 
                          global const short4* b, 
                          global short4* c) 
    {
        int id = get_global_id(0);
        c[id] = a[id] + b[id];
    }


The second version of vector addition, using local buffers ::

    kernel void VectorAdd(global const short4* a, 
                          global const short4* b, 
                          global short4* c, 
                          local  short4* temp) 
    {
        int id  = get_global_id(0);
        int lid = get_local_id(0);
        int lsz = get_local_size(0);

        temp[lid]  = a[id] + b[id];

        event_t ev = async_work_group_copy(&c[lsz*get_group_id(0)], temp, lsz, 0); 
        wait_group_events(1,&ev); 
    }


Use async_work_group_copy and async_work_group_strided_copy
===========================================================
The previous section illustrated the use of an async_work_group_copy call.
Both OpenCL built-in functions async_work_group_copy and 
async_work_group_strided_copy use a system DMA operation to perform the 
movement of data from one location to another.  There are several reasons 
why this can be beneficial:

    #. As the name implies the async... functions are asynchronous, meaning that
       the call initiates a data transfer but it does not wait for completion 
       before returning.  The subsequent wait_group_events call blocks until 
       the data transfer is complete.  This allows additional work to be 
       performed concurrent with the data transfer.

    #. DDR writes through the system DMA occur in optimal burst sizes, whereas 
       DSP writes to DDR memory do not, because the caches are set to write 
       through mode on the DSPs in order to avoid a false-sharing problem that 
       could result in incorrect results.


Avoid DSP writes to directly to DDR
===================================
See the previous two subsections.

Use the reqd_work_group_size attribute on kernels
=================================================
If you followed the Host optimization tip to "Prefer Kernels with 1
work-item per work-group", then you should annotate your kernel with the
**reqd_work_group_size** attribute to inform the OpenCL C compiler that the
kernel will only have the one work-item. This communicates information to the
OpenCL C compiler that it would otherwise not know and there are a number of
optimizations that are enabled based on this knowledge. An example using this
attribute would look like ::

    kernel __attribute__((reqd_work_group_size(1, 1, 1)))
    void k1wi(global int *p)
    {
       ...
    }

Even if the kernel does have > 1 work-item per work-group, this attribute is
useful to the OpenCL C compiler.  Of course, to use it you would be asserting
that the host code will enqueue this kernel with the same local size as the 
numbers you specify in the attribute. If the kernel is enqueued with a different 
local size than specified in the attribute, the runtime will give a well defined 
error. The below kernel is using the attribute to assert that dimension 1 has a
local size of 640 and dimension 2 has a local size of 480 and dimension three
is unused::

    kernel __attribute__((reqd_work_group_size(640, 480, 1)))
    void img_alg(global int *p)
    {
       ...
    }

Use the TI OpenCL extension than allows Standard C code to be called from OpenCL C code
==============================================================================================
Call existing, optimized, std C code library functions.
Or write your own standard C code.

Avoid OpenCL C Barriers
=======================
Avoid OpenCL C barriers if possible. Particularly prevent private data from being live across barriers.
barrier(), async...(), wait...()

Use the most efficient data type on the DSP
===========================================
Pick the most efficient data type for an application. E.g., if it is sufficient, prefer the 'char' type to represent a 8-bit data over using a 'float' type. This could have huge impact because:

  * More data transfer can happen
  * More compute can happen

As a rule of thumb, the C66x DSP can do a maximum of
  * 16 8-bit SIMD
  * 8 16-bit SIMD
  * 8 32-bit SIMD etc.,

It is observed that if 8-bit of storage is sufficient for a given application, more parallelism is obtained both in terms of *compute* and *data transfer* using the char vs. float.

Do Not Use Large Vector Types
=============================
Do not use vector types where the size of the vector type is > 64 bits. The C66x DSP has limited instruction support for long vector types, so their use is not performance beneficial.

Vector types with total size <= 64 bits may be beneficial, but the benefit is not guaranteed.

Consecutive memory accesses
===========================
Data Access pattern plays a key role in generating efficient codes. Consecutive memory access is the fastest way. Also, the data flow can happen in different data sizes like
 
1. Single Byte ld/st 
2. Half Word ld/st
3. Single Word ld/st
4. Double Word ld/st

The data flow rate is in ascending rate for the memory operations in the above list. 

.. note:: 
   data flow rate = No. of. bytes transferred / DSP cycle
   
   i.e. Single byte *ld* in 1 cycle > Half word *ld* in 1 cycle > Single word *ld* in 1 cycle > Double word *ld* in 1 cycle

It is most beneficial to use the Double Word ld/st as it has the highest data flow rate.

This could be exploited to transfer data in different packing granularities. Say double word ld can bring in data in various packing granularities like

* Single 64-bit data
* Two 32-bit data
* Four 16-bit data
* Eight 8-bit data

Depending on the nature of the application, different sizes of loading may be preferred. The main focus here is to try to achieve higher data flow rate.

For example:

  A mxn image is represented as a 1D array of type 'char'. This image is convolved with a Gaussian filter kernel. In order to utilize the SIMD operations as discussed previously, a vector length of 4 is chosen.

  In order to bring in the data effectively,

.. highlight:: c
   :linenothreshold: 5

.. code-block:: c
   :linenos:

   char* image;
   char4 r1_7654, r1_3210;

   r1_7654 = vload4(0, image);
   r1_3210 = vload4(4, image);


.. Native math operations vs. standard ones.
.. ----------------------------------------------------------------------------------------

.. Use the TI Std C intrinsics 
.. ----------------------------------------------------------------------------------------

.. Fixed point over floating point if possible
.. ----------------------------------------------------------------------------------------

.. Double buffer technique
.. ----------------------------------------------------------------------------------------

.. Low level DSP optimization
.. ----------------------------------------------------------------------------------------

Prefer the CPU style of writing OpenCL code over the GPU style
==============================================================
There is a large body of existing OpenCL code available and the majority have
been targeted toward and optimized for either GPUs or CPUs.  Often, an
application will have different kernels optimized for each.  Generally, the
versions targeting CPUs will perform better than the version targeting GPUs,
when executed on TI SoCs and using the DSP as a device.


