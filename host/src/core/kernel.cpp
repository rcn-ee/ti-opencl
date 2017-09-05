/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2012-2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * \file core/kernel.cpp
 * \brief Kernel
 */

#include "kernel.h"
#include "propertylist.h"
#include "program.h"
#include "memobject.h"
#include "sampler.h"
#include "context.h"
#include "deviceinterface.h"

#include <string>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <llvm/Support/Casting.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Metadata.h>


using namespace Coal;
Kernel::Kernel(Program *program)
: Object(Object::T_Kernel, program), p_has_locals(false), wi_alloca_size(0),
  p_timeout_ms(0)
{
    // TODO: Say a kernel is attached to the program (that becomes unalterable)

    null_dep.device   = 0;
    null_dep.kernel   = 0;
    null_dep.function = 0;
    null_dep.module   = 0;
}

Kernel::~Kernel()
{
    while (p_device_dependent.size())
    {
        DeviceDependent &dep = p_device_dependent.back();

        delete dep.kernel;

        p_device_dependent.pop_back();
    }
}

const Kernel::DeviceDependent &Kernel::deviceDependent(DeviceInterface *device) const
{
    for (size_t i=0; i<p_device_dependent.size(); ++i)
    {
        const DeviceDependent &rs = p_device_dependent[i];

        if (rs.device == device || (!device && p_device_dependent.size() == 1))
            return rs;
    }

    return null_dep;
}

Kernel::DeviceDependent &Kernel::deviceDependent(DeviceInterface *device)
{
    for (size_t i=0; i<p_device_dependent.size(); ++i)
    {
        DeviceDependent &rs = p_device_dependent[i];

        if (rs.device == device || (!device && p_device_dependent.size() == 1))
            return rs;
    }

    return null_dep;
}


/******************************************************************************
* cl_int Kernel::addFunction
******************************************************************************/
cl_int Kernel::addFunction(DeviceInterface *device, llvm::Function *function,
                           llvm::Module *module)
{
    p_name = function->getName().str();

    // Get wi_alloca_size, to be used for computing wg_alloca_size
    std::string WAS_str = function->getAttributes().getAttribute
                         (llvm::AttributeSet::FunctionIndex, "_wi_alloca_size")
                                             .getValueAsString();

    //YUAN if (!WAS_str.empty()) wi_alloca_size = std::stoi(WAS_str);
    if (!WAS_str.empty()) wi_alloca_size = atoi(WAS_str.c_str());

    /*-------------------------------------------------------------------------
    * Add a device dependent
    *------------------------------------------------------------------------*/
    DeviceDependent dep;

    dep.device   = device;
    dep.function = function;
    dep.module   = module;

    /*-------------------------------------------------------------------------
    * Build the arg list of the kernel (or verify it if a previous function
    * was already registered)
    *------------------------------------------------------------------------*/
    llvm::FunctionType *f = function->getFunctionType();
    bool append = (p_args.size() == 0);

    if (!append && p_args.size() != f->getNumParams())
        return CL_INVALID_KERNEL_DEFINITION;

    for (unsigned int i=0; i<f->getNumParams(); ++i)
    {
        llvm::Type *arg_type = f->getParamType(i);
        Arg::Kind kind = Arg::Invalid;
        Arg::File file = Arg::Private;
        bool is_subword_int_uns = false;
        unsigned short vec_dim = 1;

        if (arg_type->isPointerTy())
        {
            // It's a pointer, dereference it
            llvm::PointerType *p_type = llvm::cast<llvm::PointerType>(arg_type);

            file = (Arg::File)p_type->getAddressSpace();
            arg_type = p_type->getElementType();

            // If it's a __local argument, we'll have to allocate memory at run time
            if (file == Arg::Local)
                p_has_locals = true;

            kind = Arg::Buffer;

            // If it's a struct, get its name
            if (arg_type->isStructTy())
            {
                llvm::StructType *struct_type =
                    llvm::cast<llvm::StructType>(arg_type);
                std::string struct_name = struct_type->getName().str();

                if (struct_name.compare(0, 14, "struct.image2d") == 0)
                {
                    kind = Arg::Image2D;
                    file = Arg::Global;
                }
                else if (struct_name.compare(0, 14, "struct.image3d") == 0)
                {
                    kind = Arg::Image3D;
                    file = Arg::Global;
                }
            }
        }
        else
        {
            if (arg_type->isVectorTy())
            {
                // It's a vector, we need its element's type
                llvm::VectorType *v_type = llvm::cast<llvm::VectorType>(arg_type);

                vec_dim = v_type->getNumElements();
                arg_type = v_type->getElementType();
            }

            // Get type kind
            if (arg_type->isFloatTy())
            {
                kind = Arg::Float;
            }
            else if (arg_type->isDoubleTy())
            {
                kind = Arg::Double;
            }
            else if (arg_type->isIntegerTy())
            {
                llvm::IntegerType *i_type = llvm::cast<llvm::IntegerType>(arg_type);

                /*-------------------------------------------------------------
                * We offset the arg index by 1, because element 0 is the return 
                * type.
                *------------------------------------------------------------*/
                is_subword_int_uns = function->getAttributes().
                                     hasAttribute(i+1, llvm::Attribute::ZExt);

                if (i_type->getBitWidth() == 8)
                {
                    kind = Arg::Int8;
                }
                else if (i_type->getBitWidth() == 16)
                {
                    kind = Arg::Int16;
                }
                else if (i_type->getBitWidth() == 32)
                {
                    // NOTE: May also be a sampler, check done in setArg
                    kind = Arg::Int32;
                }
                else if (i_type->getBitWidth() == 64)
                {
                    kind = Arg::Int64;
                }
            }
        }

        // Check if we recognized the type
        if (kind == Arg::Invalid)
            return CL_INVALID_KERNEL_DEFINITION;

        // Create arg
        Arg a(vec_dim, file, kind, is_subword_int_uns);

        // If we also have a function registered, check for signature compliance
        if (!append && a != p_args[i])
            return CL_INVALID_KERNEL_DEFINITION;

        // Append arg if needed
        if (append)
            p_args.push_back(a);
    }

    dep.kernel = device->createDeviceKernel(this, dep.function);
    p_device_dependent.push_back(dep);

    return CL_SUCCESS;
}

llvm::Function *Kernel::function(DeviceInterface *device) const
{
    const DeviceDependent &dep = deviceDependent(device);

    return dep.function;
}

/******************************************************************************
* cl_int Kernel::setArg
*
* Note: the argument void *value can either be a pointer to raw data, or a
* derived type of MemObject, upcast to an ICD descriptor (see icd.h).
* In this case, we must be careful to distinguish between the two and do the
* downcast to a clover object if valule is a pointer to an ICD object.
******************************************************************************/
cl_int Kernel::setArg(cl_uint index, size_t size, const void *value)
{
    if (index > p_args.size())
        return CL_INVALID_ARG_INDEX;

    Arg &arg = p_args[index];

    /*-------------------------------------------------------------------------
    * Special case for __local pointers
    *------------------------------------------------------------------------*/
    if (arg.file() == Arg::Local)
    {
        if (size == 0)  return CL_INVALID_ARG_SIZE;
        if (value != 0) return CL_INVALID_ARG_VALUE;

        arg.setAllocAtKernelRuntime(size);
        return CL_SUCCESS;
    }

    /*-------------------------------------------------------------------------
    * Check that size corresponds to the arg type
    *------------------------------------------------------------------------*/
    size_t arg_size = arg.vecValueSize();

    /*-------------------------------------------------------------------------
    * Special case for samplers (pointers in C++, uint32 in OpenCL).
    *------------------------------------------------------------------------*/
    if (size == sizeof(cl_sampler) && arg_size == 4 &&
        (*(Object **)value)->isA(T_Sampler))
    {
        unsigned int bitfield = (*(Sampler **)value)->bitfield();

        arg.refineKind(Arg::Sampler);
        arg.alloc();
        arg.loadData(&bitfield);

        return CL_SUCCESS;
    }

    if (size != arg_size) return CL_INVALID_ARG_SIZE;

    /*-------------------------------------------------------------------------
    * Downcast 'void *value' from a potential &cl_mem argument to a MemObject
    * if arg type is one of Arg::Buffer, Arg::Image2D, or Arg::Image3D.
    * Also, check for null values.
    *------------------------------------------------------------------------*/
    MemObject *mem_value = NULL;

    switch (arg.kind())
    {
        /*-------------------------------------------------------------
        * Special case buffers : value can be 0 (or point to 0)
        *------------------------------------------------------------*/
        case Arg::Buffer:
        case Arg::Image2D:
        case Arg::Image3D:
            if (value) {
                mem_value = pobj(*(cl_mem *)value);
            }
            value = &mem_value;
            break;
        default:
            if (!value) return CL_INVALID_ARG_VALUE;
    }

    /*-------------------------------------------------------------------------
    * Copy the data
    *------------------------------------------------------------------------*/
    arg.alloc();
    arg.loadData(value);

    return CL_SUCCESS;
}

unsigned int Kernel::numArgs() const
{
    return p_args.size();
}

const Kernel::Arg &Kernel::arg(unsigned int index) const
{
    return p_args.at(index);
}

bool Kernel::argsSpecified() const
{
    for (size_t i=0; i<p_args.size(); ++i)
        if (!p_args[i].defined()) return false;
    return true;
}

bool Kernel::hasLocals() const
{
    return p_has_locals;
}

DeviceKernel *Kernel::deviceDependentKernel(DeviceInterface *device) const
{
    const DeviceDependent &dep = deviceDependent(device);

    return dep.kernel;
}

llvm::Module *Kernel::deviceDependentModule(DeviceInterface *device) const
{
    const DeviceDependent &dep = deviceDependent(device);

    return dep.module;
}

cl_int Kernel::info(cl_kernel_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const
{
    void *value = 0;
    size_t value_length = 0;

    union {
        cl_uint cl_uint_var;
        cl_program cl_program_var;
        cl_context cl_context_var;
    };

    switch (param_name)
    {
        case CL_KERNEL_FUNCTION_NAME:
            MEM_ASSIGN(p_name.size() + 1, p_name.c_str());
            break;

        case CL_KERNEL_NUM_ARGS:
            SIMPLE_ASSIGN(cl_uint, p_args.size());
            break;

        case CL_KERNEL_REFERENCE_COUNT:
            SIMPLE_ASSIGN(cl_uint, references());
            break;

        case CL_KERNEL_CONTEXT:
            SIMPLE_ASSIGN(cl_context, desc((Context *)(parent()->parent())));
            break;

        case CL_KERNEL_PROGRAM:
            SIMPLE_ASSIGN(cl_program, desc((Program *)parent()));
            break;

        default:
            return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < value_length)
        return CL_INVALID_VALUE;

    if (param_value_size_ret)
        *param_value_size_ret = value_length;

    if (param_value)
        std::memcpy(param_value, value, value_length);

    return CL_SUCCESS;
}

std::string Kernel::get_name(){
    return p_name;
}


void Kernel::reqdWorkGroupSize(llvm::Module *module, cl_uint dims[3]) const
{
    llvm::NamedMDNode *kernels = module->getNamedMetadata("opencl.kernels");
    llvm::MDNode *kernel = NULL;

    dims[0] = dims[1] = dims[2] = 0;

    if (!kernels) return;
   
    for (unsigned int i = 0, e = kernels->getNumOperands(); i != e; ++i){
       llvm::MDNode *kernel_iter = kernels->getOperand(i);

       assert (kernel_iter->getNumOperands() > 0);

       /*---------------------------------------------------------------------
       * Each node has only one operand : a llvm::Function
       *--------------------------------------------------------------------*/
       llvm::Function *f = 
          llvm::cast<llvm::Function>(
             llvm::dyn_cast<llvm::ValueAsMetadata>(
                kernel_iter->getOperand(0))->getValue());
    
       if(f->getName().str() != p_name) continue;
       kernel = kernel_iter;
    }

    if (!kernel) return;

    unsigned e = kernel->getNumOperands();
    for (unsigned int i=1; i != e; ++i)
    {
        llvm::MDNode *meta = llvm::cast<llvm::MDNode>(kernel->getOperand(i));

        if (meta->getNumOperands() <= 1) return;

        std::string meta_name = llvm::cast<llvm::MDString>(
              meta->getOperand(0))->getString().str();
        if ((meta->getNumOperands() == 4) && 
            (meta_name == "reqd_work_group_size"))
        {
	    dims[0] = (llvm::cast<llvm::ConstantInt>(
               llvm::cast<llvm::ConstantAsMetadata>(
                  meta->getOperand(1))->getValue()))->getLimitedValue();
	    dims[1] = (llvm::cast<llvm::ConstantInt>(
               llvm::cast<llvm::ConstantAsMetadata>(
                  meta->getOperand(2))->getValue()))->getLimitedValue();
	    dims[2] = (llvm::cast<llvm::ConstantInt>(
               llvm::cast<llvm::ConstantAsMetadata>(
                  meta->getOperand(3))->getValue()))->getLimitedValue();

            return;
        }
    }
}


cl_int Kernel::workGroupInfo(DeviceInterface *device,
                             cl_kernel_work_group_info param_name,
                             size_t param_value_size,
                             void *param_value,
                             size_t *param_value_size_ret) const
{
    void *value = 0;
    size_t value_length = 0;

    union {
        size_t size_t_var;
        size_t three_size_t[3];
        cl_ulong cl_ulong_var;
    };

    const DeviceDependent &dep = deviceDependent(device);

    switch (param_name)
    {
        case CL_KERNEL_WORK_GROUP_SIZE:
            SIMPLE_ASSIGN(size_t, dep.kernel->workGroupSize());
            break;

        case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
            {
            cl_uint dims[3];
            reqdWorkGroupSize(dep.module, dims);
            three_size_t[0] = dims[0];
            three_size_t[1] = dims[1];
            three_size_t[2] = dims[2];
            value = &three_size_t;
            value_length = sizeof(three_size_t);
            }
            break;

        case CL_KERNEL_LOCAL_MEM_SIZE:
            SIMPLE_ASSIGN(cl_ulong, dep.kernel->localMemSize());
            break;

        case CL_KERNEL_PRIVATE_MEM_SIZE:
            SIMPLE_ASSIGN(cl_ulong, dep.kernel->privateMemSize());
            break;

        case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
            SIMPLE_ASSIGN(size_t, dep.kernel->preferredWorkGroupSizeMultiple());
            break;

        default:
            return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < value_length)
        return CL_INVALID_VALUE;

    if (param_value_size_ret)
        *param_value_size_ret = value_length;

    if (param_value)
        std::memcpy(param_value, value, value_length);

    return CL_SUCCESS;
}

/*
 * Kernel::Arg
 */
Kernel::Arg::Arg(unsigned short vec_dim, File file, Kind kind, bool is_subword_int_uns)
: p_vec_dim(vec_dim), p_file(file), p_kind(kind), p_data(0), p_defined(false),
  p_runtime_alloc(0), p_is_subword_int_uns(is_subword_int_uns)
{ }

Kernel::Arg::~Arg()
{
    if (p_data) std::free(p_data);
}

void Kernel::Arg::alloc()
{
    if (!p_data) p_data = std::malloc(vecValueSize());
}

void Kernel::Arg::loadData(const void *data)
{
    std::memcpy(p_data, data, p_vec_dim * valueSize());
    p_defined = true;
}

void Kernel::Arg::setAllocAtKernelRuntime(size_t size)
{
    p_runtime_alloc = size;
    p_defined       = true;
}

void Kernel::Arg::refineKind (Kernel::Arg::Kind kind)
{
    p_kind = kind;
}

bool Kernel::Arg::operator!=(const Arg &b)
{
    bool same = (p_vec_dim == b.p_vec_dim) &&
                (p_file    == b.p_file) &&
                (p_kind    == b.p_kind);

    return !same;
}

size_t Kernel::Arg::valueSize() const
{
    switch (p_kind)
    {
        case Invalid: return 0;
        case Int8:    return 1;
        case Int16:   return 2;
        case Int32:
        case Sampler: return 4;
        case Int64:   return 8;
        case Float:   return sizeof(cl_float);
        case Double:  return sizeof(double);
        case Buffer:
        case Image2D:
        case Image3D: return sizeof(cl_mem);
    }

    return 0;
}

size_t Kernel::Arg::vecValueSize() const
{
  return valueSize() * (p_vec_dim == 3 ? 4 : p_vec_dim);
}

unsigned short    Kernel::Arg::vecDim()    const { return p_vec_dim; }
Kernel::Arg::File Kernel::Arg::file()      const { return p_file;    }
Kernel::Arg::Kind Kernel::Arg::kind()      const { return p_kind;    }
bool              Kernel::Arg::defined()   const { return p_defined; }
const void *      Kernel::Arg::data()      const { return p_data;    }
size_t Kernel::Arg::allocAtKernelRuntime() const {return p_runtime_alloc;}
bool Kernel::Arg::is_subword_int_uns()   const {return p_is_subword_int_uns;}

const void *Kernel::Arg::value(unsigned short index) const
{
    const char *data = (const char *)p_data;
    unsigned int offset = index * valueSize();

    data += offset;

    return (const void *)data;
}



