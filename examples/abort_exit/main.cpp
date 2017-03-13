/******************************************************************************
 * Copyright (c) 2016-2017, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <signal.h>
#include <time.h>
#include "ocl_util.h"
#include "kernel.dsp_h"

#ifdef _TI_RTOS
#include "../rtos_main.c"
#endif

using namespace cl;
using namespace std;

const int size   = 1 << 23;
const int wgsize = 1 << 20;

void run_kernel_wait(KernelFunctor&, Buffer&, int, const char*);
void run_task_nowait(KernelFunctor&, Buffer&, int, int, const char*);

#ifdef _TI_RTOS
void ocl_main(UArg arg0, UArg arg1)
{
  int    argc = (int)     arg0;
  char **argv = (char **) arg1;
  cl_char *ary = (cl_char *) __malloc_ddr(size);
  assert(ary != nullptr);
#else
#define RETURN(x) return x
cl_char ary [size];
int main(int argc, char *argv[])
{
#endif
  /*-------------------------------------------------------------------------
  * Catch ctrl-c so we ensure that we call dtors and the dsp is reset properly
  *------------------------------------------------------------------------*/
  signal(SIGABRT, exit);
  signal(SIGTERM, exit);
  memset(ary, 0, size);

  try
  {
    Context             context(CL_DEVICE_TYPE_ACCELERATOR);
    std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    Buffer              buf (context, CL_MEM_WRITE_ONLY, size);

    Program::Binaries   binary(1, make_pair(kernel_dsp_bin,
                                            sizeof(kernel_dsp_bin)));
    Program             program = Program(context, devices, binary);
    program.build(devices);

    CommandQueue  IOQ (context, devices[0]);
    CommandQueue  OOQ (context, devices[0],
                                      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

    Kernel        K (program, "devset");
    Kernel        T (program, "devset_t");
    KernelFunctor k_io = K.bind(IOQ, NDRange(size), NDRange(wgsize));
    KernelFunctor k_oo = K.bind(OOQ, NDRange(size), NDRange(wgsize));
    KernelFunctor t_io = T.bind(IOQ, NDRange(1), NDRange(1));
    KernelFunctor t_oo = T.bind(OOQ, NDRange(1), NDRange(1));

    srand(time(NULL));
    printf ("# k_io w   error:\n");
    run_kernel_wait(k_io, buf, rand() % size, "  # k_io w   error:");
    printf ("# k_io w/o error:\n");
    run_kernel_wait(k_io, buf, -1, "  # k_io w/o error:");
    printf ("# k_oo w   error:\n");
    run_kernel_wait(k_oo, buf, rand() % size, "  # k_oo w   error:");
    printf ("# k_oo w/o error:\n");
    run_kernel_wait(k_oo, buf, -1, "  # k_oo w/o error:");

    printf ("# t_io w/o error:\n");
    run_task_nowait(t_io, buf, size, -1, "  # t_io w/o error:");
    printf ("# t_io w   error:\n");
    run_task_nowait(t_io, buf, size, 0, "  # t_io w   error:");
    printf ("# t_oo w/o error:\n");
    run_task_nowait(t_oo, buf, size, -1, "  # t_oo w/o error:");
    printf ("# t_oo w   error:\n");
    run_task_nowait(t_oo, buf, size, 0, "  # t_oo w   error:");

    IOQ.finish();
    OOQ.finish();
    run_kernel_wait(k_io, buf, -1, "# final k_io w/o error:");
    IOQ.enqueueReadBuffer(buf, CL_TRUE, 0, size, ary);
  }
  catch (Error err)
  {
    cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
         << ocl_decode_error(err.err()) << ")" << endl;
    exit(-1);
  }

  for (int i = 0; i < size; ++i) assert(ary[i] == 'x');
#ifdef _TI_RTOS
  __free_ddr(ary);
#endif
  std::cout << "Done!" << std::endl;

  RETURN(0);
}

#define KERNELNAMELEN  32
typedef struct {
  int expect_error;
  char kernel_name[KERNELNAMELEN];
} callback_data_t;

void ev_complete_func(cl_event e, cl_int status, void *data)
{
  if (data != NULL)
  {
    int  expect_error = ((callback_data_t *) data)->expect_error;
    char *kernel_name = ((callback_data_t *) data)->kernel_name;
    if ((expect_error && (status != CL_ERROR_KERNEL_ABORT_TI &&
                          status != CL_ERROR_KERNEL_EXIT_TI)) ||
        (!expect_error && status < 0)) {
      cout << kernel_name << " Expecting kernel excution abort/exit error: "
           << (expect_error ? "true" : "false") << ", but got "
           << status << endl;
      free(data);
      exit(-1);
    }
    cout << kernel_name << " "
         << (expect_error ?
             ((status == CL_ERROR_KERNEL_ABORT_TI) ? "aborted" : "exited") :
            "finished") << endl;
    free(data);
  }
}

void run_kernel_wait(KernelFunctor& k, Buffer& buf, int abort_gid,
                     const char *name)
{
  Event ev;
  int expect_error = (abort_gid < 0) ? 0 : 1;
  try
  {
    ev = k(buf, abort_gid);
    ev.wait();
    cout << name << " finished" << endl;
    if (expect_error)
    {
        cout << "ERROR: expecting exit, but finished" << endl;
        exit(-1);
    }
  }
  catch (Error err)
  {
    cl_int status;
    ev.getInfo(CL_EVENT_COMMAND_EXECUTION_STATUS, &status);
    if (status == CL_ERROR_KERNEL_ABORT_TI)
        cout << name << " aborted" << endl;
    else if (status == CL_ERROR_KERNEL_EXIT_TI)
        cout << name << " exited" << endl;
    else
        cout << "ERROR: " << err.what() << "(" << err.err() << ", "
             << ocl_decode_error(err.err()) << ")" << endl;
    if (! expect_error)
    {
      cout << "ERROR: not expecting abort/exit, but error happened" << endl;
      exit(-1);
    }
    return;
  }
}

void run_task_nowait(KernelFunctor& k, Buffer& buf, int size, int exit_gid,
                     const char *name)
{
  Event ev;
  try
  {
    callback_data_t *data = (callback_data_t *) malloc(
                                                      sizeof(callback_data_t));
    if (data != NULL)
    {
      data->expect_error = (exit_gid < 0) ? 0 : 1;
      snprintf(data->kernel_name, KERNELNAMELEN, "%s", name);
    }
    ev = k(buf, size, exit_gid);
    ev.setCallback(CL_COMPLETE, ev_complete_func, data);
  }
  catch (Error err)
  {
    cl_int status;
    ev.getInfo(CL_EVENT_COMMAND_EXECUTION_STATUS, &status);
    if (status == CL_ERROR_KERNEL_ABORT_TI)
        cout << name << " aborted" << endl;
    else if (status == CL_ERROR_KERNEL_EXIT_TI)
        cout << name << " exited" << endl;
    else
      cout << "ERROR: " << err.what() << "(" << err.err() << ", "
           << ocl_decode_error(err.err()) << ")" << endl;
    return;
  }
}
