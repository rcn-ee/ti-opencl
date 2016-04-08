******************************************************
Cache Operations
******************************************************
Knowledge of the details of the cache system and the coherency of the various
caches in the system is not required in order to author correct OpenCL
applications.  The OpenCL runtime will manage coherency operations that are not
already automatic through hardware.

However, TI SoC devices offer flexibility in cache configuration and knowledge
of the cache operation and reconfiguration of the caches can sometimes benefit
application performance.  The TI OpenCL implementation provides additional
OpenCL C built-in functions that allow DSP data cache reconfiguration and explicit
coherency operation. 

For cache-able memory regions residing in DDR and MSMC memory, the data path
to/from the DSP cores will go through an L1D cache and an L2 cache. The default
sizes of these caches are documented in :doc:`device-memory`.

TI SoC devices allow both L1D and L2 memory regions to be partitioned into a
cache region and an SRAM region.  Application characteristics will dictate an
appropriate partition size for these memory areas.  By default, however, the
OpenCL runtime will choose a generic partition, which generally will work well 
for most applications.  

Additional OpenCL C built-in functions are provided in the TI OpenCL
implementation to allow an application author to modify the default partition
and change the relative sizes of the cache vs. SRAM areas in L1D and L2.  

For L1D, the below functions allow you to set the size of the L1D cache,
and secondarily to also set the size of L1D SRAM memory that can be used as
fast scratchpad memory for an application.  The size of the L1D cache
subtracted from the total amount of L1D SRAM available will give the amount of
L1D SRAM that can be used as scratchpad memory. The default partition for L1D 
is all cache and no scratchpad.

.. c:function:: void     __cache_l1d_none  (void)

    Sets the L1D memory to 0 bytes cache and all bytes SRAM scratchpad.

.. c:function:: void     __cache_l1d_all   (void)

    Sets the L1D memory to all bytes cache and 0 bytes SRAM scratchpad.

.. c:function:: void     __cache_l1d_4k    (void)

    Sets the L1D memory to 4K bytes cache and the remainder of SRAM as scratchpad.

.. c:function:: void     __cache_l1d_8k    (void)

    Sets the L1D memory to 8K bytes cache and the remainder of SRAM as scratchpad.

.. c:function:: void     __cache_l1d_16k   (void)

    Sets the L1D memory to 16K bytes cache and the remainder of SRAM as scratchpad.

.. c:function:: void     __cache_l1d_flush (void)

    User controlled, explicit L1D cache flush operation.  This will write-back
    any dirty lines in the L1D cache and will mark all lines as invalid.

.. c:function:: void*      __scratch_l1d_start (void)

    Returns the base address of the L1D SRAM memory region that can be used as
    scratchpad memory.  Available starting in release 1.1.5.0.

.. c:function:: uint32_t*  __scratch_l1d_size  (void)

    Returns the size of the L1D SRAM memory region that can be used as
    scratchpad memory.  By default, this will be 0, but if the L1D cache is
    reduced, then the value returned by this function will increase.  The value
    returned by this built-in function may vary from core to core because each
    core independently sets an l1d cache size.  Available starting in release 1.1.5.0.

.. Note:: The function __scratch_l1d_size only returns valid sizes after
          calling __cache_l1d_*changesize* functions listed above.  If user
          calls CSL (Chip Support Library) functions to change L1D cache
          directly, __scratch_l1d_size will NOT return valid size.  This
          will be fixed in the next TI OpenCL product release.

L2 memory is similar to L1D in that it can be software partitioned between
cache and scratchpad.  The below functions can be used to control that 
partition.  However, there are some differences between L1D and L2.

    #. The default L2 cache size will be a fraction of the total size and will
       typically be 128K, or smaller if the total L2 memory area is small.

    #. A portion of the L2 scratchpad memory is reserved for use by the OpenCL runtime.

    #. OpenCL already has a mechanism that allows the remaining L2 scratchpad
       memory to be used by applications.  That mechanism is local buffers.
       Local buffers are allocated from L2 scratchpad memory.

Where for L1D cache, the typical use case for using the reconfiguration
functions would be to reduce the cache and thus increase the L1D available as
scratchpad, for L2 the typical use case would be to increase cache for
applications that can benefit from a larger cache capacity and are not already
using local scratch buffers.


.. c:function:: void     __cache_l2_none   (void)

    Sets the L2 memory to 0 bytes cache and the all bytes SRAM scratchpad.

.. c:function:: void     __cache_l2_128k   (void)

    Sets the L2 memory to 128K bytes cache and the remainder of SRAM as
    scratchpad. (default)

.. c:function:: void     __cache_l2_256k   (void)

    Sets the L2 memory to 256K bytes cache and the remainder of SRAM as scratchpad.
    Only available if total L2 space is >= 512KB.

.. Note:: The function __cache_l2_256k is not available on the AM57 platform.

.. c:function:: void     __cache_l2_512k   (void)

    Sets the L2 memory to 512K bytes cache and the remainder of SRAM as scratchpad.
    Only available is total L2 space is >= 1MB.
    
.. Note:: The function __cache_l2_512k is not available on the AM57 platform.

.. c:function:: void     __cache_l2_flush  (void)

    User controlled, explicit L2 cache flush operation.  This will write-back
    any dirty lines in the L1D cache and L2 cache and will mark all lines in
    both cache levels as invalid.

.. Note::

    Configuring all of L2 as cache is not an available option, because the
    OpenCL runtime needs some L2 scratchpad memory for proper operation.

.. Note::

    The L2 cache is a shared data / program cache.  Reducing the size of the L2
    cache will also affect the caching behavior of the program code and may
    reduce application performance

.. Warning::

    Increasing the size of the L2 cache in OpenCL C code must be used with caution.
    The host OpenCL runtime will not be aware of the use of the cache resizing
    functions. Because the OpenCL runtime is also managing the L2 scratchpad memory 
    for use as local buffers, an opportunity for resource conflict exists.  As a 
    general rule of thumb, do not increase L2 cache size in functions that are using
    local buffers.

.. Warning::

    The cache size reconfiguration functions should not be used in kernels with
    > 1 work-item per work-group.
