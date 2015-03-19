#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <cstdlib>

using namespace cl;
using std::cout;
using std::cerr;
using std::endl;

void report_event_timing(const Event &ev, const char* name);

const char * kernelStr =
    "int DSP_dotprod (global short* x, global short* y, int nx); "

    "kernel void DSP_dotprod_Facade(global const short* x, "
    "                               global const short* y, "
    "                               int nx, "
    "                               int *result) "
    "{ *result = DSP_dotprod(x,y,nx); }" ;

const int NumElements     = 4*1024*1024;
const int NumWorkGroups   = 1; 
const int WorkGroupSize   = NumElements / NumWorkGroups;

cl_short srcX  [NumElements];
cl_short srcY  [NumElements];
int Result;
int Golden;

int main(int argc, char *argv[])
{
   cl_int err     = CL_SUCCESS;
   int    bufsize = sizeof(srcX);
   Result = Golden = 0;

   for (int i=0; i < NumElements; ++i) 
   { 
       srcX[i]   = srcY[i] = i; 
       Golden   += srcY[i] * srcX[i]; 
   }

   try 
   {
     Context             context(CL_DEVICE_TYPE_ACCELERATOR); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     Buffer bufX   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufY   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufRes (context, CL_MEM_WRITE_ONLY, sizeof(int));

     Program::Sources    source(1, std::make_pair(kernelStr,strlen(kernelStr)));
     Program             program = Program(context, source);
     program.build(devices, "/opt/ti/dsplib_c66x_3_1_0_0/lib/dsplib.ae66"); 

     Kernel fkernel(program, "DSP_dotprod_Facade");
     fkernel.setArg(0, bufX);
     fkernel.setArg(1, bufY);
     fkernel.setArg(2, NumElements);
     fkernel.setArg(3, bufRes);

     Event ev1,ev2,ev3,ev4, ev5, ev6;

     CommandQueue Q(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

     Q.enqueueWriteBuffer(bufX, CL_TRUE, 0, bufsize, srcX, NULL, &ev1);
     Q.enqueueWriteBuffer(bufY, CL_TRUE, 0, bufsize, srcY, NULL, &ev2);
     Q.enqueueTask(fkernel, NULL, &ev6);
     Q.enqueueReadBuffer (bufRes, CL_TRUE, 0, sizeof(int), &Result, NULL, &ev4);

     report_event_timing(ev1, "Write BufX ");
     report_event_timing(ev2, "Write BufY ");
     report_event_timing(ev6, "DotProd Exec");
     report_event_timing(ev4, "Read BufRes");
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   if (Result == Golden) cout << "Success!" << endl; 
   else                  cout << "Failed!"  << endl; 
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
