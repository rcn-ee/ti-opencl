******************************************************
Buffer Read/Write vs. Map/Unmap
******************************************************

The OpenCL APIs support two mechanisms for the host application to interact
with OpenCL buffers. They can:

- Read and write buffers using clEnqueueReadBuffer and clEnqueueWriteBuffer in
  C or the member functions enqueueReadBuffer and enqueueWriteBuffer in C++, or
- Map and unmap using clEnqueueMapBuffer and clEnqueueUnmapMemObject in C.

The read and write APIs imply a movement of data to and from OpenCL buffers.
This typically means a movement of data from Linux system memory to CMEM memory
where an OpenCL buffer typically resides.

The map/unmap APIs map the underlying memory store of a buffer into the host
address space and allows the host application to read and write directly
from/to the buffer's content.  This method has the advantages of:

1. Not requiring 2 storage areas containing the same data (one in Linux system
   memory and one in the Buffer in CMEM memory), and 
2. Not requiring extra data movement between the two storage areas.

There are situations where the read/write buffer is preferable, however.  For
smaller buffers, the overhead of the extra copies is small and the extra
commands enqueued to the CommandQueue do have some overhead.  In the below
examples of read/write and map/unmap use, you can see that there are 4 commands
enqueued for data movement in the map/unmap case and there are only two
commands enqueued for data movement in the read/write case.

The map/unmap commands will perform cache coherency operations and do entail
some cost.  The read and write buffer commands currently use memcpy for data
transfer.  

For the examples below, please refer to the OpenCL 1.1 specification or online
reference pages for the details of the APIs. In these examples, most of the
arguments to the read/write or map/unmap enqueue commands are obvious with the
exception of CL_TRUE as the second argument and 0 as the third argument to
read/write and fourth argument to map/unmap.  The CL_TRUE argument indicates to
the OpenCL runtime that you would like this enqueue command to block  until the
operation is complete. OpenCL enqueue commands are typically asynchronous, the
command is enqueued and the main thread continues execution in parallel with
the operations enqueued.  If these APIs are passed CL_FALSE as a second
argument, then they behave asynchronously as well.  

The 0 argument is an offset into the buffer being read, written, mapped or
unmapped.

The below code fragment illustrates a write/read buffer use case using the C++
OpenCL Binding. An OpenCL context ctx, CommandQueue Q and Kernel K are already
created and bufsize represents the number of bytes in the buffers. Note that
bufsize bytes are allocated in the Linux heap for the array ary and an
additional bufsize bytes are allocated in CMEM for the buffer buf.  This double
allocation is clearly a limitation if bufsize is particularly large.  If it is
not large, then an application can double buffer using this approach.  After
the buffer write, the memory pointed to by ary can be repopulated and a
pipeline can be established. Obviously, in that use case, the example would
need some modification to not reuse ary for the read buffer.::

    int *ary = (int*) malloc(bufsize);

    // populate ary

    Buffer buf (ctx, CL_MEM_READ_WRITE, bufsize);
    K.setArg(0, buf);

    Q.enqueueWriteBuffer(buf, CL_TRUE, 0, bufsize, ary);
    Q.enqueueTask(K);
    Q.enqueueReadBuffer (buf, CL_TRUE, 0, bufsize, ary);

    // consume ary

The below code fragment illustrates a map/unmap buffer use case using the C++
OpenCL Binding. ::

    Buffer buf (ctx, CL_MEM_READ_WRITE, bufsize);
    K.setArg(0, buf);

    int * ary = (int*)Q.enqueueMapBuffer(buf, CL_TRUE, CL_MAP_WRITE, 0, bufsize);
    // populate ary
    Q.enqueueUnmapMemObject(buf, ary);

    Q.enqueueTask(K);

    ary = (int*)Q.enqueueMapBuffer(buf, CL_TRUE, CL_MAP_READ, 0, bufsize);
    // consume ary
    Q.enqueueUnmapMemObject(buf, ary);
