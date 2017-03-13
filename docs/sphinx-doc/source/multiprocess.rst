**************************************
Dispatch from multiple Linux processes
**************************************

On OpenCL versions 1.1.11 and older, the Linux process corresponding to an OpenCL application takes ownership of the OpenCL DSP device on encountering the first OpenCL API call and relinquishes ownership when it ends. Mutually exclusive access was implemented using a file lock, ``/var/lock/opencl``. If two processes A and B contained OpenCL API calls and A started running, A would acquire the file lock when it executed the first OpenCL API call. Process A would hold the lock until it completed exectution at which time process B can acquire the lock. Also, if a process forked a child process and both processes invoked OpenCL APIs, the processes would deadlock.

Starting with version 1.1.12, the process level lock, ``/var/lock/opencl`` was eliminated. Multiple Linux processes can dispatch OpenCL kernels to the DSPs. Each kernel runs to completion. The execution of kernels submitted by multiple processes is interleaved.

ti-mctd
-------

To enable concurrent processes to dispatch OpenCL APIs, a daemon was added (``ti-mctd``). The deamon provides the following services:

#. Manage the OpenCL contiguous memory (CMEM) heap. Since multiple processes with OpenCL APIs share the heap, the daemon provides a centralized location to manage the heap.

#. OpenCL DSP monitor lifetime management (only on K2x SoCs).  On K2x devices, the ti-mctd daemon uses MPM to reset, load and run the OpenCL monitor on the DSPs during boot. 

The ti-mctd daemon is started during boot as a systemd service. The associated systemd unit file is located at ``/lib/systemd/system/ti-mct-daemon.service``

.. note::

    The daemon can be stopped by the user by executing ``pkill ti-mctd`` and restarted by executing ``ti-mctd``. Scenarios in which the user will need to restart the daemon include:

    #. The DSPs crash  (or)
    #. The Linux inter-process shared memory heap managed by the daemon runs out of space (application terminates with a ``boost::interprocess::bad_alloc`` exception). 
           
    
With scenario #2, the ``ti-mctd`` daemon can be stopped and restarted with the -s option to specify a larger Linux shared memory region. E.g. ``ti-mctd -s 262144`` to allocate 256KB:

.. code-block:: bash

    # pkill ti-mctd
    # ti-mctd -s 262144
    # ls -lh /dev/shm/HeapManager
    -rw-r--r--    1 root     root      256.0K Feb 20 17:55 /dev/shm/HeapManager

ti-mct-heapcheck
----------------
There is also another program, ``ti-mct-heap-check``. This program prints out the current state of the OpenCL CMEM heap.  The –c option can be used to garbage collect any allocated blocks associated with non-running processes.
 
