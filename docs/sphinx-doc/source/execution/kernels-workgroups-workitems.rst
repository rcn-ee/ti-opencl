*****************************************************************
Understanding Kernels, Work-groups and Work-items
*****************************************************************

In order to best structure your OpenCL code for fast execution, a clear
understanding of OpenCL C kernels, work-groups, work-items, explicit
iteration in kernels and the relationship between these concepts is imperative. 

Additionally, knowing how these concepts map to an underlying hardware 
architecture is important, so that your application may be structured 
appropriately.

Knowledge of these concepts is not required in order to run an OpenCL
application.  OpenCL code is expression portable, meaning that a conformant
OpenCL application should run correctly on any conformant OpenCL
implementation. However, knowledge of these concepts is required in order to
run an OpenCL application efficiently, i.e. OpenCL applications are not
necessarily performance portable, and simple tweaks to the balance of explicit
kernel iteration versus the number of work-items in a work-group can have a
very large performance impact.

Enqueueing a Kernel
********************

There are two OpenCL APIs for enqueueing a kernel.

#. enqueueNDRangeKernel(), and
#. enqueueTask().

enqueueTask is just a special case of enqueueNDRangeKernel where the offset,
global size, and local size are fixed to 0, 1, and 1 respectively in a single
dimension.  This special case is very useful in many cases, but for the
purposes of explaining the relationship of OpenCL C kernels, work-items and
work-groups, this section will focus on the general API enqueueNDRangeKernel.  
This section will also assume the offset for the enqueueNDRangeKernel is 0 is
all dimensions.  The offset is useful in limited use cases relative to the
global size and local size, and we refer you to the `OpenCL 1.1 specification
<http://www.khronos.org/registry/cl/specs/opencl-1.1.pdf>`_ for more information
on the use of offset.

The expression of an OpenCL C kernel is really an algorithm expression for one
work-item. That one work-item expression is also associated with a kernel name.
When an enqueueNDRangeKernel API call is made, the key three arguments to the
API are :

#. the kernel object which has previously been associated with the kernel name, and
#. the number of work-items you wish to execute (called the global size), and
#. the number of work-items you wish to group into a work-group (called the local size).

For example, the following C++ code ::

    Q.enqueueNDRangeKernel(K, NullRange, NDRange(1024), NDRange(128));

Illustrates an enqueueNDRangeKernel command that will enqueue to the OpenCL
queue named **Q** a kernel object named **K**.  

The second argument is the offset and as previously stated we will assume it is
0 in all dimensions.  The NullRange object will satisfy that 0 specification.  

The third argument is the global size and it specifies a wish to execute 1024
instances of the work-item specified in the kernel source associated with the
kernel object **K**. 

The fourth argument is the local size and it specifies how many of the
work-items should be grouped into a work-group.  In this case, it is specified
to be 128 work-items per work-group. 

Since there are 1024 total work-items and 128 work-items / work-group, a simple
division of 1024 / 128 = 8 work-groups.

#. The global size (GSZ) is the total number of work-items (WI)
#. The local size (LSZ) is the number of work-items per work-group (WI/WG)
#. The number of work-groups is the global size / local size, or GSZ/LSZ, or WG

It is also possible for the global size and local size to be specified in 2 or
3 dimensions.  For example a 2D kernel enqueue may look like ::

    Q.enqueueNDRangeKernel(K, NullRange, NDRange(640, 480), NDRange(640, 1))

This enqueue specifies:

- A global size of 640 work-items in dimension 0 and 480 work-items in
  dimension 1, for a total of 640 * 480 = 307,200 total work-items (WI). 
- It also specifies the local size to be 640 WI/WG in dimension 0 and 1 WI/WG
  in dimension 1.
- This results in 640/640 = 1 work-group in dimension 0 and 480/1 work-groups
  in dimension 1. for a total of 480 work-groups (WG)

Mapping the OpenCL C work-item Built-in Functions
****************************************************

OpenCL C contains eight built-in functions that can be used in the algorithmic
expression of a kernel to query global size, local size, etc.  Those eight
functions are:

======================================= ==================================================================================
Function                                Property returned
======================================= ==================================================================================
uint get_work_dim()                     The number of dimensions 
size_t get_global_id(uint dimidx)       The ID of the current work-item [0,WI) in dimension dimidx
size_t get_global_size(uint dimidx)     The total number of work-items (WI) in dimension dimidx
size_t get_global_offset(uint dimidx)   The offset as specified in the enqueueNDRangeKernel API in dimension dimidx
size_t get_group_id(uint dimidx)        The ID of the current work-group [0, WG) in dimension dimidx
size_t get_local_id(uint dimidx)        The ID of the work-item within the work-group [0, WI/WG) in dimension dimidx
size_t get_local_size (uint dimidx)     The number of work-items per work-group = WI/WG in dimension dimidx
size_t get_num_groups(uint dimidx)      The total number of work-groups (WG) in dimension dimidx
======================================= ==================================================================================


OpenCL C Kernel Code
**********************
The code in an OpenCL C kernel represents the algorithm to be applied to a
single work-item. The granularity of a work item is determined by the
implementer.  If we take an element wise vector add example, where we take two
1 dimensional vectors as input, add them together element wise and write the
result back into the first vector, we can express a kernel to achieve this
behavior  with either of the following ::

    kernel vectorAdd(global int* A, global const int * B)
    {
        int gid = get_global_id(0);
        A[gid] += B[gid];
    }

Next example ::

    #define ITER 16

    kernel vectorAdd(global int* A, global const int * B)
    {
        int gid = get_global_id(0) * ITER;
        int i;
        for (i = 0; i < ITER; ++i)
        {
            A[gid + i] += B[gid+ i];
        }
    }


.. How WG, WI/WG and ITER map to GPU
.. ***********************************

.. How WG, WI/WG and ITER map to DSP
.. ***********************************

.. The DSP transformation, turning WI/WG to ITER
.. **********************************************


