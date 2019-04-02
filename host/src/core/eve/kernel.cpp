/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include "kernel.h"
#include "device.h"
#include "buffer.h"
#include "program.h"
#include "utils.h"
#include "u_locks_pthread.h"
#include "device_info.h"
#include "dspmem.h"

#include "../kernel.h"
#include "../memobject.h"
#include "../events.h"
#include "../program.h"
#include "../oclenv.h"
#include "../error_report.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#ifndef _SYS_BIOS
#include <sys/mman.h>
#else
#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))
#define MAN(X, Y)  ((X) > (Y) ? (X) : (Y))
#endif
#include <sys/param.h>


#define ROUNDUP(val, pow2)   (((val) + (pow2) - 1) & ~((pow2) - 1))

using namespace Coal;
using namespace tiocl;


EVEKernel::EVEKernel(EVEDevice *device, Kernel *kernel,
                     KernelEntry *kernel_entry)
: DeviceKernel(), p_device(device), p_kernel(kernel), p_kernel_entry(kernel_entry)
{
}

EVEKernel::~EVEKernel()
{
}

cl_uint EVEKernel::builtin_kernel_index()
{
    return p_kernel_entry->index;
}

/******************************************************************************
* void EVEKernel::preAllocBuffers()
******************************************************************************/
cl_int EVEKernel::preAllocBuffers()
{
    for (unsigned int i=0; i < kernel()->numArgs(); ++i)
    {
        const Kernel::Arg &arg = kernel()->arg(i);

        if (arg.kind() == Kernel::Arg::Buffer &&
            arg.file() != Kernel::Arg::Local)
        {
            MemObject *buffer = *(MemObject **)arg.data();
            if (buffer && !buffer->allocate(device()))
                return CL_MEM_OBJECT_ALLOCATION_FAILURE;
        }
    }
    return CL_SUCCESS;
}


static int kernelID = 0;

/*=============================================================================
* EVEKernelEvent
*============================================================================*/
EVEKernelEvent::EVEKernelEvent(EVEDevice *device, KernelEvent *event)
: p_ret_code(CL_SUCCESS),
  p_device(device), p_event(event), p_kernel((EVEKernel*)event->deviceKernel())
{
    p_kernel_id = __sync_fetch_and_add(&kernelID, 1);

    p_ret_code  = callArgs(EVE_MAX_ARGS_ON_STACK_SIZE);

    /*-------------------------------------------------------------------------
    * Populate some of the kernel_msg_t structure.
    *------------------------------------------------------------------------*/
    p_msg.command           = TASK;
    p_msg.u.k_eve.Kernel_id = p_kernel_id;
    p_msg.u.k_eve.builtin_kernel_index = (unsigned) p_kernel->builtin_kernel_index();
}

EVEKernelEvent::~EVEKernelEvent() { }

#define DEVICE_READ_ONLY(buffer)  (buffer->flags() & CL_MEM_READ_ONLY)
#define DEVICE_WRITE_ONLY(buffer) (buffer->flags() & CL_MEM_WRITE_ONLY)
#define HOST_NO_ACCESS(buffer)    (buffer->flags() & CL_MEM_HOST_NO_ACCESS)
#define HOST_READ_ONLY(buffer)    (buffer->flags() & CL_MEM_HOST_READ_ONLY)
#define HOST_WRITE_ONLY(buffer)   (buffer->flags() & CL_MEM_HOST_WRITE_ONLY)

#define SETMOREARG(sz, pval) do \
    { \
        more_arg_offset = ROUNDUP(more_arg_offset, sz); \
        memcpy(args_on_stack+more_arg_offset, pval, sz); \
        more_arg_offset += sz; \
    } while(0)

static inline void setarg_inreg(int index, int sz, unsigned int *args_in_reg,
                                void *pval)
{
    memcpy(&args_in_reg[index], pval, sz);  // Little Endian!!!
}

/******************************************************************************
* EVEKernelEvent::callArgs
******************************************************************************/
cl_int EVEKernelEvent::callArgs(unsigned max_args_size)
{
    int num_args_in_reg    = 0;
    int args_on_stack_size = 0;
    int more_arg_offset    = 0;  // top of stack reseved on EVE side
    unsigned  *args_in_reg = (unsigned*)p_msg.u.k_eve.args_in_reg;
    memset(args_in_reg, 0, sizeof(p_msg.u.k.kernel.args_in_reg));
    uint8_t   *args_on_stack = (uint8_t*)p_msg.u.k_eve.args_on_stack;
    memset(args_on_stack, 0, sizeof(p_msg.u.k_eve.args_on_stack));

    /*-------------------------------------------------------------------------
    * Write Arguments
    *------------------------------------------------------------------------*/
    for (int i = 0;  i < p_kernel->kernel()->numArgs(); ++i)
    {
        const Kernel::Arg & arg  = p_kernel->kernel()->arg(i);
        size_t              size = arg.vecValueSize();

        if (size == 0)
        {
            ReportError(ErrorType::FatalNoExit, ErrorKind::KernelArgSizeZero);
            return(CL_INVALID_ARG_SIZE);
        }
        if (size > 4)
        {
            ReportError(ErrorType::FatalNoExit,
                        ErrorKind::KernelArgSizeTooBig, 4, "EVE");
            return(CL_INVALID_ARG_SIZE);
        }
        if (more_arg_offset + 4 >= EVE_MAX_ARGS_ON_STACK_SIZE)
        {
            ReportError(ErrorType::FatalNoExit,
                        ErrorKind::KernelArgSizesMaxExceeded, 128, "EVE");
            return(CL_INVALID_KERNEL_ARGS);
        }

        /*---------------------------------------------------------------------
        * We may have to perform some changes in the values (buffers, etc)
        *--------------------------------------------------------------------*/
        switch (arg.kind())
        {
            case Kernel::Arg::Buffer:
            {
                MemObject    *buffer = 0;
                DSPDevicePtr buf_ptr = 0;
                if (arg.data()) buffer = *(MemObject **)arg.data();

                DSPVirtPtr *buf_dspvirtptr =
                                 (num_args_in_reg < EVE_MAX_ARGS_IN_REG_SIZE) ?
                              (DSPVirtPtr *)(&args_in_reg[num_args_in_reg]) :
                   (DSPVirtPtr *)(args_on_stack+ROUNDUP(more_arg_offset,4));

                /*-------------------------------------------------------------
                * Alloc a buffer and pass it to the kernel
                *------------------------------------------------------------*/
                if (arg.file() == Kernel::Arg::Local)
                {
                    // TODO: should not happen
                    assert(0);
                }
                else if (buffer != NULL)
                {
                    /*---------------------------------------------------------
                    * Get the DSP buffer, allocate it and get its pointer
                    *--------------------------------------------------------*/
                    if (  (buffer->flags() & CL_MEM_USE_HOST_PTR) &&
                        ! buffer->get_host_ptr_clMalloced())
                    {
                        p_hostptr_tmpbufs.push_back(
                           HostptrPair(buffer, DSPPtrPair(0, buf_dspvirtptr)));
                    }
                    else
                    {
                        DSPBuffer *dspbuf =
                                   (DSPBuffer *)buffer->deviceBuffer(p_device);
                        buffer->allocate(p_device);
                        DSPDevicePtr64 addr64 = dspbuf->data();

                        if (addr64 < 0xFFFFFFFF)
                            buf_ptr = addr64;
                        else
                        {
                            // TODO: not supported
                            assert(0);
                        }

                        if (buffer->get_host_ptr_clMalloced())
                            p_hostptr_clMalloced_bufs.push_back(buffer);
                    }
                }

                /*---------------------------------------------------------
                * Use 0 for temporary buffer here, it will be overwritten
                * with allocated local buffer address at kernel dispatch time.
                *--------------------------------------------------------*/
                if (num_args_in_reg < EVE_MAX_ARGS_IN_REG_SIZE)
                    setarg_inreg(num_args_in_reg++, 4, args_in_reg, &buf_ptr);
                else
                    SETMOREARG(4, &buf_ptr);

                break;
            }

            case Kernel::Arg::Image2D:
            case Kernel::Arg::Image3D:
                {
                    ReportError(ErrorType::FatalNoExit,
                                ErrorKind::KernelArgImageNotSupported);
                    return(CL_INVALID_KERNEL_ARGS);
                }
                break;

            /*-----------------------------------------------------------------
            * Non-Buffers
            *----------------------------------------------------------------*/
            default:
                if (num_args_in_reg < EVE_MAX_ARGS_IN_REG_SIZE)  // args_in_reg
                {
                    int dummy = 0;

                    if (arg.is_subword_int_uns())
                    {
                        if (size == 1)
                            dummy = (unsigned) *((unsigned char*)arg.data());
                        else if (size == 2)
                            dummy = (unsigned) *((unsigned short*)arg.data());
                    }
                    else
                    {
                        if (size == 1)
                            dummy = (int) *((signed char*)arg.data());
                        else if (size == 2)
                            dummy = (int) *((short*)arg.data());
                    }

                    void *p_data = (dummy == 0) ? (void*)arg.data()
                                                : (void *) &dummy;
                    size         = (dummy == 0) ? size : 4;

                    setarg_inreg(num_args_in_reg++, size, args_in_reg, p_data);
                }
                else // args_on_stack
                {
                    SETMOREARG(size, arg.data());
                }

                break;
        }
    }

    p_msg.u.k_eve.args_on_stack_size = ROUNDUP(more_arg_offset, 4);

    return CL_SUCCESS;
}


/******************************************************************************
* bool EVEKernelEvent::run()
******************************************************************************/
cl_int EVEKernelEvent::run(Event::Type evtype)
{
    Program    *p    = (Program *)p_kernel->kernel()->parent();
    EVEProgram *prog = (EVEProgram *)(p->deviceDependentProgram(p_device));

    cl_int err = CL_SUCCESS;

    /*-------------------------------------------------------------------------
    * Allocate temporary global buffer for non-clMalloced USE_HOST_PTR
    *------------------------------------------------------------------------*/
    err = allocate_temp_global();
    if (err != CL_SUCCESS) return err;

    /*-------------------------------------------------------------------------
    * Ensure that __malloc_xxx USE_HOST_PTR buffers are flushed from cache
    *------------------------------------------------------------------------*/
    err = flush_special_use_host_ptr_buffers();
    if (err != CL_SUCCESS) return err;

    /*---------------------------------------------------------------------
    * Order!! After switching to two worker threads, we must push the complete
    * pending first, then send out the mails, to prevent the extrememly fast
    * EVE reply that has no corresponding complete pending from happening.
    *--------------------------------------------------------------------*/
    p_device->push_complete_pending(p_kernel_id, p_event,
                                    p_device->numHostMails(p_msg));
    p_device->mail_to(p_msg);

    /*-------------------------------------------------------------------------
    * Do not wait for completion
    *------------------------------------------------------------------------*/
    return CL_SUCCESS;
}

/******************************************************************************
* Allocate temporary global buffer for non-clMalloced USE_HOST_PTR
******************************************************************************/
cl_int EVEKernelEvent::allocate_temp_global(void)
{
    SharedMemory *shm = p_device->GetSHMHandler();

    for (int i = 0; i < p_hostptr_tmpbufs.size(); ++i)
    {
        MemObject      *buffer       =  p_hostptr_tmpbufs[i].first;
        DSPDevicePtr64 *p_addr64     = &p_hostptr_tmpbufs[i].second.first;
        DSPVirtPtr     *p_arg_word   =  p_hostptr_tmpbufs[i].second.second;

        *p_addr64 = shm->AllocateGlobal(buffer->size(), false);

        if (!(*p_addr64))
        {
            ReportError(ErrorType::FatalNoExit,
                        ErrorKind::TempMemAllocationFailed);
            return(CL_MEM_OBJECT_ALLOCATION_FAILURE);
        }

        if (*p_addr64 < 0xFFFFFFFF)
            *p_arg_word = *p_addr64;
        else
            assert(0);  // TODO

        if (! DEVICE_WRITE_ONLY(buffer))
        {
            void *mapped_tmpbuf = shm->Map(*p_addr64, buffer->size(), false);
            memcpy(mapped_tmpbuf, buffer->host_ptr(), buffer->size());
            shm->Unmap(mapped_tmpbuf, *p_addr64, buffer->size(), true);
        }
    }

    return CL_SUCCESS;
}

/*-------------------------------------------------------------------------
* Ensure that __malloc_xxx USE_HOST_PTR buffers are flushed from cache
*------------------------------------------------------------------------*/
cl_int EVEKernelEvent::flush_special_use_host_ptr_buffers(void)
{
    SharedMemory *shm = p_device->GetSHMHandler();

    /*-------------------------------------------------------------------------
    * PSDK3.1/CMEM4.12: reverts back to pre-PSDK3.0/pre-CMEM4.11 implementation
    *------------------------------------------------------------------------*/
    int total_buf_size = 0;
    for (int i = 0; i < p_hostptr_clMalloced_bufs.size(); ++i)
    {
        MemObject *buffer = p_hostptr_clMalloced_bufs[i];
         // Exclude buffers not accessed
        if (HOST_NO_ACCESS(buffer) || HOST_READ_ONLY(buffer)) continue;
        total_buf_size += buffer->size();
    }

    /*-------------------------------------------------------------------------
    * Threshold is 32MB.
    *------------------------------------------------------------------------*/
    int  threshold  = CMEM_THRESHOLD;
    bool wb_inv_all = false;
    if (total_buf_size >= threshold)  wb_inv_all = shm->CacheWbInvAll();

    if (! wb_inv_all)
    {
        for (int i = 0; i < p_hostptr_clMalloced_bufs.size(); ++i)
        {
            MemObject *buffer = p_hostptr_clMalloced_bufs[i];
            DSPBuffer *dspbuf = (DSPBuffer *) buffer->deviceBuffer(p_device);
            DSPDevicePtr64 data = (DSPDevicePtr64)dspbuf->data();

            if (! DEVICE_READ_ONLY(buffer) &&
                ! HOST_NO_ACCESS(buffer)   &&
                ! HOST_READ_ONLY(buffer))
                shm->CacheWbInv(data, buffer->host_ptr(), buffer->size());
            else if (! DEVICE_WRITE_ONLY(buffer) &&
                     ! HOST_NO_ACCESS(buffer)    &&
                     ! HOST_READ_ONLY(buffer))
                shm->CacheWb(data, buffer->host_ptr(), buffer->size());
        }
    }

    return CL_SUCCESS;
}


/******************************************************************************
* free_tmp_bufs allocated for kernel allocas, and for use_host_ptr
******************************************************************************/
void EVEKernelEvent::free_tmp_bufs()
{
    SharedMemory *shm = p_device->GetSHMHandler();

    for (int i = 0; i < p_hostptr_tmpbufs.size(); ++i)
    {
        MemObject *buffer     = p_hostptr_tmpbufs[i].first;
        DSPDevicePtr64 addr64 = p_hostptr_tmpbufs[i].second.first;

        if (! DEVICE_READ_ONLY(buffer))
        {
            void *mapped_tmpbuf = shm->Map(addr64, buffer->size(), true);
            memcpy(buffer->host_ptr(), mapped_tmpbuf, buffer->size());
            shm->Unmap(mapped_tmpbuf, addr64, buffer->size(), false);
        }
        shm->FreeGlobal(addr64);
    }

    /*-------------------------------------------------------------------------
    * Cache-Inv mapped buffers for clMalloced USE_HOST_PTR
    * CHANGE: We do cacheWbInv before dispatching kernel for !READ_ONLY
    * CHANGE AGAIN: On AM57, we see (7 out of 8000) failures that 1 or 2
    *               previously invalidated cache line got back into cache,
    *               so we inv again here
    * Linux 4.4.12 / CMEM 4.11: CacheInv -> transfer ownership back to cpu
    *------------------------------------------------------------------------*/
    // /***
    for (int i = 0; i < p_hostptr_clMalloced_bufs.size(); ++i)
    {
        MemObject *buffer = p_hostptr_clMalloced_bufs[i];
        DSPBuffer *dspbuf = (DSPBuffer *) buffer->deviceBuffer(p_device);
        DSPDevicePtr64 data = (DSPDevicePtr64)dspbuf->data();
        if (! DEVICE_READ_ONLY(buffer))
            shm->CacheInv(data, buffer->host_ptr(), buffer->size());
    }
    // ***/
}

