/******************************************************************************
 * Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
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
    CommandQueue  *tIOQ = NULL;
    CommandQueue  *tOOQ = NULL;
    cl_command_queue_properties devq_prop;
    devices[0].getInfo(CL_DEVICE_QUEUE_PROPERTIES, &devq_prop);
    if ((devq_prop & CL_QUEUE_KERNEL_TIMEOUT_COMPUTE_UNIT_TI) != 0)
    {
      tIOQ = new CommandQueue(context, devices[0],
                                    CL_QUEUE_KERNEL_TIMEOUT_COMPUTE_UNIT_TI);
      tOOQ = new CommandQueue(context, devices[0],
                                    CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                                    CL_QUEUE_KERNEL_TIMEOUT_COMPUTE_UNIT_TI);
    }

    Kernel        K (program, "devset");
    Kernel        T (program, "devset_t");
    KernelFunctor k_io = K.bind(IOQ, NDRange(size), NDRange(wgsize));
    KernelFunctor k_oo = K.bind(OOQ, NDRange(size), NDRange(wgsize));
    KernelFunctor t_io = T.bind(IOQ, NDRange(1), NDRange(1));
    KernelFunctor t_oo = T.bind(OOQ, NDRange(1), NDRange(1));

    printf ("# k_io w/o timeout:\n");
    run_kernel_wait(k_io, buf, -1, "  # k_io w/o timeout:");
    printf ("# k_oo w/o timeout:\n");
    run_kernel_wait(k_oo, buf, -1, "  # k_oo w/o timeout:");
    if (tIOQ != NULL && tOOQ != NULL)
    {
      __ti_set_kernel_timeout_ms(K(), 100);
      KernelFunctor k_io_t = K.bind(*tIOQ, NDRange(size), NDRange(wgsize));
      KernelFunctor k_oo_t = K.bind(*tOOQ, NDRange(size), NDRange(wgsize));
      printf ("# k_io w   timeout:\n");
      run_kernel_wait(k_io_t, buf, 70, "  # k_io w   timeout:");
      printf ("# k_oo w   timeout:\n");
      run_kernel_wait(k_oo_t, buf, 70, "  # k_oo w   timeout:");
    }

    printf ("# t_io w/o timeout:\n");
    run_task_nowait(t_io, buf, size, -1, "  # t_io w/o timeout:");
    printf ("# t_oo w/o timeout:\n");
    run_task_nowait(t_oo, buf, size, -1, "  # t_oo w/o timeout:");
    if (tIOQ != NULL && tOOQ != NULL)
    {
      __ti_set_kernel_timeout_ms(T(), 100);
      KernelFunctor t_io_t = T.bind(*tIOQ, NDRange(1), NDRange(1));
      KernelFunctor t_oo_t = T.bind(*tOOQ, NDRange(1), NDRange(1));
      printf ("# t_io w   timeout:\n");
      run_task_nowait(t_io_t, buf, size, 70, "  # t_io w   timeout:");
      printf ("# t_oo w   timeout:\n");
      run_task_nowait(t_oo_t, buf, size, 70, "  # t_oo w   timeout:");
      tIOQ->finish();
      tOOQ->finish();
    }

    IOQ.finish();
    OOQ.finish();
    run_kernel_wait(k_io, buf, -1, "# final k_io w/o timeout:");
    IOQ.enqueueReadBuffer(buf, CL_TRUE, 0, size, ary);

    delete tIOQ;
    delete tOOQ;
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
  int expect_timeout;
  char kernel_name[KERNELNAMELEN];
} callback_data_t;

void ev_complete_func(cl_event e, cl_int status, void *data)
{
  if (data != NULL)
  {
    int  expect_timeout = ((callback_data_t *) data)->expect_timeout;
    char *kernel_name = ((callback_data_t *) data)->kernel_name;
    if ((expect_timeout && status != CL_ERROR_KERNEL_TIMEOUT_TI) ||
        (!expect_timeout && status < 0)) {
      cout << kernel_name << " Expecting kernel execution timeout: "
           << (expect_timeout ? "true" : "false") << ", but got "
           << status << endl;
      free(data);
      exit(-1);
    }
    cout << kernel_name << (expect_timeout ? " terminated due to timeout"
                                           : " finished") << endl;
    free(data);
  }
}

void run_kernel_wait(KernelFunctor& k, Buffer& buf, int timeout_flag,
                     const char *name)
{
  Event ev;
  int expect_timeout = (timeout_flag <= 0) ? 0 : 1;
  try
  {
    ev = k(buf, timeout_flag);
    ev.wait();
    cout << name << " finished" << endl;
    if (expect_timeout)
    {
      cout << "ERROR: expecting timeout, but finished" << endl;
      exit(-1);
    }
  }
  catch (Error err)
  {
    cl_int status;
    ev.getInfo(CL_EVENT_COMMAND_EXECUTION_STATUS, &status);
    if (status == CL_ERROR_KERNEL_TIMEOUT_TI)
      cout << name << " terminated due to timeout" << endl;
    else
      cout << "ERROR: " << err.what() << "(" << err.err() << ", "
           << ocl_decode_error(err.err()) << ")" << endl;
    if (! expect_timeout)
    {
      cout << "ERROR: not expecting timeout, but error happened" << endl;
      exit(-1);
    }
    return;
  }
}

void run_task_nowait(KernelFunctor& k, Buffer& buf, int size, int timeout_flag,
                     const char *name)
{
  Event ev;
  try
  {
    callback_data_t *data = (callback_data_t *) malloc(
                                                  sizeof(callback_data_t));
    if (data != NULL)
    {
        data->expect_timeout = (timeout_flag <= 0) ? 0 : 1;
        snprintf(data->kernel_name, KERNELNAMELEN, "%s", name);
    }
    ev = k(buf, size, timeout_flag);
    ev.setCallback(CL_COMPLETE, ev_complete_func, data);
  }
  catch (Error err)
  {
    cl_int status;
    ev.getInfo(CL_EVENT_COMMAND_EXECUTION_STATUS, &status);
    if (status == CL_ERROR_KERNEL_TIMEOUT_TI)
        cout << name << " terminated due to timeout" << endl;
    else
        cout << "ERROR: " << err.what() << "(" << err.err() << ", "
             << ocl_decode_error(err.err()) << ")" << endl;
    return;
  }
}
