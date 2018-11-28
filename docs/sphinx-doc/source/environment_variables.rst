*********************************************
Environment Variables
*********************************************

These environment variables can be used to control OpenCL behavior and provide visibility for debugging.

.. envvar:: TI_OCL_KEEP_FILES

    When OpenCL C kernels are compiled for DSPs, the result is a binary
    .out file in the /tmp sub-directory. They are then subsequently available
    for download to the DSPs for running. The process of compiling generates
    several intermediate files for each source file. OpenCL typically removes
    these temporary files. However, it can sometimes be useful to inspect these
    files.  This environment variable can be set to instruct the runtime to
    leave the temporary files in /tmp. Inspecting the assembly file associated
    with the out file, can be useful to see how well your code was optimized.

.. envvar:: TI_OCL_DEBUG

    Setting this environment variable modifies the execution of OpenCL
    applications to enable debug of the OpenCL C kernels.  If your application
    uses on-line compilation (i.e. it compiles from a string rather than a
    binary), then that on-line compilation asserts the debug flag to the
    compiler. (If the application is using off-line compilation, i.e. creating
    a program from binary, then you would need to pass -g as an option to the
    off-line compiler clocl. The OpenCL runtime pauses your application before
    dispatch of all kernels. While paused the runtime indicates to the user
    that a kernel dispatch is pending.

    Currently two debugging methods are supported by setting TI_OCL_DEBUG to
    either "gdb" or "ccs".  If set to "gdb", the runtime provides the gdbc6x
    commands to connect to the DSP and setup appropriate breakpoints.  If set
    to "ccs", the runtime provides the Code Composer Studio (CCS) instructions
    to connect to the DSP and setup appropriate breakpoints.  With either
    method, the runtime forces all kernels and work-groups within kernels
    to execute only on DSP core 0.  Details can be found in
    :doc:`debug/index`.

.. envvar:: TI_OCL_CACHE_KERNELS

    On-line compilation of kernels is a useful feature for portable OpenCL
    programs. All the detail required to compile kernels for devices is
    encapsulated in the OpenCL API calls. However, on-line compilation for the
    DSPs can be time-consuming. Setting this environment variable causes the
    OpenCL runtime to perform one on-line compilation of your kernels and cache
    the results in a database in /tmp. Running the application again without
    modification of the kernel source or the options used to compile it,
    results in the compilation step being bypassed, and the use of the cached
    kernel binary.

    .. Warning::

        If OpenCL C kernels call standard C code, modifications to the standard
        C code are not seen by the OpenCL runtime and a cached result may be
        used when it is not appropriate. If calling standard C code, either
        disable this environment variable or clean the cache anytime the
        standard C code is modified.


    .. Warning::

        Using this environment variable causes persistent data to accumulate in
        /tmp, and /tmp may grow to capacity causing run-time errors. If this
        occurs, remove cached objects in /tmp or increase the size of the /tmp
        partition. To explicitly remove the cache, execute the
        command: :command:`rm -f /tmp/opencl*`.

    .. Note::

        The OpenCL compilation cache is automatically removed during a Linux reboot

.. envvar::  TI_OCL_COMPUTE_UNIT_LIST

    Specify the compute units available to the OpenCL runtime as a comma
    separated list of compute unit indices starting at 0.  The specified
    compute unit list must be consecutive, e.g. ``"1,2,3,4"`` on K2H.
    If the environment variable is not specified, the runtime defaults to
    using all the compute units available on the device (or starting from
    OpenCL product version 1.1.13, all available compute units specified in
    ``/etc/ti-mctd/ti_mctd_config.json``).

    Example usage on AM572x:

    .. code-block:: bash
        :caption: runs the vecadd kernel only on DSP1

        -> TI_OCL_COMPUTE_UNIT_LIST="0" ./vecadd

    .. code-block:: bash
        :caption: runs the vecadd kernel only on DSP2

        -> TI_OCL_COMPUTE_UNIT_LIST="1" ./vecadd


    .. code-block:: bash
        :caption: runs the vecadd kernel on both DSP1 and DSP2 (default behavior)

        -> TI_OCL_COMPUTE_UNIT_LIST="0, 1" ./vecadd


    .. Warning::

        Prior to OpenCL product version 1.1.13, this environment variable is
        available only on AM572x.

.. envvar::  TI_OCL_LOAD_KERNELS_ONCHIP

    By default, OpenCL kernel related code and global data is allocated out of
    DDR memory. If this environment variable is set, kernel related code and
    global data is allocated out of MSMC memory.

    ..Warning::

        Rarely used K2x only feature, will be deprecated starting with OpenCL version 1.1.13.0.


.. envvar::  TI_OCL_CPU_DEVICE_ENABLE

    Currently, OpenCL ARM CPU devices only support native kernels (see the
    OpenCL 1.1 spec for a description of native kernels). As a result, the ARM
    CPU is not, by default, treated as a COMPUTE DEVICE when doing an OpenCL
    platform query. If your application only uses the ARM CPU for native
    kernels, then this environment variable can be used to enable it as a
    COMPUTE DEVICE for OpenCL. Enqueueing NDRangeKernels or Tasks to the CPU is
    not supported, even when this environment variable is set.

.. envvar::  TI_OCL_WORKER_SLEEP

    .. Warning::
        Removed in OpenCL 1.1.17, no longer required.

    The OpenCL runtime starts a new CPU thread for every OpenCL command
    queue defined in your application. These threads
    manage the OpenCL command queues and the communication
    between the CPU and the device to which the command queue is associated. If
    there are any OpenCL kernels actively running on the device, the thread
    assigned to monitor the communication with the device on behalf of those
    kernels consumes CPU resources, checking the status of those kernels.
    This environment variable can be used to provide a level of control on how
    much CPU resource is consumed. When TI_OCL_WORKER_SLEEP is unset, the
    OpenCL runtime uses more CPU capacity to ensure the fastest turnaround
    latency on kernel execution. When the TI_OCL_WORKER_SLEEP environment
    variable is set to a number of microseconds, it degrades the
    turnaround latency for a kernel execution to reduce the CPU capacity
    needed to monitor the kernel. If an application is not performance limited
    by CPU cycles or if the application enqueues many fine-grained kernels,
    then having the TI_OCL_WORKER_SLEEP environment variable unset is
    appropriate. In the opposite cases, when CPU cycles are limiting the
    performance of an application or if fewer, but longer running kernels are
    enqueued, then setting TI_OCL_WORKER_SLEEP to some number of microseconds
    is appropriate. The correct number of microseconds to use depends on
    the execution platform and the particular application. However, using a
    microseconds value in the range from 80 to 150 is a reasonable starting
    point.

.. envvar::  TI_OCL_ENABLE_FP64

    The C66x DSP is double precision floating point capable and all the optional
    features in the OpenCL specification for double precision floating point
    are supported in this OpenCL implementation, except for the requirement
    that double FP support include subnormal behavior or graceful underflow.
    The 64-bit floating point hardware on the C66x DSP does not support
    subnormal behavior. It supports flush to zero behavior. To support
    subnormal behavior for doubles would require software emulation that would
    entail a significant performance penalty versus the hardware capabilities
    of the C66x DSP. Therefore, by default the platform and devices supported in
    the TI OpenCL implementation do not report support for double floating
    point. That is, if the platform or device is queried for extensions,
    cl_khr_fp64 is not listed by default. Additionally the OpenCL C predefined
    macro cl_khr_fp64 is not be defined by default. When the
    TI_OCL_ENABLE_FP64 environment variable is set, the TI OpenCL
    implementation reports support for double floating point, i.e.
    cl_khr_fp64 is listed as an extension for the platform and the DSP
    device and cl_khr_fp64 is defined when compiling OpenCL C kernels.
    This environment variable controls whether the OpenCL implementation
    reports support for double. However, double, all double vector types and
    all built-in functions using doubles are supported and available without
    regard to the setting of this environment variable.

.. envvar::  TI_OCL_VERBOSE_ERROR

    The OpenCL specification provides a well-defined mechanism for returning
    error codes from API functions. However, It is often the case that a
    generic error code is returned for differing reasons. When this
    environment variable is set, the OpenCL runtime may print more description
    error messages in addition to the defined return code error mechanism.

.. envvar::  TI_OCL_WG_SIZE_LIMIT

    OpenCL provides a query to a device for the maximum number of work-items
    allowed in a work-group. The DSP device in TI's implementation allows a
    large number of work-items per work-group. Other OpenCL implementations have
    much smaller max work-group size limit. When running code designed and
    optimized for other OpenCL implementations, this environment variable can
    be used to limit the max work-group size reported.

.. envvar::  TI_OCL_PROFILING_EVENT_TYPE

    Specifies the hardware event type to profile. The two basic divisions,
    stall cycle events and memory events, are described in :doc:`profiling`.
    If 1 is specified, OpenCL runtime will profile a stall cycle event.
    If 2 is specified, OpenCL runtime will profile one or two memory event(s).
    Otherwise, profiling is disabled.

.. envvar::  TI_OCL_PROFILING_EVENT_NUMBER1

    Specifies the event number to profile. The exact value of this variable
    represents the offset from either AET_GEM_STALL_EVT_START or
    AET_GEM_MEM_EVT_START, depending on the event type.
    For a full event list, see :doc:`profiling`.

.. envvar::  TI_OCL_PROFILING_EVENT_NUMBER2

    Specifies the second memory event number to profile.  Can be skipped.

.. envvar::  TI_OCL_PROFILING_STALL_CYCLE_THRESHOLD

    Specified the threshold of stall cycles to count.  Only stall events with
    stall cycles higher than this threshold are captured in the counter.
    Default value in OpenCL runtime is 1, i.e. all stall events are captured.

