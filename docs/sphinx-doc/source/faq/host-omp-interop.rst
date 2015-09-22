*********************************************
OpenCL Interoperability with Host OpenMP
*********************************************

The OpenCL API's are defined to be thread safe.  It is therefore safe for
multiple threads created in an OpenMP parallel region to enqueue to a shared
OpenCL queue.  The following example C++ code will create one OpenCL command
queue and will enqueue a kernel from multiple threads in an OpenMP parallel
region.  The number of times the kernel is enqueued is dependent on the default
number of threads the OpenMP runtime creates.  This will usually be equal to
the number of CPU cores in the system.  This example uses the C++ functor
construct to make a kernel enqueue command appear to be a function call.  This
example also uses the one Q.finish() command outside the parallel region to
effectively wait_all on the asynchronous hello kernel enqueues.::

    #define __CL_ENABLE_EXCEPTIONS
    #include <CL/cl.hpp>
    #include <iostream>
    #include <cstdlib>
    #include <cstdio>

    using namespace cl;
    using namespace std;

    const char * kernelStr = "kernel void Hello() { printf(\"Hello\\n\"); }";

    int main(int argc, char *argv[])
    {
       try
       {
         Context             context (CL_DEVICE_TYPE_ACCELERATOR);
         std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
         Program::Sources    source(1, std::make_pair(kernelStr,strlen(kernelStr)));
         Program             program = Program(context, source);
         program.build(devices);
         CommandQueue        Q     (context, devices[0], CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
         Kernel              kernel(program, "Hello");
         KernelFunctor hello = kernel.bind(Q, NDRange(1), NDRange(1));

         #pragma omp parallel
             hello();

         Q.finish();
       }
       catch (Error err)
       { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }
    }


The previous example contained one OpenCL Q that was shared across multiple
threads.  It is also valid to define multiple OpenCL command queues tied to the
same device, where each command Queue is private to the thread.  The following
example is a slightly modified version of the above illustrating the use of
private command queues. Note the definition of the command queue and the
functor in the parallel region.  Also note that since there will not be a
command queue defined in the main thread, one finish() API call to wait on all
threads is not possible, so a wait is attached to each kernel enqueue command.::

    #define __CL_ENABLE_EXCEPTIONS
    #include <CL/cl.hpp>
    #include <iostream>
    #include <cstdlib>
    #include <cstdio>

    using namespace cl;
    using namespace std;

    const char * kernelStr = "kernel void Hello() { printf(\"Hello\\n\"); }";

    int main(int argc, char *argv[])
    {
       try
       {
         Context             context (CL_DEVICE_TYPE_ACCELERATOR);
         std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
         Program::Sources    source(1, std::make_pair(kernelStr,strlen(kernelStr)));
         Program             program = Program(context, source);
         program.build(devices);
         Kernel              kernel(program, "Hello");

         #pragma omp parallel
         {
             CommandQueue  Q (context, devices[0], CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
             KernelFunctor hello = kernel.bind(Q, NDRange(1), NDRange(1));
             hello().wait();
         }
       }
       catch (Error err)
       { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }
    }
