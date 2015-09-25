*********************************************
Environment Variables
*********************************************

These environment variables can be used to control OpenCL behavior and provide visibility for debugging.

.. envvar:: TI_OCL_KEEP_FILES          

    When OpenCL C kernels are compiled for DSPs, they are compiled to a binary
    .out file in the /tmp sub-directory. They are then subsequently available
    for download to the DSPs for running. The process of compiling generates
    several intermediate files for each source file. OpenCL typically removes
    these temporary files. However, it can sometimes be useful to inspect these
    files.  This environment variable can be set to instruct the runtime to
    leave the temporary files in /tmp. This can be useful to inspect the
    assembly file associated with the out file, to see how well your code was
    optimized.

.. envvar:: TI_OCL_DEBUG               

    When this environment variable is set and an OpenCL application is run, the
    run will be modified in a few ways to enable debug of the OpenCL C kernels.
    If you application uses online compilation (i.e. it compiles from a string
    rather than a binary), then that online compilation will assert the debug
    flag to the compiler. (If the application is using offline compilation,
    i.e. creating a program from binary, then you would need to pass -g as an
    option to the offline compiler clocl. The OpenCL runtime will pause your
    application before dispatch of all kernels. While paused the runtime
    indicate to the user that a kernel dispatch is pending. It will provide the
    gdb6x command to connect to the DSP and to setup appropriate breakpoints.
    The runtime will also force all kernels and workgroups within kernels to
    execute on only DSP core 0.

.. envvar:: TI_OCL_CACHE_KERNELS       

    Online compilation of kernels is a useful feature for portable OpenCL
    programs. All the details of compiling kernels for devices is encapsulated
    in the OpenCL API calls. However, online compilation for the DSPs can be
    time consuming. When this environment variable is set, the OpenCL runtime
    will perform online compilation of your kernels and will save the result in
    a database in /tmp. If you run the application again without modifying the
    kernel source or the options used to compile it, the compile step will be
    by passed and the cached compile result will be used instead. Note however,
    If your OpenCL C kernels call standard C code routines from a linked object
    file or library, modifications to that standard C routine cannot be seen by
    the OpenCL runtime and a cached result may be used when it is not
    appropriate. This option may not be appropriate in this circumstance When
    this option is used, persistent data is cached in /tmp, and as a result
    /tmp may grow to capacity. If this occurs, you will need to remove cached
    objects in /tmp and/or increase the size of the /tmp partition. You can
    explicitly remove the cache by executing the command: rm -f /tmp/opencl*.
    Since the cached objects are in /tmp, they will be removed automatically
    when Linux is rebooted.

.. envvar::  TI_OCL_LOAD_KERNELS_ONCHIP 

    By default, OpenCL kernel related code and global data is allocated out of
    of DDR memory. If this environment variable is set, kernel related code and
    global data will be allocated out of MSMC memory. 

.. envvar::  TI_OCL_CPU_DEVICE_ENABLE   

    Currently, OpenCL ARM cpu devices only support native kernels (see the
    OpenCL 1.1 spec for a description of native kernels). As a result, the ARM
    cpu is not, by default, treated as a COMPUTE DEVICE when doing an OpenCL
    platform query. If your application only uses the ARM cpu for native
    kernels then this environment variable can be used to enable it as a
    COMPUTE DEVICE for OpenCL. Enqueueing NDRangeKernels or Tasks to the CPU is
    not supported, even if this environment variable is set. 
    
.. envvar::  TI_OCL_WORKER_SLEEP        

    The OpenCL runtime will start a new CPU thread for every OpenCL command
    queue defined in your application. These threads are responsible for
    managing the OpenCL command queues and for also managing the communication
    between the CPU and the device to which the command queue is associated. If
    there are any OpenCL kernels actively running on the device, the thread
    assigned to monitor the communication with the device on behalf of those
    kernels will consume CPU resources, checking the status of those kernels.
    This environment variable can be used to provide a level of control on how
    much CPU resource is consumed. When TI_OCL_WORKER_SLEEP is unset, the
    OpenCL runtime will use more CPU capacity to ensure the fastest turnaround
    latency on kernel enqueues. When the TI_OCL_WORKER_SLEEP environment
    variable is set to a specific number of microseconds, it will degrade the
    turnaround latency for a kernel enqueue in order to reduce the CPU capacity
    needed to monitor the kernel. If an application is not performance limited
    by CPU cycles or if the application enqueues many fine grained kernels,
    then having the TI_OCL_WORKER_SLEEP environment variable unset is
    appropriate. In the opposite cases, when CPU cycles are limiting the
    performance of an application or if fewer, but longer running kernels are
    enqueued, then setting TI_OCL_WORKER_SLEEP to some number of microseconds
    is appropriate. The correct number of microseconds to use will depend on
    the execution platform and the specific application. However, using a
    microseconds value in the range from 80 to 150 is a reasonable starting
    point.

.. envvar::  TI_OCL_ENABLE_FP64         

    The C66 DSP is double precision floating point capable and all the optional
    features in the OpenCL specification for double precision floating point
    are supported in this OpenCL implementation, except for the requirement
    that double FP support include subnormal behavior, or graceful underflow.
    The 64 bit floating point hardware on the C66 DSP does not support
    subnormal behavior. It supports flush to zero behavior. To support
    subnormal behavior for doubles would require software emulation which would
    entail a significant performance penalty versus the hardware capabilities
    of the C66 DSP. Therefore, by default the platform and devices supported in
    the TI OpenCL implementation do not report support for double floating
    point. That is, if the platform or device is queried for extensions,
    cl_khr_fp64 is not listed by default. Additionally the OpenCL C predefined
    macro cl_khr_fp64 will not be defined by default. When the
    TI_OCL_ENABLE_FP64 environment variable is set, the TI OpenCL
    implementation will report support for double floating point. That is,
    cl_khr_fp64 will be listed as an extension in the platform and the DSP
    device and cl_khr_fp64 will be defined when compiling OpenCL C kernels.
    This environment variable controls whether the OpenCL implementation
    reports support for double. However, double, all double vector types and
    all builtin functions using doubles are supported and available without
    regard to the setting of this environment variable.

.. envvar::  TI_OCL_VERBOSE_ERROR       

    The OpenCL specification provides a well defined mechanism for returning
    error codes from API functions. However, It is often the case that a
    generic error code is returned for a number of differing reasons. When this
    environment variable is set, the OpenCL runtime may print more description
    error messages in addition to the defined return code error mechanism.

.. envvar::  TI_OCL_WG_SIZE_LIMIT       

    OpenCL provides a query to a device for the maximum number of work-items
    allowed in a workgroup. The DSP device in TI's implementation allows a very
    large number of work-items per workgroup. Other OpenCL implementations have
    much smaller max workgroup size limit. When running code designed and
    optimized for other OpenCL implementations, this environment variable can
    be used to artificially limit the max workgroup size reported. 

.. envvar::  TI_OCL_CGT_INSTALL         

    The OpenCL runtime is dependent on the C66 DSP compiler product for
    compilation of OpenCL C kernels. When OpenCL C kernels are compiled on the
    target ARM/Linux system, the C66 compiler is assumed to be installed in the
    standard Linux locations. However, offline cross compilation of OpenCL C
    kernels is also supported from x86 Ubuntu machines and in that use case, it
    is required that this environment variable is set to the top level
    directory path where the C66 cross compiler tools are installed. 

.. envvar::  TI_OCL_DSP_1_25GHZ         

    Initialize the C66 DSPs to run at 1.25 Ghz rather than the default 1.00 Ghz.

    The TI_OCL_DSP_1_25GHZ environment variable is only applicable to the 
    DSPC8681 OpenCL Implementation.  The DSP frequency on the other platforms 
    is determined at Linux boot time.

