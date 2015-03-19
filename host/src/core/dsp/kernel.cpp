#include "kernel.h"
#include "device.h"
#include "buffer.h"
#include "program.h"
#include "utils.h"
#include "u_locks_pthread.h"
#include "mailbox.h"
#include "message.h"

#include "../kernel.h"
#include "../memobject.h"
#include "../events.h"
#include "../program.h"

#include <llvm/Function.h>
#include <llvm/Constants.h>
#include <llvm/Instructions.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <sys/mman.h>

#define ERR(x) std::cerr << x << std::endl
#define ERROR() std::cerr << "Unknown error in dsp/kernel.cpp" << std::endl


using namespace Coal;

DSPKernel::DSPKernel(DSPDevice *device, Kernel *kernel)
: DeviceKernel(), p_device(device), p_kernel(kernel), 
    p_device_entry_pt((DSPDevicePtr)0),
    p_data_page_ptr  ((DSPDevicePtr)0xffffffff)
{
}

DSPKernel::~DSPKernel()
{
}


template<typename T>
T k_exp(T base, unsigned int e)
{
    T rs = base;
    for (unsigned int i=1; i<e; ++i) rs *= base;
    return rs;
}

/*-----------------------------------------------------------------------------
* This and the next function are called from the multiple worker threads. They
* may all enter the set the name section, but they will all set the same value,
* so even though there is a race, there is no race error. when work group 
* division is pushed down to the dsp, the race will go away.
*----------------------------------------------------------------------------*/
DSPDevicePtr  DSPKernel::device_entry_pt() 
{ 
    if (!p_device_entry_pt) 
    {
        size_t name_length;
        p_kernel->info(CL_KERNEL_FUNCTION_NAME, 0, 0, &name_length);

        void *name = malloc(name_length);
        p_kernel->info(CL_KERNEL_FUNCTION_NAME, name_length, name, 0);

        Program    *p     = (Program *)p_kernel->parent();
        DSPProgram *prog  = (DSPProgram *)(p->deviceDependentProgram(p_device));

        if (!prog->is_loaded()) ERROR();
        p_device_entry_pt = prog->query_symbol((char*)name);
        free (name);
    }
    return p_device_entry_pt; 
}

/******************************************************************************
* The data page pointer can frequently be 0, so we will initialize it to be 
* 0xffffffff as a start value instead of 0.
******************************************************************************/
DSPDevicePtr  DSPKernel::data_page_ptr()
{ 
    if (p_data_page_ptr == (DSPDevicePtr)0xffffffff) 
    {
        Program    *p     = (Program *)p_kernel->parent();
        DSPProgram *prog  = (DSPProgram *)(p->deviceDependentProgram(p_device));

        if (!prog->is_loaded()) ERROR();
        //p_data_page_ptr = prog->query_symbol("__TI_STATIC_BASE");
        p_data_page_ptr = prog->data_page_ptr();
    }
    return p_data_page_ptr; 
}

/******************************************************************************
* void DSPKernel::preAllocBuffers()
******************************************************************************/
void DSPKernel::preAllocBuffers()
{
    for (unsigned int i=0; i < kernel()->numArgs(); ++i)
    {
        const Kernel::Arg &arg = kernel()->arg(i);

        if (arg.kind() == Kernel::Arg::Buffer &&
            arg.file() != Kernel::Arg::Local)
        {
            MemObject *buffer = *(MemObject **)arg.data();
            if (buffer) buffer->allocate(device());
        }
    }
}


/******************************************************************************
* Try to find the size a work group needs to be executed the fastest on the DSP.
******************************************************************************/
size_t DSPKernel::guessWorkGroupSize(cl_uint num_dims, cl_uint dim,
                                     size_t global_work_size) const
{
    unsigned int dsps = p_device->numDSPs();

    /*-------------------------------------------------------------------------
    * Don't break in too small parts
    *------------------------------------------------------------------------*/
    if (k_exp(global_work_size, num_dims) > 64)
        return global_work_size;

    /*-------------------------------------------------------------------------
    * Find the divisor of global_work_size the closest to dsps but >= than it
    *------------------------------------------------------------------------*/
    unsigned int divisor = dsps;

    while (true)
    {
        if ((global_work_size % divisor) == 0)
            break;

        /*---------------------------------------------------------------------
        * Don't let the loop go up to global_work_size, the overhead would be
        * too huge
        *--------------------------------------------------------------------*/
        if (divisor > global_work_size || divisor > dsps * 32)
        {
            divisor = 1;  // Not parallel but has no CommandQueue overhead
            break;
        }
    }

    /*-------------------------------------------------------------------------
    * Return the size
    *------------------------------------------------------------------------*/
    return global_work_size / divisor;
}

Kernel *         DSPKernel::kernel()   const { return p_kernel; }
DSPDevice *      DSPKernel::device()   const { return p_device; }

// From Wikipedia : http://www.wikipedia.org/wiki/Power_of_two#Algorithm_to_round_up_to_power_of_two
template <class T>
T next_power_of_two(T k) 
{
    if (k == 0) return 1;

    k--;
    for (int i=1; i<sizeof(T)*8; i<<=1)
            k = k | k >> i;
    return k+1;
}

size_t DSPKernel::typeOffset(size_t &offset, size_t type_len)
{
    size_t rs = offset;

    // Align offset to stype_len
    type_len = next_power_of_two(type_len);
    if (type_len > 8) type_len = 8; // The c66 has no alignment need > 8 bytes

    size_t mask = ~(type_len - 1);

    while (rs & mask != rs)
        rs++;

    // Where to try to place the next value
    offset = rs + type_len;

    return rs;
}

/*=============================================================================
* DSPKernelEvent
*============================================================================*/
DSPKernelEvent::DSPKernelEvent(DSPDevice *device, KernelEvent *event)
: p_device(device), p_event(event), p_kernel((DSPKernel*)event->deviceKernel()) { }

DSPKernelEvent::~DSPKernelEvent() { }

#define READABLE_BUFFER(buffer) (buffer->flags() & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY))
/******************************************************************************
* DSPKernelEvent::callArgs
******************************************************************************/
int DSPKernelEvent::callArgs(
        unsigned                  * args_in_mem, 
        std::vector<DSPDevicePtr> * local_bufs,
        std::vector<DSPMemRange>  * flush_bufs)
{
    int arg_words = 0;

    /*-------------------------------------------------------------------------
    * Currently do not handle kernel arguments passed on stack, so therefore
    * we are limited to the nu,ber of arguments that are passed in registers,
    * i.e. 10.
    *------------------------------------------------------------------------*/
    if (p_kernel->kernel()->numArgs() > 10) ERR("Too many Kernel Arguments");

    /*-------------------------------------------------------------------------
    * Write Arguments
    *------------------------------------------------------------------------*/
    for (int i = 0;  i < p_kernel->kernel()->numArgs(); ++i)
    {
        const Kernel::Arg & arg  = p_kernel->kernel()->arg(i);
        size_t              size = arg.valueSize() * arg.vecDim();

        if (size == 0) ERR("Kernel Argument has size == 0");
        if (size < 4) size = 4;
        if (size != 4 && size != 8) ERR("Invalid Kernel Argument size");

        /*---------------------------------------------------------------------
        * We may have to perform some changes in the values (buffers, etc)
        *--------------------------------------------------------------------*/
        switch (arg.kind())
        {
            case Kernel::Arg::Buffer:
            {
                MemObject *buffer = *(MemObject **)arg.data();
                if (args_in_mem) args_in_mem[arg_words] = sizeof(DSPDevicePtr);
                arg_words++;

                /*-------------------------------------------------------------
                * Alloc a buffer and pass it to the kernel
                *------------------------------------------------------------*/
                if (arg.file() == Kernel::Arg::Local)
                {
                    int          lbufsz = arg.allocAtKernelRuntime();
                    DSPDevicePtr lbuf   = p_device->malloc_l2(lbufsz);

                    if (!lbuf) ERR("Could not alloc local memory");
                    else 
                    {
                        if (local_bufs) local_bufs->push_back(lbuf);
                        if (flush_bufs && READABLE_BUFFER(buffer))
                            flush_bufs->push_back(DSPMemRange(lbuf, lbufsz));
                    }
                    if (args_in_mem) args_in_mem[arg_words] = (unsigned)lbuf;
                    arg_words++;
                }
                else if (!buffer) 
                {
                    if (args_in_mem) args_in_mem[arg_words] = 0;
                    arg_words++;
                }
                else
                {
                    /*---------------------------------------------------------
                    * Get the DSP buffer, allocate it and get its pointer
                    *--------------------------------------------------------*/
                    DSPBuffer    *dspbuf = (DSPBuffer *)buffer->deviceBuffer(p_device);
                    DSPDevicePtr *buf_ptr = 0;

                    buffer->allocate(p_device);
                    buf_ptr = dspbuf->data();

                    if (args_in_mem) args_in_mem[arg_words] = (unsigned)buf_ptr;
                    arg_words++;

                    if (flush_bufs && READABLE_BUFFER(buffer))
                        flush_bufs->push_back(DSPMemRange(buf_ptr, buffer->size()));
                }
                break;
            }

            case Kernel::Arg::Image2D:
            case Kernel::Arg::Image3D: ERR("Images not yet supported"); break;

            /*-----------------------------------------------------------------
            * Non-Buffers 
            *----------------------------------------------------------------*/
            default:
                if (args_in_mem) args_in_mem[arg_words] = size;
                arg_words++;

                if (args_in_mem) args_in_mem[arg_words] = *((unsigned*)arg.data()); 
                arg_words++;

                if (size == 8)
                {
                    if (args_in_mem) args_in_mem[arg_words] = *((unsigned*)arg.data() + 4); 
                    arg_words++;
                }
                break;
        }
    }
    if (args_in_mem) args_in_mem[arg_words] = 0;  // 0 terminator for args area
    arg_words++;

    return arg_words;
}

/******************************************************************************
* bool DSPKernelEvent::run()
******************************************************************************/
bool DSPKernelEvent::run()
{
#if 0
    Program    *p    = (Program *)p_kernel->kernel()->parent();
    DSPProgram *prog = (DSPProgram *)(p->deviceDependentProgram(p_device));
    // perhaps ensure that prog is loaded.
#endif

    std::vector<DSPDevicePtr> local_bufs;
    std::vector<DSPMemRange>  flush_bufs;

    /*-------------------------------------------------------------------------
    * Determine how much area the args will need, and 
    * Account for the KernelConfig structure as well.
    *------------------------------------------------------------------------*/
    int num_arg_words = callArgs(NULL, &local_bufs, &flush_bufs);
    int words_in_KernelConfig = sizeof(KernelConfig) / sizeof(unsigned);
    num_arg_words += words_in_KernelConfig;

    /*-------------------------------------------------------------------------
    * Allocate host memory area for the args
    *------------------------------------------------------------------------*/
    unsigned *     host_args_ptr = new unsigned [num_arg_words];
    unsigned *     arg_area      = &host_args_ptr[words_in_KernelConfig];
    KernelConfig * cfg           = (KernelConfig*)(host_args_ptr);
    int            dim           = p_event->work_dim();

    if (!host_args_ptr) ERR("OUT OF HOST MEMORY");

    /*-------------------------------------------------------------------------
    * Populate the config area in memory
    *------------------------------------------------------------------------*/
    cfg->dims              = dim;
    cfg->global_size_0     = p_event->global_work_size(0);
    cfg->global_size_1     = dim > 1 ? p_event->global_work_size(1) : 1;
    cfg->global_size_2     = dim > 2 ? p_event->global_work_size(2) : 1;
    cfg->local_size_0      = p_event->local_work_size(0);
    cfg->local_size_1      = dim > 1 ? p_event->local_work_size(1) : 1;
    cfg->local_size_2      = dim > 2 ? p_event->local_work_size(2) : 1;
    cfg->global_offset_0   = p_event->global_work_offset(0);
    cfg->global_offset_1   = p_event->global_work_offset(1);
    cfg->global_offset_2   = p_event->global_work_offset(2);
    cfg->WG_gid_start_0    = 0;
    cfg->WG_gid_start_1    = 0;
    cfg->WG_gid_start_2    = 0;
    cfg->entry_point       = (unsigned)p_kernel->device_entry_pt();
    cfg->data_page_ptr     = (unsigned)p_kernel->data_page_ptr();

    /*-------------------------------------------------------------------------
    * Populate the arg area in memory
    *------------------------------------------------------------------------*/
    callArgs(arg_area, NULL, NULL);

    /*-------------------------------------------------------------------------
    * Write the args and config into target memory
    *------------------------------------------------------------------------*/
    size_t       device_size = num_arg_words * sizeof(unsigned);
    DSPDevicePtr device_addr = p_device->malloc_ddr(device_size);
    if (!device_addr) ERR("OUT OF TARGET MEMORY");

    p_device->dma_write(device_addr, host_args_ptr, device_size);

    /*-------------------------------------------------------------------------
    * Create a message for the DSP
    *------------------------------------------------------------------------*/
    Msg_t msg;
    msg.number_commands  = flush_bufs.size() + 1;
    msg.commands[0].code = NDRKERNEL;
    msg.commands[0].addr = device_addr;
    msg.commands[0].size = device_size;

    /*-------------------------------------------------------------------------
    * Make sure we do not overflow the number of commands a mailbox can handle
    *------------------------------------------------------------------------*/
    if (flush_bufs.size() > 19) ERR("To many buffers to flush");

    /*-------------------------------------------------------------------------
    * Populate Flush commands for any buffers that are read by the DSP
    *------------------------------------------------------------------------*/
    for (int i=0; i < flush_bufs.size(); ++i)
    {
        msg.commands[i+1].code = CACHEINV;
        msg.commands[i+1].addr = flush_bufs[i].first; 
        msg.commands[i+1].size = flush_bufs[i].second; 
    }

    /*-------------------------------------------------------------------------
    * Dispatch the commands through the mailbox
    *------------------------------------------------------------------------*/
    p_device->mail_to(msg);
    while (!p_device->mail_query()) ;
    int retval = p_device->mail_from();

    /*-------------------------------------------------------------------------
    * Free allocated local buffers
    *------------------------------------------------------------------------*/
    for (size_t i = 0; i < local_bufs.size(); ++i)
        p_device->free_l2(local_bufs[i]);

    p_device->free_ddr(device_addr);

    delete[] host_args_ptr;

    return (retval == SUCCESS || retval == READY) ? true : false;
}
