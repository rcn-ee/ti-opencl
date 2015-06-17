******************************************************
Device Memory 
******************************************************

The following device components are relevant to the memory discussion for
OpenCL.

+----------------------------------+-----------+----------------+
| Attribute                        | 66AK2H    | AM57           |
+==================================+===========+================+
| ARM A15 CPU cores                | 4         | 2              |
+----------------------------------+-----------+----------------+
| C66 DSP cores                    | 8         | 2              |
+----------------------------------+-----------+----------------+
| L1P per C66 core                 | 32KB      | 32KB           |
+----------------------------------+-----------+----------------+
| L1D per C66 core                 | 32KB      | 32KB           |
+----------------------------------+-----------+----------------+
| L2 cache shared across ARM cores | 4MB       | 2MB            |
+----------------------------------+-----------+----------------+
| L2 memory per C66 core           | 1MB       | 288KB          |
+----------------------------------+-----------+----------------+
| DDR3 available                   | up to 8GB | 2GB            |
+----------------------------------+-----------+----------------+
| On-chip shared memory            | 6MB       | N/A            |
+----------------------------------+-----------+----------------+

The L1 and L2 memory areas in the C66 cores can be configured as all cache, all
scratchpad or partitioned with both. For OpenCL applications, this partition is 
fixed as follows for each C66 core:

+---------------------------------------+--------+-------+
| Attribute                             | 66AK2H | AM57  |
+=======================================+========+=======+
| L1P cache                             | 32KB   | 32KB  |
+---------------------------------------+--------+-------+
| L1D cache                             | 32KB   | 32KB  |
+---------------------------------------+--------+-------+
| L2 cache                              | 128KB  | 128KB |
+---------------------------------------+--------+-------+
| L2 reserved                           | 128KB  | 32KB  |
+---------------------------------------+--------+-------+
| L2 available for OpenCL local buffers | 768KB  | 128KB |
+---------------------------------------+--------+-------+

.. Note::
    The amount of Local, MSMC and DDR that is available for OpenCL use
    may change from release to release.  The amount available for OpenCL use
    can be queried from the OpenCL runtime.  This is illustrated in the
    platform example shipped with the product:
    ``/usr/share/ti/examples/opencl/platforms``.
