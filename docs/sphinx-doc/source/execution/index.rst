##################
Execution Model
##################

OpenCL is a host CPU based library containing APIs that allow you to:

1. Discover other devices in the system that are available for compute, 
2. Define programs to run on those other devices, 
3. Define buffers that can be used to communicate data between the host 
   program and programs running on those other devices, and 
4. To queue work to those other devices.

Step 1 is documented in :doc:`device-discovery`.  Step 3 is documented in
:doc:`../memory/index`.  Compilation of programs defined in step 2 is documented
in :doc:`../compilation`.  

The remaining items:

- The expression of programs in step 2, and 
- The mechanisms used to queue work in step 4

are the essence of the execution model as it relates to how work is
accomplished on devices and is the main topic for this chapter.

.. toctree::
   :maxdepth: 2

   device-discovery

