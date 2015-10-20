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
#include "driver.h"

#include "../kernel.h"
#include "../memobject.h"
#include "../events.h"
#include "../program.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/param.h>

#include "llvm/IR/InstIterator.h"

#if defined(DEVICE_K2X)
extern "C"
{
    #include <ti/runtime/mmap/include/mmap_resource.h>
}
#endif


#define ROUNDUP(val, pow2)   (((val) + (pow2) - 1) & ~((pow2) - 1))
#define QERR(msg, retcode)   do { std::cerr << "OCL ERROR: " << msg << std::endl; return retcode; } while(0)
#define ERR(x) std::cerr << x << std::endl
#define ERROR() std::cerr << "Unknown error in dsp/kernel.cpp" << std::endl

using namespace Coal;


DSPKernel::DSPKernel(DSPDevice *device, Kernel *kernel, llvm::Function *function)
: DeviceKernel(), p_device(device), p_kernel(kernel), 
    p_device_entry_pt((DSPDevicePtr)0),
    p_data_page_ptr  ((DSPDevicePtr)0xffffffff),
    p_function(function)
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
cl_int DSPKernel::preAllocBuffers()
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


/******************************************************************************
* Try to find the size a work group needs to be executed the fastest on the DSP.
******************************************************************************/
size_t DSPKernel::guessWorkGroupSize(cl_uint num_dims, cl_uint dim,
                                     size_t global_work_size) const
{
    // ASW TODO - what the ????
    unsigned int dsps = p_device->dspCores();

    /*-------------------------------------------------------------------------
    * Find the divisor of global_work_size the closest to dsps but >= than it
    *------------------------------------------------------------------------*/
    unsigned int divisor = dsps <= 0 ? 1 : dsps;

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

        divisor -= 1;
    }

    /*-------------------------------------------------------------------------
    * Return the size
    *------------------------------------------------------------------------*/
    return global_work_size / divisor;
}

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


/******************************************************************************
* workGroupSize()
******************************************************************************/
size_t DSPKernel::workGroupSize()  const 
{ 
    size_t wgsize = 0;

    int wi_alloca_size = p_kernel->get_wi_alloca_size();

    /*-------------------------------------------------------------------------
    * Get the device max limit size, which can change if the environment
    * variable TI_OCL_WG_SIZE_LIMIT is used 
    *-------------------------------------------------------------------------*/
    p_device->info(CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(wgsize),&wgsize,NULL);

    /*-------------------------------------------------------------------------
    * if no wi_alloca, use the work group size limit specified by the device
    *-------------------------------------------------------------------------*/
    if (wi_alloca_size == 0) return wgsize;

    /*-------------------------------------------------------------------------
    * else use the smaller of the device limit or up to 16M of wi_alloca
    * space per kernel.  Recall that an area is reserved for each core,
    * so we divide 2M rather than 16M.
    *-------------------------------------------------------------------------*/
    else return MIN(wgsize, (2<<20) / next_power_of_two(wi_alloca_size));
}
 
/******************************************************************************
* localMemSize() 
*   This does not comprehend barrier variable expansion that may be allocated
*   int L2.  We will not know the size until we know the local size in the 
*   DSPKernelEvent.  If a reqd_work_group_size pragma is used, we could know 
*   here.  TODO
******************************************************************************/
cl_ulong DSPKernel::localMemSize() const
{
    Program    *p     = (Program *)p_kernel->parent();
    DSPProgram *prog  = (DSPProgram *)(p->deviceDependentProgram(p_device));

    uint32_t  size;
    DSPDevicePtr end_allocated_l2 = locals_as_args_extent(size);

    end_allocated_l2  += size;

    DSPDevicePtr L2addr = p_device->get_L2_extent(size);

    return end_allocated_l2 - L2addr;
}

Kernel *         DSPKernel::kernel()   const { return p_kernel; }
DSPDevice *      DSPKernel::device()   const { return p_device; }

/******************************************************************************
* locals_in_kernel_extent
*
* Assuming an L2 layout beyond the monitor reserved area like the below:
*    return the kernel .ocl_local_overlay address and a kernel specific 
*    .ocl_local_overlay size.
*
* +=========+====================+=================+====================+=========+
* | .mem_l2 | .ocl_local_overlay | local arguments | variable expansion | scratch |
* +=========+====================+=================+====================+=========+
* ^                                                                     ^
* |                                                                     |
* L2 start                                                        scratch start
*
******************************************************************************/
DSPDevicePtr DSPKernel::locals_in_kernel_extent(uint32_t &ret_size) const 
{
    Program    *p     = (Program *)p_kernel->parent();
    DSPProgram *prog  = (DSPProgram *)(p->deviceDependentProgram(p_device));

    uint32_t size;
    DSPDevicePtr addr = prog->mem_l2_section_extent(size);

    addr += size;

    /*-------------------------------------------------------------------------
    * Get kernel attr indicating size of kernel static local buffers
    *------------------------------------------------------------------------*/
    llvm::Function* function = p_kernel->function(p_device);
    std::string fattrs = function->getAttributes().getAsString(
                                                   llvm::AttributeSet::FunctionIndex);

    uint32_t    locals_in_kernel_size = 0;
    const char* attr_string           = "_kernel_local_size=";
    std::size_t found                 = fattrs.find(attr_string);

    if (found != std::string::npos)
        locals_in_kernel_size = atoi(fattrs.data() + found + strlen(attr_string));

    ret_size = ROUNDUP(locals_in_kernel_size, MIN_BLOCK_SIZE);
    return addr;
}

DSPDevicePtr DSPKernel::locals_as_args_extent(uint32_t &ret_size) const
{
    uint32_t size;
    DSPDevicePtr addr = locals_in_kernel_extent(size);
    addr += size;

    uint32_t local_args_size = 0;

    for (int i = 0;  i < kernel()->numArgs(); ++i)
    {
        const Kernel::Arg &arg = kernel()->arg(i);

        if (arg.kind() == Kernel::Arg::Buffer && 
            arg.file() == Kernel::Arg::Local)
              local_args_size += ROUNDUP(arg.allocAtKernelRuntime(), MIN_BLOCK_SIZE);
    }

    ret_size = local_args_size;
    return addr;
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

static int kernelID = 0;

/*=============================================================================
* DSPKernelEvent
*============================================================================*/
DSPKernelEvent::DSPKernelEvent(DSPDevice *device, KernelEvent *event)
: p_ret_code(CL_SUCCESS),
  p_device(device), p_event(event), p_kernel((DSPKernel*)event->deviceKernel()),
  p_debug_kernel(NODEBUG), p_num_arg_words(0),
  p_WG_alloca_start(0), 
  argref_offset(0)
{ 
    p_kernel_id = __sync_fetch_and_add(&kernelID, 1);

    char *dbg = getenv("TI_OCL_DEBUG");
    if (dbg) p_debug_kernel = (strcmp(dbg, "ccs") == 0) ? CCS : GDBC6X;

    p_ret_code = callArgs(MAX_ARGS_TOTAL_SIZE);

    /*-------------------------------------------------------------------------
    * Populate some of the kernel_msg_t structure. 
    *------------------------------------------------------------------------*/
    p_msg.u.k.kernel.Kernel_id     = p_kernel_id;
    p_msg.u.k.kernel.entry_point   = (unsigned)p_kernel->device_entry_pt();
    p_msg.u.k.kernel.data_page_ptr = (unsigned)p_kernel->data_page_ptr();
}

DSPKernelEvent::~DSPKernelEvent() { }

#define READ_ONLY_BUFFER(buffer)  (buffer->flags() & CL_MEM_READ_ONLY)
#define WRITE_ONLY_BUFFER(buffer) (buffer->flags() & CL_MEM_WRITE_ONLY)
#define HOST_NO_ACCESS(buffer)    (buffer->flags() & CL_MEM_HOST_NO_ACCESS)

#define SETMOREARG(sz, pval) do \
    { \
        more_arg_offset = ROUNDUP(more_arg_offset, sz); \
        memcpy(more_args_in_mem+more_arg_offset, pval, sz); \
        more_arg_offset += sz; \
    } while(0)

#define TOTAL_REGS	(MAX_IN_REG_ARGUMENTS*2)
#define REG_START	4

// returns index in args_in_reg array
int getarg_inreg_index(int sz, int &first_apair, int &first_bpair,
                               int &first_aquad, int &first_bquad)
{
    int index;   // index of [A4,A5,B4,B5,A6,A7,B6,B7...], start from 0
    if (sz <= 8)        // fits in a pair, e.g. A5:A4
    { 
        if (first_apair <= first_bpair) 
        {
            index = (first_apair - REG_START)*2;
            if (index+1 >= TOTAL_REGS) return -1;
            first_apair += 2;
            if (first_apair % 4 != 0) first_aquad += 4;
            else if (first_apair < first_aquad) first_apair = first_aquad;
        }
        else
        {
            index = (first_bpair - REG_START)*2 + 2;
            if (index+1 >= TOTAL_REGS) return -1;
            first_bpair += 2;
            if (first_bpair % 4 != 0) first_bquad += 4;
            else if (first_bpair < first_bquad) first_bpair = first_bquad;
        }
    }
    else if (sz <= 16)  // fits in a quad, e.g. A7:A6:A5:A4
    {
        if (first_aquad <= first_bquad)
        {
            index = (first_aquad - REG_START)*2;
            if (index+5 >= TOTAL_REGS) return -1;
            if (first_apair == first_aquad) first_apair += 4;
            first_aquad += 4;
        }
        else
        {
            index = (first_bquad - REG_START)*2 + 2;
            if (index+5 >= TOTAL_REGS) return -1;
            if (first_bpair == first_bquad) first_bpair += 4;
            first_bquad += 4;
        }
    }
    else  return -1;

    return index;
}

void setarg_inreg(int index, int sz, unsigned int *args_in_reg, void *pval)
{
    if (sz <= 8)        // fits in a pair, e.g. A5:A4
    { 
        memcpy(&args_in_reg[index], pval, sz);  // Little Endian!!!
    }
    else if (sz == 16)  // fits in a quad, e.g. A7:A6:A5:A4
    {
        memcpy(&args_in_reg[index],   pval,   8);           // Little Endian!!!
        memcpy(&args_in_reg[index+4], ((char*) pval)+8, 8); // next 8 bytes
    }
}


/******************************************************************************
* DSPKernelEvent::callArgs
******************************************************************************/
cl_int DSPKernelEvent::callArgs(unsigned max_args_size)
{
    int args_total_size = 0;

    unsigned  *args_in_reg = (unsigned*)p_msg.u.k.kernel.args_in_reg;
    memset(args_in_reg, 0, sizeof(p_msg.u.k.kernel.args_in_reg));
    int AP = REG_START;  // initialize for argument register allocation
    int BP = REG_START;
    int AQ = REG_START;
    int BQ = REG_START;

    char      *more_args_in_mem = (char *)args_on_stack;
    int        more_arg_offset  = 4;
    p_msg.u.k.kernel.args_on_stack_addr = 0;

    /*-------------------------------------------------------------------------
    * Write Arguments
    *------------------------------------------------------------------------*/
    for (int i = 0;  i < p_kernel->kernel()->numArgs(); ++i)
    {
        int args_in_reg_index = -1;

        const Kernel::Arg & arg  = p_kernel->kernel()->arg(i);
        size_t              size = arg.valueSize() * arg.vecDim();


        if (size == 0)
            QERR("Kernel argument has size of 0", CL_INVALID_ARG_SIZE);

        args_total_size += size;
        if (args_total_size > max_args_size)
            QERR("Total size of arguments exceeds allowed maximum (1024 bytes)",
                 CL_INVALID_KERNEL_ARGS);

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

                args_in_reg_index = getarg_inreg_index(4, AP, BP, AQ, BQ);
                DSPVirtPtr *buf_dspvirtptr = (args_in_reg_index >= 0) ?
                                             (&args_in_reg[args_in_reg_index]) :
                   (DSPVirtPtr *)(more_args_in_mem+ROUNDUP(more_arg_offset,4));

                /*-------------------------------------------------------------
                * Alloc a buffer and pass it to the kernel
                *------------------------------------------------------------*/
                if (arg.file() == Kernel::Arg::Local)
                {
                    uint32_t     lbufsz = arg.allocAtKernelRuntime();
                    p_local_bufs.push_back(LocalPair(buf_dspvirtptr, lbufsz));

                    /*-----------------------------------------------------
                    * Since the only reader and writer of local memory (L2)
                    * will be the core itself, I do not believe we need 
                    * to flush local buffers for correctness. 
                    *----------------------------------------------------*/
                    //p_flush_bufs->push_back(DSPMemRange(lbuf, lbufsz));
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
                        DSPBuffer *dspbuf = (DSPBuffer *)buffer->deviceBuffer(p_device);
                        buffer->allocate(p_device);
                        DSPDevicePtr64 addr64 = dspbuf->data();
                        if (addr64 < 0xFFFFFFFF)
                            buf_ptr = addr64;
                        else
                            p_64bit_bufs.push_back(DSPMemRange(DSPPtrPair(
                                     addr64, buf_dspvirtptr), buffer->size()));

                        if (buffer->get_host_ptr_clMalloced())
                            p_hostptr_clMalloced_bufs.push_back(buffer);
      
                        if (! WRITE_ONLY_BUFFER(buffer))
                            p_flush_bufs.push_back(DSPMemRange(DSPPtrPair(
                                     addr64, buf_dspvirtptr), buffer->size()));
                    }
                }

                /*---------------------------------------------------------
                * Use 0 for local buffer address here, it will be overwritten
                * with allocated local buffer address at kernel dispatch time.
                * Same for allocating temporary buffer for use_host_ptr.
                *--------------------------------------------------------*/
                if (args_in_reg_index >= 0)
                    setarg_inreg(args_in_reg_index, 4, args_in_reg, &buf_ptr);
                else
                    SETMOREARG(4, &buf_ptr);

                break;
            }

            case Kernel::Arg::Image2D:
            case Kernel::Arg::Image3D: 
                QERR("Images not yet supported", CL_INVALID_KERNEL_ARGS);
                break;

            /*-----------------------------------------------------------------
            * Non-Buffers 
            *----------------------------------------------------------------*/
            default:
                args_in_reg_index = getarg_inreg_index(size, AP, BP, AQ, BQ);
                if (args_in_reg_index >= 0)  // args_in_reg
                {
                    int dummy = 0;

                    if (arg.is_subword_int_uns())
                    {
                        if (size == 1) dummy = (unsigned) *((unsigned char*)arg.data());
                        else if (size == 2)  dummy = (unsigned) *((unsigned short*)arg.data());
                    }
                    else
                    {
                        if (size == 1) dummy = (int) *((signed char*)arg.data());
                        else if (size == 2)  dummy = (int) *((short*)arg.data());
                    }

                    void *p_data = (dummy == 0) ? (void*)arg.data() : (void *) &dummy;
                    size         = (dummy == 0) ? size : 4;

                    setarg_inreg(args_in_reg_index, size, args_in_reg, p_data);
                }
                else if (size <= 16)         // args_on_stack
                {
                    SETMOREARG(size, arg.data());
                }
                else                         // args_of_argref
                {
                    // 1. get address of argref in_reg or on_stack
                    args_in_reg_index = getarg_inreg_index(4, AP, BP, AQ, BQ);
                    DSPVirtPtr *argref_dspvirtptr = (args_in_reg_index >= 0) ?
                                            (&args_in_reg[args_in_reg_index]) :
                   (DSPVirtPtr *)(more_args_in_mem+ROUNDUP(more_arg_offset,4));

                    // 2. put argref placeholder (dummy 0) in_reg or on_stack
                    DSPVirtPtr dummy_addr = 0;
                    if (args_in_reg_index >= 0)  // args_in_reg
                        setarg_inreg(args_in_reg_index, 4, args_in_reg,
                                     &dummy_addr);
                    else
                        SETMOREARG(4, &dummy_addr);

                    // 3. bookkeep offset, to be written back of actual addr
                    p_argrefs.push_back(LocalPair(argref_dspvirtptr,
                                                  argref_offset));

                    // 4. copy data into args_of_argref[], increment offset
                    //    give everybody 8 byte alignment
                    memcpy(&args_of_argref[argref_offset], arg.data(), size);
                    argref_offset = ROUNDUP(argref_offset + size, 8);
                }
                    
                break;
        }
    }

    int num_regs = 0;
    int last_reg = AP - 2;                     // last A or B register used
    if (last_reg < AQ - 2) last_reg = AQ - 2;
    if (last_reg < BP - 2) last_reg = BP - 2;
    if (last_reg < BQ - 2) last_reg = BQ - 2;
    if (last_reg >= REG_START) num_regs = (last_reg - REG_START) * 2 + 4;
    if (num_regs > TOTAL_REGS) num_regs = TOTAL_REGS;
    p_msg.u.k.kernel.args_in_reg_size = num_regs;

    p_msg.u.k.kernel.args_on_stack_size = (more_arg_offset > 4) ?
                                         ROUNDUP(more_arg_offset, 8) : 0;

    return CL_SUCCESS;
}

/******************************************************************************
* debug_pause
******************************************************************************/
static void debug_pause(uint32_t entry, uint32_t dsp_id, 
                        const char* outfile, char *name, DSPDevicePtr load_addr,
                        bool is_gdbc6x)
{
    Driver *driver = Driver::instance();

    if (is_gdbc6x)
        printf("gdbc6x -q "
               "-iex \"target remote /dev/gdbtty%d\" "
               "-iex \"set confirm off\" "
               "-iex \"symbol-file %s\" "
               "-iex \"add-symbol-file %s 0x%08x\" "
               "-iex \"b exit\" "
               "-iex \"b %s\" "
               "\n",
                dsp_id, driver->dsp_monitor(dsp_id).c_str(),
                outfile, load_addr, name);
    else
        printf("CCS Suspend dsp core 0\n"
               "CCS Load symbols: %s, code offset: 0x%x\n"
               "CCS Add symbols: %s, no code offset\n"
               "CCS Add breakpoint: %s\n"
               "CCS Resume dsp core 0\n",
               outfile, load_addr, driver->dsp_monitor(dsp_id).c_str(), name);

    printf("Press any key, then enter to continue\n");
    do { char t; std::cin >> t; } while(0);
}



/******************************************************************************
* bool DSPKernelEvent::run()
******************************************************************************/
cl_int DSPKernelEvent::run(Event::Type evtype)
{
    // TODO perhaps ensure that prog is loaded.
    Program    *p    = (Program *)p_kernel->kernel()->parent();
    DSPProgram *prog = (DSPProgram *)(p->deviceDependentProgram(p_device));

    /*-------------------------------------------------------------------------
    * Allocate local buffers in L2 for a  kernel run instance
    *------------------------------------------------------------------------*/
    uint32_t     remaining_l2_size;
    DSPDevicePtr local_scratch;

    cl_int err = allocate_and_assign_local_buffers(remaining_l2_size, local_scratch);
    if (err != CL_SUCCESS) return err;

    /*-------------------------------------------------------------------------
    * Populate the kernel_config_t structure
    *------------------------------------------------------------------------*/
    err = init_kernel_runtime_variables(evtype, remaining_l2_size, local_scratch);
    if (err != CL_SUCCESS) return err;

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

    /*-------------------------------------------------------------------------
    * Compute MPAX mappings from DSPDevicePtr64 to DSPVirtPtr in p_64bit_bufs
    *------------------------------------------------------------------------*/
    err = setup_extended_memory_mappings();
    if (err != CL_SUCCESS) return err;

    p_msg.u.k.flush.need_cache_op = p_flush_bufs.size();

    /*-------------------------------------------------------------------------
    * Handle stack based kernel arguments (i.e. args > 10)
    *------------------------------------------------------------------------*/
    err = setup_stack_based_arguments();
    if (err != CL_SUCCESS) return err;

    /*-------------------------------------------------------------------------
    * Feedback to user for debug
    *------------------------------------------------------------------------*/
    int ret = debug_kernel_dispatch();
    if (ret != CL_SUCCESS) return ret;

    /*---------------------------------------------------------------------
    * For host scheduled devices, i.e. AM57, NDRkernels send to all cores.
    * The monitor will determine how to divide the work. Need to wait on 
    * all replies.
    * Order!! After switching to two worker threads, we must push the complete
    * pending first, then send out the mails, to prevent the extrememly fast
    * DSP reply that has no corresponding complete pending from happening.
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
* Allocating local buffer in L2 per kernel run instance
******************************************************************************/
cl_int DSPKernelEvent::allocate_and_assign_local_buffers(
           uint32_t &remaining_l2_size, DSPDevicePtr &local_scratch)
{
    uint32_t     l2size;
    DSPDevicePtr l2start = p_device->get_L2_extent(l2size);

    uint32_t     size;
    DSPDevicePtr addr    = p_kernel->locals_in_kernel_extent(size);

    local_scratch        = addr + size;
    remaining_l2_size    = l2size - (local_scratch - l2start);

    for (size_t i = 0; i < p_local_bufs.size(); ++i)
    {
        DSPVirtPtr *p_arg_word     = p_local_bufs[i].first; 
        unsigned    local_buf_size = p_local_bufs[i].second;

        uint32_t rounded_sz = ROUNDUP(local_buf_size, MIN_BLOCK_SIZE);
        if (rounded_sz > remaining_l2_size)
        {
            QERR("Total size of local buffers exceeds available local size",
                 CL_MEM_OBJECT_ALLOCATION_FAILURE);
        }
        
        *p_arg_word        = local_scratch;
        local_scratch     += rounded_sz;
        remaining_l2_size -= rounded_sz;
    }
    return CL_SUCCESS;
}

/******************************************************************************
* Initialize the kernel configuration parameters
******************************************************************************/
cl_int DSPKernelEvent::init_kernel_runtime_variables(Event::Type evtype,
                                               uint32_t     remaining_l2_size,
                                               DSPDevicePtr local_scratch)
{
    int dim  = p_event->work_dim();

    /*-------------------------------------------------------------------------
    * Create a message for the DSP
    *------------------------------------------------------------------------*/
    kernel_config_t *cfg  = &p_msg.u.k.config;

    bool effective_task = (p_event->global_work_size(0) == 1 &&
                           p_event->global_work_size(1) == 1 &&
                           p_event->global_work_size(2) == 1);

    /*-------------------------------------------------------------------------
    * Overloaded use of this field.  The WG_gid_start fields are not 
    * communicated from host to device, which allows this overload.
    *------------------------------------------------------------------------*/
    cfg->WG_gid_start[0] = p_debug_kernel != NODEBUG ? DEBUG_MODE_WG_GID_START
                                                     : NORMAL_MODE_WG_GID_START;

    if (evtype == Event::TaskKernel || effective_task)
    {
        p_msg.command  = TASK;

        CommandQueue *q = (CommandQueue *) p_event->parent();
        cl_command_queue_properties q_prop = 0;
        q->info(CL_QUEUE_PROPERTIES, sizeof(q_prop), &q_prop, NULL);

        /*---------------------------------------------------------------------
        * Overloaded use of this field for tasks.
        *--------------------------------------------------------------------*/
        cfg->global_size[0] = (q_prop & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) ?
                           OUT_OF_ORDER_TASK_SIZE : IN_ORDER_TASK_SIZE;

        /*---------------------------------------------------------------------
        * Note: We do not bother to write the fields that are static for tasks.
        * i.e. the fields in the else clause below.
        *--------------------------------------------------------------------*/
    }
    else 
    {
        p_msg.command = NDRKERNEL;

        cfg->num_dims         = dim;
        cfg->global_size[0]   = p_event->global_work_size(0);
        cfg->global_size[1]   = p_event->global_work_size(1);
        cfg->global_size[2]   = p_event->global_work_size(2);
        cfg->local_size[0]    = p_event->local_work_size(0);
        cfg->local_size[1]    = p_event->local_work_size(1);
        cfg->local_size[2]    = p_event->local_work_size(2);
        cfg->global_offset[0] = p_event->global_work_offset(0);
        cfg->global_offset[1] = p_event->global_work_offset(1);
        cfg->global_offset[2] = p_event->global_work_offset(2);
    }

    /*-------------------------------------------------------------------------
    * Allocate temporary space for kernel variable expanded private data.
    * Will attempt allocation from faster to slower memory.
    *------------------------------------------------------------------------*/
    cfg->WG_alloca_size = p_kernel->kernel()->get_wi_alloca_size() * 
        p_event->local_work_size(0)* p_event->local_work_size(1)* p_event->local_work_size(2);

    if (cfg->WG_alloca_size > 0)
    {
        uint32_t chip_alloca_size = cfg->WG_alloca_size * p_device->dspCores();
        if (cfg->WG_alloca_size <= remaining_l2_size)
        {
            p_WG_alloca_start = local_scratch;
            /*----------------------------------------------------------------
             * In generated kernel init code:
             * _wg_alloca_start = WG_alloca_start
             *                  + __core_num() * WG_alloca_size
             * set 0 so that each core get same WG_alloca_start in L2
             *---------------------------------------------------------------*/
            cfg->WG_alloca_size = 0;

            uint32_t rounded_sz = ROUNDUP(cfg->WG_alloca_size, MIN_BLOCK_SIZE);
            local_scratch      += rounded_sz;
            remaining_l2_size  -= rounded_sz;
        }
        else 
        {
            p_WG_alloca_start = p_device->malloc_msmc(chip_alloca_size);
            if (!p_WG_alloca_start)
                p_WG_alloca_start = p_device->malloc_global(chip_alloca_size, true);
        }

        if (!p_WG_alloca_start)
        {
            QERR("Workgroup alloca size exceeds available global memory",
                 CL_OUT_OF_RESOURCES);
        }

        if (p_WG_alloca_start < 0xFFFFFFFF)
            cfg->WG_alloca_start = (DSPVirtPtr) p_WG_alloca_start;
        else
            p_64bit_bufs.push_back(DSPMemRange(DSPPtrPair(
                 p_WG_alloca_start, &cfg->WG_alloca_start), chip_alloca_size));
    }

    cfg->L2_scratch_start = local_scratch;
    cfg->L2_scratch_size  = remaining_l2_size;

    return CL_SUCCESS;
}

/******************************************************************************
* Allocate temporary global buffer for non-clMalloced USE_HOST_PTR
******************************************************************************/
cl_int DSPKernelEvent::allocate_temp_global(void)
{
    Driver *driver = Driver::instance();
    for (int i = 0; i < p_hostptr_tmpbufs.size(); ++i)
    {
        MemObject      *buffer       =  p_hostptr_tmpbufs[i].first;
        DSPDevicePtr64 *p_addr64     = &p_hostptr_tmpbufs[i].second.first;
        DSPVirtPtr     *p_arg_word   =  p_hostptr_tmpbufs[i].second.second;
        
        *p_addr64 = p_device->malloc_global(buffer->size(), false);

        if (!(*p_addr64))
        {
            QERR("Temporary memory for CL_MEM_USE_HOST_PTR buffer exceeds available global memory",
                 CL_MEM_OBJECT_ALLOCATION_FAILURE);
        }

        if (*p_addr64 < 0xFFFFFFFF)
            *p_arg_word = *p_addr64;
        else
            p_64bit_bufs.push_back(DSPMemRange(DSPPtrPair(
                                      *p_addr64, p_arg_word), buffer->size()));

        if (! WRITE_ONLY_BUFFER(buffer))
        {
            void *mapped_tmpbuf = driver->map(p_device, *p_addr64,
                                              buffer->size(), false);
            memcpy(mapped_tmpbuf, buffer->host_ptr(), buffer->size());
            p_flush_bufs.push_back(DSPMemRange(DSPPtrPair(
                                      *p_addr64, p_arg_word), buffer->size()));
            driver->unmap(p_device, mapped_tmpbuf, *p_addr64, buffer->size(),
                          true);
        }
    }

    return CL_SUCCESS;
}

/*-------------------------------------------------------------------------
* Ensure that __malloc_xxx USE_HOST_PTR buffers are flushed from cache
*------------------------------------------------------------------------*/
cl_int DSPKernelEvent::flush_special_use_host_ptr_buffers(void)
{
    Driver *driver = Driver::instance();
    int total_buf_size = 0;
    for (int i = 0; i < p_hostptr_clMalloced_bufs.size(); ++i)
    {
        MemObject *buffer = p_hostptr_clMalloced_bufs[i];
        if (HOST_NO_ACCESS(buffer)) continue; // Exclude buffers not accessed
        total_buf_size += buffer->size();
    }

    /*-------------------------------------------------------------------------
    * Threshold is 32MB.
    *------------------------------------------------------------------------*/
    int  threshold  = (32<<20);
    bool wb_inv_all = false;
    if (total_buf_size >= threshold)  wb_inv_all = driver->cacheWbInvAll();

    if (! wb_inv_all)
    {
        for (int i = 0; i < p_hostptr_clMalloced_bufs.size(); ++i)
        {
            MemObject *buffer = p_hostptr_clMalloced_bufs[i];
            DSPBuffer *dspbuf = (DSPBuffer *) buffer->deviceBuffer(p_device);
            DSPDevicePtr64 data = (DSPDevicePtr64)dspbuf->data();

            if (! READ_ONLY_BUFFER(buffer) && ! HOST_NO_ACCESS(buffer))
                driver->cacheWbInv(data, buffer->host_ptr(), buffer->size());
            else if (! WRITE_ONLY_BUFFER(buffer) && ! HOST_NO_ACCESS(buffer))
                driver->cacheWb(data, buffer->host_ptr(), buffer->size());
        }
    }

    return CL_SUCCESS;
}

/******************************************************************************
* Compute MPAX mappings from DSPDevicePtr64 to DSPVirtPtr in p_64bit_bufs
******************************************************************************/
cl_int DSPKernelEvent::setup_extended_memory_mappings()
{
    p_msg.u.k.flush.num_mpaxs = 0;
    uint32_t num_64bit_bufs = p_64bit_bufs.size();
#if defined(DEVICE_K2X)
    if (num_64bit_bufs > 0)
    {
        uint64_t *phys_addrs = new uint64_t[num_64bit_bufs];
        uint32_t *lengths    = new uint32_t[num_64bit_bufs];
        uint32_t *prots      = new uint32_t[num_64bit_bufs];
        uint32_t *virt_addrs = new uint32_t[num_64bit_bufs];
        for (int i = 0; i < p_64bit_bufs.size(); ++i)
        {
            phys_addrs[i] = p_64bit_bufs[i].first.first;
            lengths[i]    = p_64bit_bufs[i].second;
            prots[i]      = 0;  // don't care yet
        }

        keystone_mmap_resources_t mpax_res;
        memcpy(&mpax_res, p_device->get_mpax_default_res(),
               sizeof(keystone_mmap_resources_t));
        if (keystone_mmap_resource_alloc(num_64bit_bufs, phys_addrs, lengths, 
                  prots, virt_addrs, &mpax_res) != KEYSTONE_MMAP_RESOURCE_NOERR)
        {
            QERR("MPAX allocation failed",
                 CL_OUT_OF_RESOURCES);
        }
    
        // set the MPAX settings in the message
        uint32_t mpax_used = 0;
        for (; mpax_res.mapping[mpax_used].segsize_power2 > 0; mpax_used += 1)
        {
            p_msg.u.k.flush.mpax_settings[2*mpax_used+1] =     // e.g. 0xC000000D
                  mpax_res.mapping[mpax_used].baddr
               | (mpax_res.mapping[mpax_used].segsize_power2-1);
            p_msg.u.k.flush.mpax_settings[2*mpax_used  ] =     // e.g. 0x8220043F
                 ((uint32_t) (mpax_res.mapping[mpax_used].raddr >> 4))
               | DEFAULT_PERMISSION;
        }
        p_msg.u.k.flush.num_mpaxs = mpax_used;
    
        // set the virtual address in arguments
        for (int i = 0; i < p_64bit_bufs.size(); ++i)
        {
            *(p_64bit_bufs[i].first.second) = virt_addrs[i];
            if (p_debug_kernel != NODEBUG)
               printf("Virtual = 0x%x, physical = 0x%llx\n",
                      virt_addrs[i], p_64bit_bufs[i].first.first);
        }
        delete [] phys_addrs;
        delete [] lengths;
        delete [] prots;
        delete [] virt_addrs;
    }
    else
    {
        // protect linux memory:  virt 0x8000_0000 to 0xA000_0000, size 512MB
        // no read/write/execute: phys 0x8:0000_0000 to 0x8:2000_0000
        p_msg.u.k.flush.num_mpaxs = 1;
        p_msg.u.k.flush.mpax_settings[1] = 0x8000001C;
        p_msg.u.k.flush.mpax_settings[0] = 0x80000000;
    }
#endif  // #ifndef DSPC868x

    return CL_SUCCESS;
}

/******************************************************************************
* Allocate device memory for args_on_stack and args_of_argref
* Write-back argref addresses
* Copy args_on_stack and args_of_argref onto device
******************************************************************************/
cl_int DSPKernelEvent::setup_stack_based_arguments()
{
    Driver *driver = Driver::instance();
    uint32_t rounded_args_on_stack_size = 
                ROUNDUP(p_msg.u.k.kernel.args_on_stack_size, 128);

    uint32_t args_in_mem_size = rounded_args_on_stack_size + argref_offset;

    if (args_in_mem_size > 0)
    {
        // 1. allocate memory
        DSPDevicePtr64 args_addr = p_device->malloc_msmc(args_in_mem_size);

        if (!args_addr)
            args_addr = p_device->malloc_global(args_in_mem_size, true);

        if (!args_addr)
            QERR("Unable to allocate memory for kernel arguments",
                 CL_OUT_OF_RESOURCES);

        // 2. write back argref addresses
        for (int i = 0; i < p_argrefs.size(); i++)
        {
            DSPVirtPtr *p_virtaddr = p_argrefs[i].first;
            uint32_t    offset     = p_argrefs[i].second;
            *p_virtaddr = (DSPVirtPtr) (args_addr + rounded_args_on_stack_size
                                                  + offset);
        }

        // 3. copy args_on_stack and args_of_argref
        void *mapped_addr = driver->map(p_device, args_addr, args_in_mem_size,
                                        false);
        if (rounded_args_on_stack_size > 0)
            memcpy(mapped_addr, args_on_stack, rounded_args_on_stack_size);

        if (argref_offset > 0)
            memcpy(((char*)mapped_addr)+rounded_args_on_stack_size, args_of_argref,
                   argref_offset);

        driver->unmap(p_device, mapped_addr, args_addr, args_in_mem_size, true);

        // 4. set p_msg.u.k.kernel.args_on_stack_addr
        p_msg.u.k.kernel.args_on_stack_addr = (DSPVirtPtr) args_addr;
    }
    return CL_SUCCESS;
}



/******************************************************************************
* Feedback to user for debug
******************************************************************************/
int DSPKernelEvent::debug_kernel_dispatch()
{
    if (p_debug_kernel != NODEBUG)
    {
        size_t name_length;
        p_kernel->kernel()->info(CL_KERNEL_FUNCTION_NAME, 0, 0, &name_length);
        char *name = (char*)malloc(name_length);
        if (!name) return CL_OUT_OF_HOST_MEMORY;
        p_kernel->kernel()->info(CL_KERNEL_FUNCTION_NAME, name_length, name, 0);

        Program    *p     = (Program *)p_kernel->kernel()->parent();
        DSPProgram *prog  = (DSPProgram *)(p->deviceDependentProgram(p_device));

        debug_pause(p_kernel->device_entry_pt(), p_device->dspID(), 
                    prog->outfile_name(), name, prog->program_load_addr(),
                    (p_debug_kernel == GDBC6X));
        free (name);
    }
    return CL_SUCCESS;
}

/******************************************************************************
* free_tmp_bufs allocated for kernel allocas, and for use_host_ptr
******************************************************************************/
void DSPKernelEvent::free_tmp_bufs()
{
    Driver *driver = Driver::instance();
    if (p_WG_alloca_start > 0)
    {
        if (   p_WG_alloca_start >= MSMC_OCL_START_ADDR
            && p_WG_alloca_start < MSMC_OCL_END_ADDR)
            p_device->free_msmc(p_WG_alloca_start);
        else
            p_device->free_global(p_WG_alloca_start);
    }

    if (p_msg.u.k.kernel.args_on_stack_addr > 0)
    {
        if (   p_msg.u.k.kernel.args_on_stack_addr >= MSMC_OCL_START_ADDR
            && p_msg.u.k.kernel.args_on_stack_addr < MSMC_OCL_END_ADDR)
            p_device->free_msmc(p_msg.u.k.kernel.args_on_stack_addr);
        else
            p_device->free_global(p_msg.u.k.kernel.args_on_stack_addr);
    }

    for (int i = 0; i < p_hostptr_tmpbufs.size(); ++i)
    {
        MemObject *buffer     = p_hostptr_tmpbufs[i].first; 
        DSPDevicePtr64 addr64 = p_hostptr_tmpbufs[i].second.first; 

        if (! READ_ONLY_BUFFER(buffer))
        {
            void *mapped_tmpbuf = driver->map(p_device, addr64, buffer->size(),
                                              true);
            memcpy(buffer->host_ptr(), mapped_tmpbuf, buffer->size());
            driver->unmap(p_device, mapped_tmpbuf, addr64, buffer->size(),
                          false);
        }
        p_device->free_global(addr64);
    }

    /*-------------------------------------------------------------------------
    * Cache-Inv mapped buffers for clMalloced USE_HOST_PTR
    * CHANGE: We do cacheWbInv before dispatching kernel for !READ_ONLY
    * CHANGE AGAIN: On AM57, we see (7 out of 8000) failures that 1 or 2
    *               previously invalidated cache line got back into cache,
    *               so we inv again here
    *------------------------------------------------------------------------*/
    // /***
    for (int i = 0; i < p_hostptr_clMalloced_bufs.size(); ++i)
    {
        MemObject *buffer = p_hostptr_clMalloced_bufs[i];
        DSPBuffer *dspbuf = (DSPBuffer *) buffer->deviceBuffer(p_device);
        DSPDevicePtr64 data = (DSPDevicePtr64)dspbuf->data();
        if (! READ_ONLY_BUFFER(buffer))
            driver->cacheInv(data, buffer->host_ptr(), buffer->size());
    }
    // ***/
}

