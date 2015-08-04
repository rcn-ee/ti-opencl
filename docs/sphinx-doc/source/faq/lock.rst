****************************************************************************************
Why do I get messages about /var/lock/opencl when running OpenCL applications?
****************************************************************************************

The TI OpenCL implementation currently allows only 1 OpenCL enabled process
to execute at any given time.  To enforce this,  The OpenCL implementation
locks the file /var/local/opencl when an OpenCL application begins and
frees it when it completes.  If two OpenCL processes attempt to run
concurrently, then one will block waiting for the file lock to be released.
It is possible for an OpenCL application to terminate abnormally and leave
the locked file in place.  If you determine that no other OpenCL process is
running and your OpenCL application still recevies the waiting on
/var/lock/opencl message, then the /var/lock/opencl file can be safely
removed to allow your process to continue. 
