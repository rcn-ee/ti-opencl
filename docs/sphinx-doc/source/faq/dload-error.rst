****************************************************************************************
Why do I get DLOAD error messages when running OpenCL applications?
****************************************************************************************

| << D L O A D >> ERROR: File location of segment 0 is past the end of file.
| << D L O A D >> ERROR: Attempt to load invalid ELF file, '(null)'.

OpenCL uses the directory /tmp to store intermediate compilation results
and to cache compilation results.  This error typically results when /tmp
is full.  You can issue the command :command"`rm /tmp/opencl*` to free /tmp
space.  When either of the environment variables :envvar:`TI_OCL_CACHE_KERNELS` or
:envvar:`TI_OCL_KEEP_FILES` is set, the OpenCL runtime will keep more persistent
data in /tmp and this error could become more frequent.  Either unset these
environment variables or modify your Linux setup to increase the amount of
space allocated to /tmp.

