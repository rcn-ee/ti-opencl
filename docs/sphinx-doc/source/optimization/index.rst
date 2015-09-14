#################
Optimization Tips
#################

OpenCL applications consist of a host application and a set of device
kernels.  There are optimization techniques for both the host code
and the device code.  There are some techniques that span the boundary between
host and device. This section provides tips for writing OpenCL 
applications that perform well. It targets TI SoCs with DSPs as accelerator devices. These tips are organized into sections based on where the tip is applicable, i.e. the host or device.


.. toctree::
   :glob:
   :maxdepth: 2

   host_code
   dsp_code
   typical_steps
   examples
   performance_data

.. MISC
.. ==============
.. #. timing functions
.. #. additional overhead in first kernel dispatch

