=======================
Offloading using OpenCL
=======================

This chapter describes a sequence of steps to offload computation from ARM/Linux to the DSPs on TI's heterogeneous multicore devices such as AM572_, AM571_ and 66AK2H_. For details on offloading from ARM/TI-RTOS, see :doc:`rtos/index`.

.. note::

  This matrix multiplication example is only meant to illustrate the steps required to offload using OpenCL. It has not been optimized for performance. Refer section :doc:`optimization/index` and :ref:`sgemm-example` for details on optimizing matrix multiplication and other OpenCL kernels for the DSP.

  Also, this example uses the `OpenCL 1.1 C++ Wrapper API`_ for conciseness. 


.. contents:: :local:


Matrix multiply on the ARM host
===============================
Listing :ref:`matmul-arm` performs matrix multiplication, ``C[N][M] = A[N][K] x B[K][M]``. It can be compiled and run on the host using the command: ``g++ -std=c++11 matmul_arm.cpp -o matmul``


.. literalinclude:: listings/matmul_arm.cpp
    :language: cpp
    :caption: matmul_arm.cpp
    :name: matmul-arm


Offloading matrix multiplication to the DSPs
============================================

Steps
+++++

Represent matrix multiplication as an OpenCL-C kernel
-----------------------------------------------------
Listing :ref:`matmul-ocl-kernel` illustrates a way of representing matrix multiplication as an OpenCL-C kernel. In this approach, each workgroup computes one row of the output C matrix. The number of workgroups is equal to the number of columns in the C matrix, ``M``. For details on how workgroups are mapped to the DSPs, refer :doc:`execution/kernels-workgroups-workitems`.

.. literalinclude:: listings/matmul_ocl.cpp
    :language: cpp
    :caption: OpenCL-C kernel for matrix multiplication
    :name: matmul-ocl-kernel
    :lines: 23-40

Allocate matrices using __malloc_<memory> functions
---------------------------------------------------
The DSPs operate out of contiguous memory. In order to avoid copies from non-contiguous host memory to contiguous DSP memory, allocate the matrices using a special contiguous allocator. Refer :doc:`memory/host-malloc-extension` for details.

.. literalinclude:: listings/matmul_ocl.cpp
    :language: cpp
    :caption: Allocate contiguous memory for matrices
    :name: matmul-ocl-alloc
    :lines: 91-94

Initialize the OpenCL runtime
-----------------------------
Boiler-plate code to create an OpenCL context. The DSPs are modeled as a single  OpenCL device of type CL_DEVICE_TYPE_ACCELERATOR. TI's OpenCL runtime on AM57x and 66AK2x SoCs supports a single device. Refer :doc:`execution/index` for details.

.. literalinclude:: listings/matmul_ocl.cpp
    :language: cpp
    :caption: Create an OpenCL context, device and queue
    :name: matmul-ocl-context
    :lines: 48-51

Compile the OpenCL-C kernel
---------------------------
Use OpenCL APIs to compile the OpenCL-C kernel for the DSP. This example uses online compilation. For an overview of the various compilation modes available, refer :doc:`compilation`.

.. literalinclude:: listings/matmul_ocl.cpp
    :language: cpp
    :caption: Online compilation of the OpenCL-C kernel
    :name: matmul-ocl-compile
    :lines: 53-57

Create a kernel object, set up arguments
----------------------------------------
Listing :ref:`matmul-ocl-kernel-obj` creates an OpenCL Kernel object and sets up the kernel arguments.

.. literalinclude:: listings/matmul_ocl.cpp
    :language: cpp
    :caption: Kernel object
    :name: matmul-ocl-kernel-obj
    :lines: 64-70

Call the kernel and wait for completion
---------------------------------------
The execution of the kernel is asynchronous. The host can perform computations that do not depend on the output matrix, ``C``, between the ``enqueueNDRangeKernel`` and ``wait``.

.. literalinclude:: listings/matmul_ocl.cpp
    :language: cpp
    :caption: Call the OpenCL-C kernel
    :name: matmul-ocl-invoke
    :lines: 72-75


Putting it all together
+++++++++++++++++++++++
Compile the host program with OpenCL offload using the following command: ``g++  -O3 -std=c++11 matmul_ocl.cpp  -lOpenCL -locl_util -o matmpy``. ``libOpenCL.so`` is TI's OpenCL Runtime library. ``libocl_util.so`` provides utility functions such as ``ocl_code_error``.

.. literalinclude:: listings/matmul_ocl.cpp
    :language: cpp
    :caption: matmul_ocl.cpp
    :name: matmul-ocl



.. Diff
.. ====
.. .. literalinclude:: matmul_arm.cpp
..    :diff: matmul_ocl.cpp


.. _AM572:              http://www.ti.com/product/AM5728
.. _AM571:              http://www.ti.com/product/AM5718
.. _66AK2H:             http://www.ti.com/product/66ak2h14
.. _OpenCL 1.1 C++ Wrapper API: http://www.khronos.org/registry/cl/specs/opencl-cplusplus-1.1.pdf
