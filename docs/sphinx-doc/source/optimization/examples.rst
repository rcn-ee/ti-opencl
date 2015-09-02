########################################
Optimizing 3x3 Gaussian smoothing filter
########################################

This section describes a step-by-step approach to optimzing the 3x3 Gaussian smoothing filter kernel for the C66x DSP.

Overview of Gaussian Filter
===========================
The Gaussian Filter is used as a smoothing filter. The filter is applied by convolving a nxn image window with a nxn Gaussian kernel and obtaining a weighted sum. More on the filter is available here: http://homepages.inf.ed.ac.uk/rbf/HIPR2/gsmooth.htm 

The kernel size that we are using here is a 3x3 kernel. Let A be a 3x3 image window and B be the 3x3 gaussian kernel. The filter is applied by convolving A and B and A is obtained in a sliding window fashion.

Natural C Code
==============
The first listing is a snippet of C code for convolution:

.. literalinclude:: code/natc.c
   :language: c
   :linenos:

Optimizing for DSP
==================
An OpenCL C kernel for convolution. Note that the types are float.

.. literalinclude:: code/gen_ocl.c
   :language: c
   :linenos:


Step 1: Initial optimization for DSP:
  * Convert the float type to uchar

.. literalinclude:: code/ast.c
   :language: c
   :linenos: 
 

Step 2: 
  * Switch to using vector types to take advantage of vector instructions available on the DSP
  * Annotate the kernel with a work-group size attribute

.. literalinclude:: code/dp.c
   :language: c
   :linenos:

Step 3: Use double buffering to overlap data movement with computation

Psuedo-code for a double-buffered version of the OpenCL C kernel:

.. literalinclude:: code/dao.c
   :language: c
   :linenos: 

Now, we have an optimized OpenCL C kernel for the DSP. Note that the kernel is a generic OpenCL C kernel and can be compiled/run on any OpenCL device.

Performance Improvement
=======================

=================================   ===============================
Description                         Performance in cycles per pixel
=================================   ===============================
Generic OpenCL C kernel             12.0 
OpenCL C kernel optimized for DSP   5.0
=================================   ===============================

	

  
