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
 * \file core/program.cpp
 * \brief Program
 */

#include "program.h"
#include "context.h"
#include "compiler.h"
#include "kernel.h"
#include "propertylist.h"
#include "deviceinterface.h"

#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Casting.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Linker.h>
#include <llvm/PassManager.h>
#include <llvm/Metadata.h>
#include <llvm/Function.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/IPO.h>

#include <runtime/stdlib.h.embed.h>
#include <runtime/stdlib.c.bc.embed.h>

using namespace Coal;

Program::Program(Context *ctx)
: Object(Object::T_Program, ctx), p_type(Invalid), p_state(Empty)
{
    p_null_device_dependent.compiler = 0;
    p_null_device_dependent.device = 0;
    p_null_device_dependent.linked_module = 0;
    p_null_device_dependent.program = 0;
}

Program::~Program()
{
    while (p_device_dependent.size())
    {
        DeviceDependent &dep = p_device_dependent.back();

        delete dep.compiler;
        delete dep.program;
        delete dep.linked_module;

        p_device_dependent.pop_back();
    }
}

void Program::setDevices(cl_uint num_devices, DeviceInterface * const*devices)
{
    p_device_dependent.resize(num_devices);

    for (cl_uint i=0; i<num_devices; ++i)
    {
        DeviceDependent &dep = p_device_dependent[i];

        dep.device        = devices[i];
        dep.program       = dep.device->createDeviceProgram(this);
        dep.linked_module = 0;
        dep.compiler      = new Compiler(dep.device);
    }
}

Program::DeviceDependent &Program::deviceDependent(DeviceInterface *device)
{
    for (size_t i=0; i<p_device_dependent.size(); ++i)
    {
        DeviceDependent &rs = p_device_dependent[i];

        if (rs.device == device || (!device && p_device_dependent.size() == 1))
            return rs;
    }

    return p_null_device_dependent;
}

const Program::DeviceDependent &Program::deviceDependent(DeviceInterface *device) const
{
    for (size_t i=0; i<p_device_dependent.size(); ++i)
    {
        const DeviceDependent &rs = p_device_dependent[i];

        if (rs.device == device || (!device && p_device_dependent.size() == 1))
            return rs;
    }

    return p_null_device_dependent;
}

DeviceProgram *Program::deviceDependentProgram(DeviceInterface *device) const
{
    const DeviceDependent &dep = deviceDependent(device);

    return dep.program;
}

std::string Program::deviceDependentCompilerOptions(DeviceInterface *device) const
{
    const DeviceDependent &dep = deviceDependent(device);

    return dep.compiler->options();
}

std::vector<llvm::Function *> Program::kernelFunctions(DeviceDependent &dep)
{
    std::vector<llvm::Function *> rs;

    llvm::NamedMDNode *kernels = 
               dep.linked_module->getNamedMetadata("opencl.kernels");

    if (!kernels) return rs;

    for (unsigned int i=0; i<kernels->getNumOperands(); ++i)
    {
        llvm::MDNode *node = kernels->getOperand(i);

        /*---------------------------------------------------------------------
        * Each node has only one operand : a llvm::Function
        *--------------------------------------------------------------------*/
        llvm::Value *value = node->getOperand(0);

        /*---------------------------------------------------------------------
        * Bug somewhere, don't crash
        *--------------------------------------------------------------------*/
        if (!llvm::isa<llvm::Function>(value)) continue;       

        llvm::Function *f = llvm::cast<llvm::Function>(value);
        rs.push_back(f);
    }

    return rs;
}

/******************************************************************************
* Kernel *Program::createKernel(const std::string &name, cl_int *errcode_ret)
******************************************************************************/
Kernel *Program::createKernel(const std::string &name, cl_int *errcode_ret)
{
    Kernel *rs = new Kernel(this);

    /*-------------------------------------------------------------------------
    * Add a function definition for each device
    *------------------------------------------------------------------------*/
    for (size_t i=0; i < p_device_dependent.size(); ++i)
    {
        bool found = false;
        DeviceDependent &dep = p_device_dependent[i];
        const std::vector<llvm::Function *> &kernels = kernelFunctions(dep);

        /*---------------------------------------------------------------------
        * Find the one with the good name
        *--------------------------------------------------------------------*/
        for (size_t j=0; j < kernels.size(); ++j)
        {
            llvm::Function *func = kernels[j];

            if (func->getNameStr() == name)
            {
                found = true;
                *errcode_ret = rs->addFunction(dep.device, func, 
                                               dep.linked_module);
                if (*errcode_ret != CL_SUCCESS) return rs;
                break;
            }
        }

        /*---------------------------------------------------------------------
        * Kernel unavailable for this device
        *--------------------------------------------------------------------*/
        if (!found)
        {
            *errcode_ret = CL_INVALID_KERNEL_NAME;
            return rs;
        }
    }

    return rs;
}

std::vector<Kernel *> Program::createKernels(cl_int *errcode_ret)
{
    std::vector<Kernel *> rs;

    /*-------------------------------------------------------------------------
    * We should never go here
    *------------------------------------------------------------------------*/
    if (p_device_dependent.size() == 0) return rs;

    /*-------------------------------------------------------------------------
    * Take the list of kernels for the first device dependent
    *------------------------------------------------------------------------*/
    DeviceDependent &dep = p_device_dependent[0];
    const std::vector<llvm::Function *> &kernels = kernelFunctions(dep);

    /*-------------------------------------------------------------------------
    * Create the kernel for each function name
    * It returns an error if the signature is not the same for every device
    * or if the kernel isn't found on all the devices.
    *------------------------------------------------------------------------*/
    for (size_t i=0; i < kernels.size(); ++i)
    {
        cl_int result  = CL_SUCCESS;
        Kernel *kernel = createKernel(kernels[i]->getNameStr(), &result);

        if (result == CL_SUCCESS) rs.push_back(kernel);
        else                      delete kernel;
    }

    return rs;
}

cl_int Program::loadSources(cl_uint count, const char **strings,
                            const size_t *lengths)
{
    p_source  = std::string(embed_stdlib_h);
    p_source += std::string("/*EOH Marker*/\n");

    // Merge all strings into one big one
    for (cl_uint i=0; i<count; ++i)
    {
        size_t len = 0;
        const char *data = strings[i];

        if (!data)
            return CL_INVALID_VALUE;

        // Get the length of the source
        if (lengths && lengths[i])
            len = lengths[i];
        else
            len = std::strlen(data);

        // Remove trailing \0's, it's not good for sources (it can arise when
        // the client application wrongly sets lengths
        while (len > 0 && data[len-1] == 0)
            len--;

        // Merge the string
        std::string part(data, len);
        p_source += part;
    }

    p_type = Source;
    p_state = Loaded;

    return CL_SUCCESS;
}

cl_int Program::loadBinaries(const unsigned char **data, const size_t *lengths,
                             cl_int *binary_status, cl_uint num_devices,
                             DeviceInterface * const*device_list)
{
    // Set device infos
    setDevices(num_devices, device_list);

    // Load the data
    for (cl_uint i=0; i<num_devices; ++i)
    {
        DeviceDependent &dep = deviceDependent(device_list[i]);

        // Load bitcode
        dep.unlinked_binary = std::string((const char *)data[i], lengths[i]);

        // Make a module of it
        const llvm::StringRef s_data(dep.unlinked_binary);
        const llvm::StringRef s_name("<binary>");

        llvm::MemoryBuffer *buffer = llvm::MemoryBuffer::getMemBuffer(s_data,
                                                                      s_name,
                                                                      false);

        if (!buffer)
            return CL_OUT_OF_HOST_MEMORY;

        dep.linked_module = ParseBitcodeFile(buffer, llvm::getGlobalContext());

        if (!dep.linked_module)
        {
            if (binary_status) binary_status[i] = CL_INVALID_VALUE;
            return CL_INVALID_BINARY;
        }

        if (binary_status) binary_status[i] = CL_SUCCESS;
    }

    p_type = Binary;
    p_state = Loaded;

    return CL_SUCCESS;
}

cl_int Program::build(const char *options,
                      void (CL_CALLBACK *pfn_notify)(cl_program program,
                                                     void *user_data),
                      void *user_data, cl_uint num_devices,
                      DeviceInterface * const*device_list)
{
    p_state = Failed;

    // Set device infos
    if (!p_device_dependent.size())
    {
        setDevices(num_devices, device_list);
    }

    // ASW TODO - optimize to compile for each device type only once.
    for (cl_uint i=0; i<p_device_dependent.size(); ++i)
    {
        DeviceDependent &dep = deviceDependent(device_list[i]);

        // Do we need to compile the source for each device ?
        if (p_type == Source)
        {
            // Load source
            const llvm::StringRef s_data(p_source);
            const llvm::StringRef s_name("<source>");

            llvm::MemoryBuffer *buffer = llvm::MemoryBuffer::getMemBuffer(s_data,
                                                                        s_name);

            // Compile
            if (!dep.compiler->compile(options ? options : std::string(), buffer))
            {
                if (pfn_notify)
                    pfn_notify((cl_program)this, user_data);

                return CL_BUILD_PROGRAM_FAILURE;
            }

            // Get module and its bitcode
            dep.linked_module = dep.compiler->module();

            llvm::raw_string_ostream ostream(dep.unlinked_binary);
            llvm::WriteBitcodeToFile(dep.linked_module, ostream);
            ostream.flush();
        }

        // Link p_linked_module with the stdlib if the device needs that
        if (dep.program->linkStdLib())
        {
            // Load the stdlib bitcode
            const llvm::StringRef s_data(embed_stdlib_c_bc,
                                         sizeof(embed_stdlib_c_bc) - 1);
            const llvm::StringRef s_name("stdlib.bc");
            std::string errMsg;

            llvm::MemoryBuffer *buffer = llvm::MemoryBuffer::getMemBuffer(s_data,
                                                                          s_name,
                                                                          false);

            if (!buffer)
                return CL_OUT_OF_HOST_MEMORY;

            llvm::Module *stdlib = ParseBitcodeFile(buffer,
                                                    llvm::getGlobalContext(),
                                                    &errMsg);

            // Link
            if (!stdlib ||
                llvm::Linker::LinkModules(dep.linked_module, stdlib,
                                          llvm::Linker::DestroySource, &errMsg))
            {
                dep.compiler->appendLog("link error: ");
                dep.compiler->appendLog(errMsg);
                dep.compiler->appendLog("\n");

                // DEBUG
                std::cout << dep.compiler->log() << std::endl;

                if (pfn_notify)
                    pfn_notify((cl_program)this, user_data);

                return CL_BUILD_PROGRAM_FAILURE;
            }
        }

        // Get list of kernels to strip other unused functions
        std::vector<const char *> api;
        std::vector<std::string> api_s;     // Needed to keep valid data in api
        const std::vector<llvm::Function *> &kernels = kernelFunctions(dep);

        for (size_t j=0; j<kernels.size(); ++j)
        {
            std::string s = kernels[j]->getNameStr();

            api_s.push_back(s);
            api.push_back(s.c_str());
        }

        // Optimize code
        llvm::PassManager *manager = new llvm::PassManager();

        // Common passes (primary goal : remove unused stdlib functions)
        manager->add(llvm::createTypeBasedAliasAnalysisPass());
        manager->add(llvm::createBasicAliasAnalysisPass());
        manager->add(llvm::createInternalizePass(api));
        manager->add(llvm::createIPSCCPPass());
        manager->add(llvm::createGlobalOptimizerPass());
        manager->add(llvm::createConstantMergePass());

        dep.program->createOptimizationPasses(manager, dep.compiler->optimize());

        manager->add(llvm::createGlobalDCEPass());

        manager->run(*dep.linked_module);
        delete manager;

        // Now that the LLVM module is built, build the device-specific
        // representation
        if (!dep.program->build(dep.linked_module))
        {
            if (pfn_notify)
                pfn_notify((cl_program)this, user_data);

            return CL_BUILD_PROGRAM_FAILURE;
        }
    }

    // TODO: Asynchronous compile
    if (pfn_notify)
        pfn_notify((cl_program)this, user_data);

    p_state = Built;

    return CL_SUCCESS;
}

Program::Type Program::type() const
{
    return p_type;
}

Program::State Program::state() const
{
    return p_state;
}

cl_int Program::info(cl_program_info param_name,
                     size_t param_value_size,
                     void *param_value,
                     size_t *param_value_size_ret) const
{
    void *value = 0;
    size_t value_length = 0;
    llvm::SmallVector<size_t, 4> binary_sizes;
    llvm::SmallVector<DeviceInterface *, 4> devices;

    union {
        cl_uint cl_uint_var;
        cl_context cl_context_var;
    };

    switch (param_name)
    {
        case CL_PROGRAM_REFERENCE_COUNT:
            SIMPLE_ASSIGN(cl_uint, references());
            break;

        case CL_PROGRAM_NUM_DEVICES:
            SIMPLE_ASSIGN(cl_uint, p_device_dependent.size());
            break;

        case CL_PROGRAM_DEVICES:
            for (size_t i=0; i<p_device_dependent.size(); ++i)
            {
                const DeviceDependent &dep = p_device_dependent[i];

                devices.push_back(dep.device);
            }

            value = devices.data();
            value_length = devices.size() * sizeof(DeviceInterface *);
            break;

        case CL_PROGRAM_CONTEXT:
            SIMPLE_ASSIGN(cl_context, parent());
            break;

        case CL_PROGRAM_SOURCE:
            MEM_ASSIGN(p_source.size() + 1, p_source.c_str());
            break;

        case CL_PROGRAM_BINARY_SIZES:
            for (size_t i=0; i<p_device_dependent.size(); ++i)
            {
                const DeviceDependent &dep = p_device_dependent[i];

                binary_sizes.push_back(dep.unlinked_binary.size());
            }

            value = binary_sizes.data();
            value_length = binary_sizes.size() * sizeof(size_t);
            break;

        case CL_PROGRAM_BINARIES:
        {
            // Special case : param_value points to an array of p_num_devices
            // application-allocated unsigned char* pointers. Check it's good
            // and std::memcpy the data

            unsigned char **binaries = (unsigned char **)param_value;
            value_length = p_device_dependent.size() * sizeof(unsigned char *);

            if (!param_value || param_value_size < value_length)
                return CL_INVALID_VALUE;

            for (size_t i=0; i<p_device_dependent.size(); ++i)
            {
                const DeviceDependent &dep = p_device_dependent[i];
                unsigned char *dest = binaries[i];

                if (!dest)
                    continue;

                std::memcpy(dest, dep.unlinked_binary.data(),
                            dep.unlinked_binary.size());
            }

            if (param_value_size_ret)
                *param_value_size_ret = value_length;

            return CL_SUCCESS;
        }

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

cl_int Program::buildInfo(DeviceInterface *device,
                          cl_program_build_info param_name,
                          size_t param_value_size,
                          void *param_value,
                          size_t *param_value_size_ret) const
{
    const void *value = 0;
    size_t value_length = 0;
    const DeviceDependent &dep = deviceDependent(device);

    union {
        cl_build_status cl_build_status_var;
    };

    switch (param_name)
    {
        case CL_PROGRAM_BUILD_STATUS:
            switch (p_state)
            {
                case Empty:
                case Loaded:
                    SIMPLE_ASSIGN(cl_build_status, CL_BUILD_NONE);
                    break;
                case Built:
                    SIMPLE_ASSIGN(cl_build_status, CL_BUILD_SUCCESS);
                    break;
                case Failed:
                    SIMPLE_ASSIGN(cl_build_status, CL_BUILD_ERROR);
                    break;
                // TODO: CL_BUILD_IN_PROGRESS
            }
            break;

        case CL_PROGRAM_BUILD_OPTIONS:
            value = dep.compiler->options().c_str();
            value_length = dep.compiler->options().size() + 1;
            break;

        case CL_PROGRAM_BUILD_LOG:
            value = dep.compiler->log().c_str();
            value_length = dep.compiler->log().size() + 1;
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
