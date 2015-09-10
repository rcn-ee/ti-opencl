################
Performance Data
################

The table below show improvements obtained from optimizing a set of image processing kernels for the C66x DSP device using the techniques described in this chapter.

==================    =================== ==================  ==============
Kernel                Generic OpenCL C    Optimized OpenCL C  Improvement
                      (cycles/pixel)      (cycles/pixel)      (times faster)
==================    =================== ==================  ==============
Convolution           12                  5                   **2.40**
Histogram             56                  1.75                **32.00**
X_Gradient            12.4                1.25                **9.92**
Edge Relaxation       530                 48                  **11.04**
==================    =================== ==================  ==============
