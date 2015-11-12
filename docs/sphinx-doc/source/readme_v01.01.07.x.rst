OpenCL v01.01.07.x Readme
=========================

* Added the following extentions OpenCL C builtin functions. ::

    int      __cache_l2_32k(void);
    int      __cache_l2_64k(void);

    void*    __scratch_l2_start(void);
    uint32_t __scratch_l2_size(void);

* Modified the following functions to return an int rather than void.  They
  will return 1 on success and 0 on failure.  Calls that will shrink the cache
  size will always succeed.  Calls that increase the cache size may fail if the
  delta amount of memory dedicated to cache is not available based on other uses
  of L2 scrathpad memory. ::

    int      __cache_l1d_none  (void);
    int      __cache_l1d_all   (void);
    int      __cache_l1d_4k    (void);
    int      __cache_l1d_8k    (void);
    int      __cache_l1d_16k   (void);
    int      __cache_l2_none   (void);
    int      __cache_l2_32k    (void);
    int      __cache_l2_64k    (void);
    int      __cache_l2_128k   (void);
    int      __cache_l2_256k   (void);
    int      __cache_l2_512k   (void);

* These list of supported C6X intrinsics has been greatly expanded. See
  :doc:`extensions/c66-intrinsics` for the complete list.

* The OpenCL C builtin function rhadd has been optimized for ucharn and shortn
  data types.


* Changed behavior of local buffers defined in OpenCL C kernel functions to
  conform to the OpenCL spec.  These buffers will now only reserve L2 memory for
  the duration of the kernel.  Previously, they reserved space in L2 from OpenCL
  C program load to program unload.  Any code that may have been written to take
  advantage of this persistent memory, will now be incorrect, as the contents of
  that L2 memory is not guaranteed to persist beyond the current kernel
  execution.


* Any standard C code that is linked with OpenCL C code and also contains
  object definitions with the __attribute__((section(".mem_l2"))) will still
  reserve memory for those object from OpenCL program load to OpenCL program
  unload.  However,  the contents of that memory is not guaranteed to persist
  between kernels calls.  Additionally, any initialization data for those object
  will be ignored
