.. _faq:

FAQs
****


How do I get support for TI OpenCL products?
  Post your questions and/or suspected defects to the `High Performance Computing forum <http://e2e.ti.com/support/applications/high-performance-computing/f/952.aspx>`_ with the tag  *opencl*.

Which version of OpenCL do I have installed?
  See the page :doc:`version` for details.

Can multiple OpenMP threads in the host application submit to OpenCL queues?
  Yes, each thread could have a private queue or the threads could share a queue.  The OpenCL API's are thread safe. See the page :doc:`opencl_with_openmp_on_host` for more details.

When running OpenCL application I get the error messages:
  ``<< D L O A D >> ERROR: File location of segment 0 is past the end of file.``
  ``<< D L O A D >> ERROR: Attempt to load invalid ELF file, '(null)'.``

  OpenCL uses the directory /tmp to store intermediate compilation results and to cache compilation results.  This error typically results when /tmp is full.  You can issue the command “rm /tmp/opencl*” to free /tmp space.  When either of the environment variables TI_OCL_CACHE_KERNELS or TI_OCL_KEEP_FILES are set, the OpenCL runtime will keep more persistent data in /tmp and this error could become more frequent.  Either unset these environment variables or modify your Linux setup to increase the amount of space allocated to /tmp.

When running OpenCL applications I get messages about /var/lock/opencl
    The TI OpenCL implementation currently allows only 1 OpenCL enabled process to execute at any given time.  To enforce this,  The OpenCL implementation locks the file /var/local/opencl when an OpenCL application begins and frees it when it completes.  If two OpenCL processes attempt to run concurrently, then one will block waiting for the file lock to be released.  It is possible for an OpenCL application to terminate abnormally and leave the locked file in place.  If you determine that no other OpenCL process is running and your OpenCL application still recevies the waiting on /var/lock/opencl message, then the /var/lock/opencl file can be safely removed to allow your process to continue. 
