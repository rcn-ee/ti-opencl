****************************
Terminology
****************************

The definitions of the below terms are copied or are modified forms of the
definitions from chapter 2 of the `OpenCL 1.1 specification
<http://www.khronos.org/registry/cl/specs/opencl-1.1.pdf>`_.  

.. glossary::

    Compute Device
       Compute devices are the target of computation to be off-loaded from the
       host CPU.  Command-queues are created in OpenCL applications and are
       tied to a specific compute device.  Internally, a compute device is a
       collection of compute units.  OpenCL compute devices typically
       correspond to a GPU, a multi-core CPU, or multi-core DSP.

    Compute Unit 
       A compute device contains one or more compute units.  For multi-core
       devices, a compute unit often corresponds to one of the cores.  A
       work-group executes on a single compute unit and multiple work-groups
       can execute concurrently on multiple compute units within a device.
       A compute unit will have local memory that is accessible only by the
       compute unit.

    Command Queue
      An object created by OpenCL APIs in the host application.  A
      command-queue is created for a specific device in a context. Command
      queues hold commands that will be executed on that specific device.
      Commands to a commandqueue are queued in-order but may be executed
      in-order or out-of-order depending on the attributes specified during the
      command queue's creation.  A compute device may have many command queues
      associated with it, but a command queue will only associate with one
      compute device.

    Kernel
      A kernel is a function declared in an OpenCL C program and executed on an
      Compute Device. A kernel is identified by the __kernel or kernel qualifier. 
      Kernels are enqueued to compute devices through command queues.

    Work-item
      When a kernel is enqueued to a command queue, the enqueue command
      specifies the number of work-items to be completed.  For
      enqueueNDRangeKernel, the number of work-items is explicitly specified by
      the global size argument.  For enqueueTask, the number of work-items is
      implicitly specified as 1.

      One of a collection of parallel executions of a kernel invoked on a
      device by a command. A work-item is executed by one or more processing
      elements as part of a work-group executing on a compute unit. A work-item
      is distinguished from other executions within the collection by its
      global ID and local ID.

    Work-group
      A collection of related work-items that execute on a single compute unit.
      The work-items in the group execute the same kernel and share local
      memory.

    Global ID
      A global ID is used to uniquely identify a work-item and is derived from
      the number of global work-items specified when executing a kernel. The
      global ID is an N-dimensional value that starts at 0 in all dimensions.

    Local ID
      A local ID specifies a unique work-item ID within a given work-group that
      is executing a kernel. The local ID is an N-dimensional value that starts
      at 0 in all dimensions.
