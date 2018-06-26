**************************************
Dispatch from multiple Linux processes
**************************************

On OpenCL versions 1.1.11 and older, the Linux process corresponding to an OpenCL application takes ownership of the OpenCL DSP device on encountering the first OpenCL API call and relinquishes ownership when it ends. Mutually exclusive access was implemented using a file lock, ``/var/lock/opencl``. If two processes A and B contained OpenCL API calls and A started running, A would acquire the file lock when it executed the first OpenCL API call. Process A would hold the lock until it completed execution at which time process B can acquire the lock. Also, if a process forked a child process and both processes invoked OpenCL APIs, the processes would deadlock.

Starting with version 1.1.12, the process level lock, ``/var/lock/opencl`` was eliminated. Multiple Linux processes can dispatch OpenCL kernels to the DSPs. Each kernel runs to completion. The execution of kernels submitted by multiple processes is interleaved.

ti-mctd
-------

To enable concurrent processes to dispatch OpenCL APIs, a daemon was added (``ti-mctd``). The daemon provides the following services:

#. Manage the OpenCL contiguous memory (CMEM) heap. Since multiple processes with OpenCL APIs share the heap, the daemon provides a centralized location to manage the heap.

#. OpenCL DSP monitor lifetime management (only on K2x SoCs).  On K2x devices, the ti-mctd daemon uses MPM to reset, load and run the OpenCL monitor on the DSPs during boot. 

The ti-mctd daemon is started during boot as a systemd service. The associated systemd unit file is located at ``/lib/systemd/system/ti-mct-daemon.service``

.. note::

    The daemon can be stopped by the user by executing ``pkill ti-mctd`` and restarted by executing ``ti-mctd``. Scenarios in which the user will need to restart the daemon include:

    #. The DSPs crash  (or)
    #. The Linux inter-process shared memory heap managed by the daemon runs out of space (application terminates with a ``boost::interprocess::bad_alloc`` exception). 
           
    
With scenario #2, the ``ti-mctd`` daemon can be stopped and restarted with a larger Linux shared memory region specified in the daemon config file, ``/etc/ti-mctd/ti_mctd_config.json``. E.g.

.. code-block:: bash

    # pkill ti-mctd
    # edit /etc/ti-mctd/ti_mctd_config.json, increase linux-shmem-size-KB from 128 to 256
    # rm /dev/shm/HeapManager (if still exists)
    # ti-mctd
    # ls -lh /dev/shm/HeapManager
    -rw-r--r--    1 root     root      256.0K May  2 15:44 /dev/shm/HeapManager

ti-mct-heapcheck
----------------
There is also another program, ``ti-mct-heap-check``. This program prints out the current state of the OpenCL CMEM heap.  The –c option can be used to garbage collect any allocated blocks associated with non-running processes.
 
ti-mctd config file
-------------------
Starting from OpenCL product version 1.1.13, the ``ti-mctd`` daemon reads
a configuration file, ``/etc/ti-mctd/ti_mctd_config.json``, when it starts.
The following is a sample configuration from K2H EVM:

.. code-block:: bash

    root@k2hk-evm:~# cat /etc/ti-mctd/ti_mctd_config.json
    {
            "cmem-block-offchip" : "0",
            "cmem-block-onchip" : "1",
            "compute-unit-list" : "0,1,2,3,4,5,6,7",
            "linux-shmem-size-KB" : "128",
            "eve-devices-disable" : "0",
    }

``cmem-block-offchip`` and ``cmem-block-onchip`` specify the CMEM block
ids for OpenCL use.  The offchip CMEM block is required for OpenCL, while
the onchip one is optional.  These two config items are provided so that
OpenCL CMEM blocks can co-exist with CMEM blocks dedicated for other uses.

``compute-unit-list`` specifies which DSPs that OpenCL applications can use.
This is a system wide config.  It can be overridden by environment variable
``TI_OCL_COMPUTE_UNIT_LIST`` on a per-shell or per-application basis.
The compute unit list must be consecutive.  But it doesn't need to start
from the first available and end with the last available DSP in the system.
For example, ``1,2,3,4,5,6`` is a valid compute unit list on K2H.
OpenCL kernels will only be dispatched to the DSPs specified in this list.
This config item is provided so that OpenCL DSPs can co-exist with DSPs
dedicated for other uses.

``linux-shmem-size-KB`` specifies the size of Linux shared memory that
the daemon and OpenCL applications use to manage shared data structure.
As already discussed in previous section, increase it if you see
``bad_alloc`` exception.

``eve-devices-disable`` specifies whether to disable EVE devices,
if available on the SoC (e.g. AM57x9s).  By default, the value is ``0``,
which means EVE devices are enabled in OpenCL runtime.  Should user
choose to not use available EVE devices in the OpenCL runtime,
please change the value to ``1``.

Restrictions on multiple OpenCL processes
-----------------------------------------
Starting from OpenCL product version 1.1.13, when OpenMP extension is not used,
an OpenCL application can run in parallel with other OpenCL applications as
long as for any two concurrent applications, they use either the same compute
unit list or two non-overlapping compute unit lists.
For example, ``sgemm`` example does not use OpenMP extension, the following
``sgemm`` instances can be run in parallel on K2H:

.. code-block:: bash

    root@k2hk-evm:~# ./sgemm -M 1024 -K 2000 -N 1000 -r & \
                     ./sgemm -M 1000 -K 2000 -N 1024 -r
    ## or launch them simultaneously in two separate windows/shells

.. code-block:: bash

    root@k2hk-evm:~/examples/sgemm# \
    TI_OCL_COMPUTE_UNIT_LIST="0,1,2,3" ./sgemm -M 1024 -K 2000 -N 1000 -r & \
    TI_OCL_COMPUTE_UNIT_LIST="0,1,2,3" ./sgemm -M 1000 -K 2000 -N 1024 -r & \
    TI_OCL_COMPUTE_UNIT_LIST="4,5" ./sgemm -M 1000 -K 2048 -N 1024 -r & \
    TI_OCL_COMPUTE_UNIT_LIST="6,7" ./sgemm -M 1024 -K 2048 -N 1000 -r
    ## or launch them simultaneously in four separate windows/shells

.. Warning::
    OpenCL applications dispatching kernels with OpenMP extension
    can not run in parallel with any other OpenCL applications.  Starting
    from OpenCL product version 1.1.13, when OpenMP extension is used,
    OpenCL application can still run with a reduced compute unit list
    (i.e. not all available dsps are used).  E.g. ``dgemm`` example
    uses OpenMP extension in its kernels, you can force it to run on only core
    1 and 2 by ``TI_OCL_COMPUTE_UNIT_LIST="1,2" ./dgemm`` on K2H.

.. Warning::
    When running OpenCL applications in parallel, in general, the side effects
    of each kernel should be self-contained and not extend beyond the kernel
    boundary.  Otherwise, they might affect kernels from other applications
    without other applications even knowing it.  For example, if a kernel
    reduces cache size to obtain some fast scratch memory, it should put the
    cache back to its original size when the kernel finishes.  The following
    is a list of actions that have side effects (not limited to this list):

    #. Changing cache sizes
    #. Allocating a buffer and using it in a kernel with user defined dsp heap
       extension
