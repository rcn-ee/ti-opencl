/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file cpu/kernel.cpp
 * \brief CPU kernel
 */

#include "kernel.h"
#include "device.h"
#include "buffer.h"
#include "program.h"
#include "builtins.h"

#include "../kernel.h"
#include "../memobject.h"
#include "../events.h"
#include "../program.h"

//#include <llvm/IR/Function.h>
//#include <llvm/IR/Constants.h>
//#include <llvm/IR/Instructions.h>
//#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
//#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/mman.h>

using namespace Coal;

CPUKernel::CPUKernel(CPUDevice *device, Kernel *kernel, llvm::Function *function)
: DeviceKernel(), p_device(device), p_kernel(kernel), p_function(function),
  p_call_function(0)
{
    pthread_mutex_init(&p_call_function_mutex, 0);
}

CPUKernel::~CPUKernel()
{
    if (p_call_function)
        p_call_function->eraseFromParent();

    pthread_mutex_destroy(&p_call_function_mutex);
}

size_t CPUKernel::workGroupSize() const
{
    return 0; // TODO
}

cl_ulong CPUKernel::localMemSize() const
{
    return 0; // TODO
}

cl_ulong CPUKernel::privateMemSize() const
{
    return 0; // TODO
}

size_t CPUKernel::preferredWorkGroupSizeMultiple() const
{
    return 0; // TODO
}

template<typename T>
T k_exp(T base, unsigned int e)
{
    T rs = base;

    for (unsigned int i=1; i<e; ++i)
        rs *= base;

    return rs;
}

// Try to find the size a work group has to have to be executed the fastest on
// the CPU.
size_t CPUKernel::guessWorkGroupSize(cl_uint num_dims, cl_uint dim,
                          size_t global_work_size) const
{
    unsigned int cpus = p_device->numCPUs();

    // Don't break in too small parts
    if (k_exp(global_work_size, num_dims) > 64)
        return global_work_size;

    // Find the divisor of global_work_size the closest to cpus but >= than it
    unsigned int divisor = cpus;

    while (true)
    {
        if ((global_work_size % divisor) == 0)
            break;

        // Don't let the loop go up to global_work_size, the overhead would be
        // too huge
        if (divisor > global_work_size || divisor > cpus * 32)
        {
            divisor = 1;  // Not parallel but has no CommandQueue overhead
            break;
        }
    }

    // Return the size
    return global_work_size / divisor;
}

llvm::Function *CPUKernel::function() const
{
    return p_function;
}

Kernel *CPUKernel::kernel() const
{
    return p_kernel;
}

CPUDevice *CPUKernel::device() const
{
    return p_device;
}

// From Wikipedia : http://www.wikipedia.org/wiki/Power_of_two#Algorithm_to_round_up_to_power_of_two
template <class T>
T next_power_of_two(T k) {
        if (k == 0)
                return 1;
        k--;
        for (int i=1; i<sizeof(T)*8; i<<=1)
                k = k | k >> i;
        return k+1;
}

size_t CPUKernel::typeOffset(size_t &offset, size_t type_len)
{
    size_t rs = offset;

    // Align offset to stype_len
    type_len = next_power_of_two(type_len);
    size_t mask = ~(type_len - 1);

    while (rs & mask != rs)
        rs++;

    // Where to try to place the next value
    offset = rs + type_len;

    return rs;
}

llvm::Function *CPUKernel::callFunction()
{
    return nullptr;
#if 0
    pthread_mutex_lock(&p_call_function_mutex);

    // If we can reuse the same function between work groups, do it
    if (p_call_function)
    {
        llvm::Function *rs = p_call_function;
        pthread_mutex_unlock(&p_call_function_mutex);

        return rs;
    }

    /* Create a stub function in the form of
     *
     * void stub(void *args) {
     *     kernel(*(int *)((char *)args + 0),
     *            *(float **)((char *)args + sizeof(int)),
     *            *(sampler_t *)((char *)args + sizeof(int) + sizeof(float *)));
     * }
     *
     * In LLVM, it is exprimed in the form of :
     *
     * @stub(i8* args) {
     *     kernel(
     *         load(i32* bitcast(i8* getelementptr(i8* args, i64 0), i32*)),
     *         load(float** bitcast(i8* getelementptr(i8* args, i64 4), float**)),
     *         ...
     *     );
     * }
     */
    llvm::FunctionType *kernel_function_type = p_function->getFunctionType();
    llvm::FunctionType *stub_function_type = llvm::FunctionType::get(
        p_function->getReturnType(),
        llvm::Type::getInt8PtrTy(
            p_function->getContext()),
        false);
    llvm::Function *stub_function = llvm::Function::Create(
        stub_function_type,
        llvm::Function::InternalLinkage,
        "",
        p_function->getParent());

    // Insert a basic block
    llvm::BasicBlock *basic_block = llvm::BasicBlock::Create(
        p_function->getContext(),
        "",
        stub_function);

    // Create the function arguments
    llvm::Argument &stub_arg = stub_function->getArgumentList().front();
    llvm::SmallVector<llvm::Value *, 8> args;
    size_t args_offset = 0;

    for (unsigned int i=0; i<kernel_function_type->getNumParams(); ++i)
    {
        llvm::Type *param_type = kernel_function_type->getParamType(i);
        llvm::Type *param_type_ptr = param_type->getPointerTo(); // We'll use pointers to the value
        const Kernel::Arg &arg = p_kernel->arg(i);

        // Calculate the size of the arg
        size_t arg_size = arg.valueSize() * arg.vecDim();

        // Get where to place this argument
        size_t arg_offset = typeOffset(args_offset, arg_size);

        // %1 = getelementptr(args, $arg_offset);
        llvm::Value *getelementptr = llvm::GetElementPtrInst::CreateInBounds(
            &stub_arg,
            llvm::ConstantInt::get(stub_function->getContext(),
                                   llvm::APInt(64, arg_offset)),
            "",
            basic_block);

        // %2 = bitcast(%1, $param_type_ptr)
        llvm::Value *bitcast = new llvm::BitCastInst(
            getelementptr,
            param_type_ptr,
            "",
            basic_block);

        // %3 = load(%2)
        llvm::Value *load = new llvm::LoadInst(
            bitcast,
            "",
            false,
            arg_size,   // We ensure that an argument is always aligned on its size, it enables things like fast movaps
            basic_block);

        // We have the value, send it to the function
        args.push_back(load);
    }

    // Create the call instruction
    llvm::CallInst *call_inst = llvm::CallInst::Create(
        p_function,
        args,
        "",
        basic_block);
    call_inst->setCallingConv(p_function->getCallingConv());
    call_inst->setTailCall();

    // Create a return instruction to end the stub
    llvm::ReturnInst::Create(
        p_function->getContext(),
        basic_block);

    // Retain the function if it can be reused
    p_call_function = stub_function;

    pthread_mutex_unlock(&p_call_function_mutex);

    return stub_function;
#endif
}

/*
 * CPUKernelEvent
 */
CPUKernelEvent::CPUKernelEvent(CPUDevice *device, KernelEvent *event)
: p_device(device), p_event(event), p_current_wg(0), p_finished_wg(0),
  p_kernel_args(0)
{
    // Mutex
    pthread_mutex_init(&p_mutex, 0);

    // Set current work group to (0, 0, ..., 0)
    std::memset(p_current_work_group, 0, event->work_dim() * sizeof(size_t));

    // Populate p_max_work_groups
    p_num_wg = 1;

    for (cl_uint i=0; i<event->work_dim(); ++i)
    {
        p_max_work_groups[i] =
            (event->global_work_size(i) / event->local_work_size(i)) - 1; // 0..n-1, not 1..n

        p_num_wg *= p_max_work_groups[i] + 1;
    }
}

CPUKernelEvent::~CPUKernelEvent()
{
    pthread_mutex_destroy(&p_mutex);

    if (p_kernel_args)
        std::free(p_kernel_args);
}

bool CPUKernelEvent::reserve()
{
    // Lock, this will be unlocked in takeInstance()
    pthread_mutex_lock(&p_mutex);

    // Last work group if current == max - 1
    return (p_current_wg == p_num_wg - 1);
}

bool CPUKernelEvent::finished()
{
    bool rs;

    pthread_mutex_lock(&p_mutex);

    rs = (p_finished_wg == p_num_wg);

    pthread_mutex_unlock(&p_mutex);

    return rs;
}

void CPUKernelEvent::workGroupFinished()
{
    pthread_mutex_lock(&p_mutex);

    p_finished_wg++;

    pthread_mutex_unlock(&p_mutex);
}

CPUKernelWorkGroup *CPUKernelEvent::takeInstance()
{
    CPUKernelWorkGroup *wg = new CPUKernelWorkGroup((CPUKernel *)p_event->deviceKernel(),
                                                    p_event,
                                                    this,
                                                    p_current_work_group);

    // Increment current work group
    incVec(p_event->work_dim(), p_current_work_group, p_max_work_groups);
    p_current_wg += 1;

    // Release event
    pthread_mutex_unlock(&p_mutex);

    return wg;
}

void *CPUKernelEvent::kernelArgs() const
{
    return p_kernel_args;
}

void CPUKernelEvent::cacheKernelArgs(void *args)
{
    p_kernel_args = args;
}

/*
 * CPUKernelWorkGroup
 */
CPUKernelWorkGroup::CPUKernelWorkGroup(CPUKernel *kernel, KernelEvent *event,
                                       CPUKernelEvent *cpu_event,
                                       const size_t *work_group_index)
: p_kernel(kernel), p_cpu_event(cpu_event), p_event(event),
  p_work_dim(event->work_dim()), p_contexts(0), p_stack_size(8192 /* TODO */),
  p_had_barrier(false)
{

    // Set index
    std::memcpy(p_index, work_group_index, p_work_dim * sizeof(size_t));

    // Set maxs and global id
    p_num_work_items = 1;

    for (unsigned int i=0; i<p_work_dim; ++i)
    {
        p_max_local_id[i] = event->local_work_size(i) - 1; // 0..n-1, not 1..n
        p_num_work_items *= event->local_work_size(i);

        // Set global id
        p_global_id_start_offset[i] = (p_index[i] * event->local_work_size(i))
                         + event->global_work_offset(i);
    }
}

CPUKernelWorkGroup::~CPUKernelWorkGroup()
{
    p_cpu_event->workGroupFinished();
}

void *CPUKernelWorkGroup::callArgs(std::vector<void *> &locals_to_free)
{
    if (p_cpu_event->kernelArgs() && !p_kernel->kernel()->hasLocals())
    {
        // We have cached the args and can reuse them
        return p_cpu_event->kernelArgs();
    }

    // We need to create them from scratch
    void *rs;

    size_t args_size = 0;

    for (unsigned int i=0; i<p_kernel->kernel()->numArgs(); ++i)
    {
        const Kernel::Arg &arg = p_kernel->kernel()->arg(i);
        CPUKernel::typeOffset(args_size, arg.valueSize() * arg.vecDim());
    }

    rs = std::malloc(args_size);

    if (!rs)
        return NULL;

    size_t arg_offset = 0;

    for (unsigned int i=0; i<p_kernel->kernel()->numArgs(); ++i)
    {
        const Kernel::Arg &arg = p_kernel->kernel()->arg(i);
        size_t size = arg.valueSize() * arg.vecDim();
        size_t offset = CPUKernel::typeOffset(arg_offset, size);

        // Where to place the argument
        unsigned char *target = (unsigned char *)rs;
        target += offset;

        // We may have to perform some changes in the values (buffers, etc)
        switch (arg.kind())
        {
            case Kernel::Arg::Buffer:
            {
                MemObject *buffer = *(MemObject **)arg.data();

                if (arg.file() == Kernel::Arg::Local)
                {
                    // Alloc a buffer and pass it to the kernel
                    void *local_buffer = std::malloc(arg.allocAtKernelRuntime());
                    locals_to_free.push_back(local_buffer);
                    *(void **)target = local_buffer;
                }
                else
                {
                    if (!buffer)
                    {
                        // We can do that, just send NULL
                        *(void **)target = NULL;
                    }
                    else
                    {
                        // Get the CPU buffer, allocate it and get its pointer
                        CPUBuffer *cpubuf =
                            (CPUBuffer *)buffer->deviceBuffer(p_kernel->device());
                        void *buf_ptr = 0;

                        buffer->allocate(p_kernel->device());
                        buf_ptr = cpubuf->data();

                        *(void **)target = buf_ptr;
                    }
                }

                break;
            }
            case Kernel::Arg::Image2D:
            case Kernel::Arg::Image3D:
            {
                // We need to ensure the image is allocated
                Image2D *image = *(Image2D **)arg.data();
                image->allocate(p_kernel->device());

                // Fall through to the memcpy
            }
            default:
                // Simply copy the arg's data into the buffer
                std::memcpy(target, arg.data(), size);
                break;
        }
    }

    // Cache the arguments if we can do so
    if (!p_kernel->kernel()->hasLocals())
        p_cpu_event->cacheKernelArgs(rs);

    return rs;
}

bool CPUKernelWorkGroup::run()
{
    return false;
#if 0
    // Get the kernel function to call
    std::vector<void *> locals_to_free;
    llvm::Function *kernel_func = p_kernel->callFunction();

    if (!kernel_func)
        return false;

    Program *p = (Program *)p_kernel->kernel()->parent();
    CPUProgram *prog = (CPUProgram *)(p->deviceDependentProgram(p_kernel->device()));

    p_kernel_func_addr =
        (void(*)(void *))prog->jit()->getPointerToFunction(kernel_func);

    // Get the arguments
    p_args = callArgs(locals_to_free);

    // Tell the builtins this thread will run a kernel work group
    setThreadLocalWorkGroup(this);

    // Initialize the dummy context used by the builtins before a call to barrier()
    p_current_work_item = 0;
    p_current_context = &p_dummy_context;

    std::memset(p_dummy_context.local_id, 0, p_work_dim * sizeof(size_t));

    do
    {
        // Simply call the "call function", it and the builtins will do the rest
        p_kernel_func_addr(p_args);
    } while (!p_had_barrier &&
             !incVec(p_work_dim, p_dummy_context.local_id, p_max_local_id));

    // If no barrier() call was made, all is fine. If not, only the first
    // work-item has currently finished. We must let the others run.
    if (p_had_barrier)
    {
        Context *main_context = p_current_context; // After the first swapcontext,
                                                   // we will not be able to trust
                                                   // p_current_context anymore.

        // We'll call swapcontext for each remaining work-item. They will
        // finish, and when they'll do so, this main context will be resumed, so
        // it's easy (i starts from 1 because the main context already finished)
        for (unsigned int i=1; i<p_num_work_items; ++i)
        {
            Context *ctx = getContextAddr(i);
            swapcontext(&main_context->context, &ctx->context);
        }
    }

    // Free the allocated locals
    if (p_kernel->kernel()->hasLocals())
    {
        for (size_t i=0; i<locals_to_free.size(); ++i)
        {
            std::free(locals_to_free[i]);
        }

        std::free(p_args);
    }

    return true;
#endif
}

CPUKernelWorkGroup::Context *CPUKernelWorkGroup::getContextAddr(unsigned int index)
{
    size_t size;
    char *data = (char *)p_contexts;

    // Each Context in data is an element of size p_stack_size + sizeof(Context)
    size = p_stack_size + sizeof(Context);
    size *= index;  // To get an offset

    return (Context *)(data + size); // Pointer to the context
}
