#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>

using namespace cl;
using std::cout;
using std::cerr;
using std::endl;

void report_event_timing(const Event &ev, const char* name);

const char * kernelStr =
    "kernel void VectorMpy(global const short4* a, "
    "                      global const short4* b, "
    "                      global short4* c) "
    "{"
    "    int id = get_global_id(0);"
    "    c[id] = a[id] * b[id];"
    "}";


const int NumElements     = 1024*1024;     
const int VectorElements  = 4;
const int NumWorkGroups   = 8;
const int NumVecElements  = NumElements / VectorElements;
const int WorkGroupSize   = NumVecElements / NumWorkGroups;

cl_short srcA  [NumElements];
cl_short srcB  [NumElements];
cl_short dst   [NumElements];
cl_short Golden[NumElements];

int main(void)
{
   cl_int err     = CL_SUCCESS;
   int    bufsize = sizeof(srcA);

   for (int i=0; i < NumElements; ++i) 
   { 
       srcA[i] = srcB[i] = i<<2; 
       Golden[i] = srcB[i] * srcA[i]; 
   }

   try 
   {
     Context             context(CL_DEVICE_TYPE_ACCELERATOR); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     Buffer bufA   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufB   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufDst (context, CL_MEM_WRITE_ONLY, bufsize);

     cl_buffer_region rgn = {0, bufsize};
     Buffer sbufA   = bufA.createSubBuffer  (CL_MEM_READ_ONLY,  CL_BUFFER_CREATE_TYPE_REGION, &rgn);
     Buffer sbufB   = bufB.createSubBuffer  (CL_MEM_READ_ONLY,  CL_BUFFER_CREATE_TYPE_REGION, &rgn);
     Buffer sbufDst = bufDst.createSubBuffer(CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &rgn);

     Program::Sources    source(1, std::make_pair(kernelStr,strlen(kernelStr)));
     Program             program = Program(context, source);
     program.build(devices);

     Kernel kernel(program, "VectorMpy");
     kernel.setArg(0, sbufA);
     kernel.setArg(1, sbufB);
     kernel.setArg(2, sbufDst);

     Event ev;

     CommandQueue Q_DSP(context, devices[0], CL_QUEUE_PROFILING_ENABLE);
     Q_DSP.enqueueWriteBuffer(sbufA, CL_FALSE, 0, bufsize, srcA);
     Q_DSP.enqueueWriteBuffer(sbufB, CL_FALSE, 0, bufsize, srcB);
     Q_DSP.enqueueBarrier();
     Q_DSP.enqueueNDRangeKernel(kernel, NullRange, NDRange(NumVecElements), NDRange(WorkGroupSize), NULL, &ev);
     Q_DSP.enqueueBarrier();
     Q_DSP.enqueueReadBuffer (sbufDst, CL_TRUE, 0, bufsize, dst);

     report_event_timing(ev, "Kernel Exec");
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   for (int i=0; i < NumElements; ++i)
       if (Golden[i] != dst[i]) { cout << "Failed at Element " << i << endl; break; }
}

/******************************************************************************
* REPORT_EVENT_TIMING - Given an OpenCL Event, report to stdout the profiling
*    info associated with the event
******************************************************************************/
void report_event_timing(const Event &ev, const char* name)
{
     cl_ulong t_que, t_sub, t_strt, t_end;
     
     ev.getProfilingInfo(CL_PROFILING_COMMAND_QUEUED, &t_que);
     ev.getProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, &t_sub);
     ev.getProfilingInfo(CL_PROFILING_COMMAND_START,  &t_strt);
     ev.getProfilingInfo(CL_PROFILING_COMMAND_END,    &t_end);

     if (!name) name = "";

     cout<< name << " : Queue  to Submit: " << t_sub-t_que  << " us" << endl;
     cout<< name << " : Submit to Start : " << t_strt-t_sub << " us" << endl;
     cout<< name << " : Start  to End   : " << t_end-t_strt << " us" << endl;
     cout<< endl;
}
