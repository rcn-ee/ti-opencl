/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2012-2016, Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef _SYS_BIOS
#include "compiler.h"
#endif
#include "kernel.h"
#include "propertylist.h"
#include "deviceinterface.h"
#include "dsp/device.h"
#include "dsp/program.h"
#include "util.h"

#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <utility>

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Casting.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/LLVMContext.h>

#ifndef _SYS_BIOS
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Linker/Linker.h"
#endif

#ifndef _SYS_BIOS
#include <runtime/stdlib.c.bc.embed.h>

#include "dsp/genfile_cache.h"
#endif

#ifndef _SYS_BIOS
#include <elf.h>
#endif
/*-----------------------------------------------------------------------------
* temporary for source file cacheing, remove from product releases
*----------------------------------------------------------------------------*/
//#include "dsp/source_cache.h"
//source_cache * source_cache::pInstance = 0;

using namespace Coal;

static llvm::Module *BitcodeToLLVMModule(const std::string &bitcode,
                                         llvm::LLVMContext &llvmcontext);
#ifndef _SYS_BIOS
bool LinkLLVMModules(llvm::Module* linked_module,
                     const std::vector<llvm::Module*>& modules,
                     llvm::LLVMContext& llvmcontext);

static bool ReadBinaryIntoString(const std::string &outfile,
                                       std::string &binary_str);
#endif

Program::Program(Context *ctx)
: Object(Object::T_Program, ctx), p_type(Invalid), p_state(Empty),
  p_device_list(), p_kernel_names(""), p_num_kernels(0)
{
#ifndef _SYS_BIOS
    p_null_device_dependent.compiler = 0;
#endif
    p_null_device_dependent.device = 0;
    p_null_device_dependent.linked_module = 0;
    p_null_device_dependent.program = 0;
    p_llvmcontext = new llvm::LLVMContext();
}

Program::~Program()
{
   resetDeviceDependent();
   delete p_llvmcontext;
}

void Program::resetDeviceDependent()
{
    while (p_device_dependent.size())
    {
        DeviceDependent &dep = p_device_dependent.back();

#ifndef _SYS_BIOS
        delete dep.compiler;
#endif
        delete dep.program;
        delete dep.linked_module;
        dep.unlinked_binary.clear();

        if (dep.native_binary_filename) delete [] dep.native_binary_filename;

        p_device_dependent.pop_back();
    }
}

void Program::setDevices(cl_uint num_devices, const cl_device_id *devices)
{
    /* device_list: Keep a record of the devices the user requested to build
     * this program for */
    p_device_list.clear();

    std::set<DeviceInterface*> root_devices;
    for (cl_uint i = 0; i < num_devices; i++)
    {
        DeviceInterface *device = pobj(devices[i]);
        p_device_list.push_back(device);

        DeviceInterface *root_device = device->GetRootDevice();
        root_devices.insert(root_device);
    }

    /* device_dependent: Only keep device dependent programs for root devices */
    resetDeviceDependent();
    p_device_dependent.resize(root_devices.size());

    cl_uint i = 0;
    for (DeviceInterface *root_device : root_devices)
    {
        DeviceDependent &dep = p_device_dependent[i++];

        dep.device                 = root_device;
        dep.program                = dep.device->createDeviceProgram(this);
        dep.is_native_binary       = false;
        dep.native_binary_filename = NULL;
        dep.linked_module          = 0;
#ifndef _SYS_BIOS
        dep.compiler               = new Compiler(dep.device);
#endif
    }
}

const Program::DeviceDependent& Program::deviceDependent(DeviceInterface* device) const
{
    for (size_t i = 0; i < p_device_dependent.size(); ++i)
    {
        const DeviceDependent& rs = p_device_dependent[i];

        if (!device)
        {
            if (p_device_dependent.size() == 1)
                return rs;
        }
        else if (rs.device == device || rs.device == device->GetRootDevice())
            return rs;
    }

    return p_null_device_dependent;
}

Program::DeviceDependent& Program::deviceDependent(DeviceInterface* device)
{
    // Avoid code duplication by casting const away for the non-const method
    const Program& P = *this;
    return const_cast<Program::DeviceDependent&>(P.deviceDependent(device));
}

DeviceProgram *Program::deviceDependentProgram(DeviceInterface *device) const
{
    const DeviceDependent &dep = deviceDependent(device);

    return dep.program;
}

std::string Program::deviceDependentCompilerOptions(DeviceInterface *device) const
{
    const DeviceDependent &dep = deviceDependent(device);

#ifndef _SYS_BIOS
    return dep.compiler->options();
#else
    return "error";
#endif
}

std::vector<llvm::MDNode *> Program::KernelMDNodes(const DeviceDependent &dep) const
{
    std::vector<llvm::MDNode *> rs;

    llvm::NamedMDNode *kernels =
               dep.linked_module->getNamedMetadata("opencl.kernels");

    if (!kernels) return rs;

    for (unsigned int i=0; i<kernels->getNumOperands(); ++i)
    {
        llvm::MDNode *node = kernels->getOperand(i);
        assert (node->getNumOperands() > 0);
        rs.push_back(node);
    }

    return rs;
}

std::vector<llvm::Function *> Program::kernelFunctions(const DeviceDependent &dep) const
{
    std::vector<llvm::Function *> rs;

    llvm::NamedMDNode *kernels =
               dep.linked_module->getNamedMetadata("opencl.kernels");

    if (!kernels) return rs;

    for (unsigned int i=0; i<kernels->getNumOperands(); ++i)
    {
        llvm::MDNode *node = kernels->getOperand(i);

        assert (node->getNumOperands() > 0);

        /*---------------------------------------------------------------------
        * Each node has multiple operands:
        * * Operand 0 is an llvm::Function
        * * The other operands are argument metadata nodes
        *--------------------------------------------------------------------*/
        llvm::Value *value = llvm::dyn_cast<llvm::ValueAsMetadata>(
                                               node->getOperand(0))->getValue();

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
        const std::vector<llvm::MDNode *> &kernel_md_nodes = KernelMDNodes(dep);

        /*---------------------------------------------------------------------
        * Find the one with the good name
        *--------------------------------------------------------------------*/
        for (size_t j=0; j < kernel_md_nodes.size(); ++j)
        {
            llvm::ValueAsMetadata *md_node_value =
                llvm::dyn_cast_or_null<llvm::ValueAsMetadata>(
                                        kernel_md_nodes[j]->getOperand(0));
            /*-----------------------------------------------------------------
             * Dont fail if cast fails, continue to next MDNode
             *----------------------------------------------------------------*/
            if (!md_node_value) continue;

            llvm::Value *value = md_node_value->getValue();

            /*-----------------------------------------------------------------
             * Dont fail if not an llvm::Function, continue to next MDNode
            *----------------------------------------------------------------*/
            if (!llvm::isa<llvm::Function>(value)) continue;

            llvm::Function *func = llvm::cast<llvm::Function>(value);

            if (!func) continue;

            if (func->getName().str() == name)
            {
                found = true;
                *errcode_ret = rs->addFunction(dep.device, kernel_md_nodes[j],
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
        Kernel *kernel = createKernel(kernels[i]->getName().str(), &result);

        if (result == CL_SUCCESS) rs.push_back(kernel);
        else                      delete kernel;
    }

    return rs;
}

void Program::PopulateKernelInfo()
{
     /* Populate num_kernels and kernel_names */
    const DeviceDependent &dep = p_device_dependent[0];
    const std::vector<llvm::Function *>& kernels = kernelFunctions(dep);
    unsigned int num_kernels = 0;
    std::string kernel_names;

    for (size_t i=0; i<kernels.size(); ++i)
    {
        llvm::Function *func = kernels[i];
        /* Only add kernel names if they are not already added */
        if (p_kernel_names.find(func->getName().str()) == std::string::npos)
        {
            kernel_names += func->getName().str();
            num_kernels += 1;
            if (i != kernels.size() - 1) kernel_names += ";";
        }
    }

    if (num_kernels > 0)
    {
        p_num_kernels += kernels.size();
        if (!p_kernel_names.empty()) p_kernel_names += ";";
        p_kernel_names += kernel_names;
    }
}



cl_int Program::loadSources(cl_uint count, const char **strings,
                            const size_t *lengths)
{
    // Initialize
    p_source  = std::string("");

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

    /*-------------------------------------------------------------------------
    * temporary for source file cacheing, remove from product releases
    *------------------------------------------------------------------------*/
    //source_cache::instance()->remember(p_source);

    p_type = Source;
    p_state = Loaded;

    return CL_SUCCESS;
}

cl_int Program::loadBinaries(const unsigned char **data, const size_t *lengths,
                             cl_int *binary_status, cl_uint num_devices,
                             const cl_device_id *device_list)
{
    // Set device infos
    setDevices(num_devices, device_list);
    p_state = Failed;

    // Load the data
    for (cl_uint i=0; i<num_devices; ++i)
    {
        DeviceDependent &dep = deviceDependent(pobj(device_list[i]));

        // If multiple devices share the same Root Device,
        // only load the first one for device dependent, skip the others
        // Loaded binary is stored in dep.unlinked_binary
        if (dep.unlinked_binary.size() != 0)  continue;

        std::string bitcode;

#ifndef _SYS_BIOS
        if (strncmp((const char*)data[i], ELFMAG, SELFMAG) == 0)
        {
#endif
            /* This is an ELF binary i.e. no BinaryHeader was added to it
             * i.e. this is a native binary or embedded header binary */
            dep.unlinked_binary = std::string((const char *)data[i],
                                              lengths[i]);
            dep.is_native_binary = true;

            if(!dep.program->ExtractMixedBinary(dep.unlinked_binary, bitcode))
            {
#ifndef _SYS_BIOS
                if (binary_status)
                    binary_status[i] = CL_INVALID_VALUE;
                return CL_INVALID_BINARY;
#else
                bitcode = dep.unlinked_binary;
                dep.is_native_binary = false;
#endif
            }

#ifndef _SYS_BIOS
            dep.is_library       = false;
        }
        else
        {
            /* This probably has a BinaryHeader i.e. it was created by
             * getProgramInfo */
            BinaryHeader header;
            size_t pos = 0;
            std::memcpy(&header, data[i], sizeof(BinaryHeader));
            pos += sizeof(BinaryHeader);
            p_type = header.program_type;

            switch(p_type)
            {
                case CompiledObject:
                    {
                        dep.unlinked_binary =
                            std::string((const char *)(data[i] + pos),
                                        header.binary_length);
                        pos += header.binary_length;
                        dep.unlinked_bc_binary =
                            std::string((const char *)(data[i] + pos),
                                        header.bc_binary_length);

                        if (!dep.program->
                            ExtractMixedBinary(dep.unlinked_bc_binary, bitcode))
                        {
                            if (binary_status)
                                binary_status[i] = CL_INVALID_VALUE;
                            return CL_INVALID_BINARY;
                        }

                        dep.is_native_binary = false;
                        dep.is_library       = false;

                        break;
                    }
                case Library:
                    {
                        dep.unlinked_binary =
                            std::string((const char *)(data[i] + pos),
                                        header.binary_length);
                        pos += header.binary_length;

                        bitcode = std::string((const char *)(data[i] + pos),
                                        header.bc_binary_length);

                        dep.is_native_binary = false;
                        dep.is_library       = true;

                        break;
                    }
                case Binary:
                    {
                        dep.unlinked_binary =
                            std::string((const char *)(data[i] + pos),
                                        header.binary_length);
                        pos += header.binary_length;

                        if (header.bc_binary_length != 0)
                        {
                            bitcode = std::string((const char *)(data[i] + pos),
                                                  header.bc_binary_length);
                        }else
                        {
                            if(!dep.program->
                               ExtractMixedBinary(dep.unlinked_binary, bitcode))
                            {
                                if (binary_status)
                                    binary_status[i] = CL_INVALID_VALUE;
                                return CL_INVALID_BINARY;
                            }
                        }

                        dep.is_native_binary = true;
                        dep.is_library       = false;

                        break;
                    }
                case Source:
                case Invalid:
                case BuiltIn:
                default:
                    {
                        if (binary_status) binary_status[i] = CL_INVALID_VALUE;
                        return CL_INVALID_BINARY;
                    }
            }
        }
#endif

        dep.linked_module = BitcodeToLLVMModule(bitcode, *p_llvmcontext);

        if (dep.linked_module == nullptr)
        {
            if (binary_status) binary_status[i] = CL_INVALID_VALUE;
            return CL_INVALID_BINARY;
        }

        if (binary_status) binary_status[i] = CL_SUCCESS;
    }

    p_state = Loaded;

    PopulateKernelInfo();

    return CL_SUCCESS;
}

#ifndef _SYS_BIOS
cl_int Program::compile(const char*                 options,
                        void (CL_CALLBACK*          pfn_notify)(cl_program program,
                                              void* user_data),
                        void*                       user_data,
                        const std::map<std::string, std::string>& input_header_src,
                        cl_uint                     num_devices,
                        const cl_device_id          *device_list)
{
    // If this program object was not created with source, return invalid
    if (p_type != Source) return CL_INVALID_PROGRAM;

    // Source: If we've already compiled this program and are re-compiling
    // (for example, with different user options) then clear out the
    // device dependent information in preparation for building again.
    if (p_state == Compiled || p_state == Built) resetDeviceDependent();

    p_state = InProgress;

    /* Create deviceDependent structures only for the root devices */
    if (!p_device_dependent.size())
    {
        setDevices(num_devices, device_list);
    }

    for (DeviceDependent& dep : p_device_dependent)
    {
        std::string opts(options ? options : "");
        std::string objfile;
        std::string bc_objfile;

        if (!dep.compiler->Compile(p_source, input_header_src,
                                   opts, objfile, bc_objfile))
        {
            p_state = Failed;
            return CL_BUILD_PROGRAM_FAILURE;
        }

        /* Remove object files if they have been successfully read into
         * strings */
        if(!objfile.empty() &&
           ReadBinaryIntoString(objfile, dep.unlinked_binary))
        {
            unlink(objfile.c_str());
        }
        else
        {
            std::cout << "ERROR: Unable to read object file: "
                      << objfile << std::endl;
            p_state = Failed;
            return CL_COMPILE_PROGRAM_FAILURE;
        }

        if(!bc_objfile.empty() &&
           ReadBinaryIntoString(bc_objfile, dep.unlinked_bc_binary))
        {
            unlink(bc_objfile.c_str());
        }
        else
        {
            std::cout << "ERROR: Unable to read object file: "
                      << bc_objfile << std::endl;
            p_state = Failed;
            return CL_COMPILE_PROGRAM_FAILURE;
        }

        dep.is_native_binary = false;

        // Extract LLVM bitcode from char array
        std::string bitcode;
        dep.program->ExtractMixedBinary(dep.unlinked_bc_binary,  bitcode);

        // Parse bitcode into a Module
        dep.linked_module = BitcodeToLLVMModule(bitcode, *p_llvmcontext);

        if (dep.linked_module == nullptr)
        {
            p_state = Failed;
            return CL_COMPILE_PROGRAM_FAILURE;
        }

        if (!dep.program->compile(dep.linked_module, dep.unlinked_binary, dep.unlinked_bc_binary))
        {
            if (pfn_notify)
                pfn_notify((cl_program)this, user_data);

            p_state = Failed;
            return CL_COMPILE_PROGRAM_FAILURE;
        }

    }

    if (pfn_notify)
        pfn_notify((cl_program)this, user_data);

    p_state = Compiled;
    p_type  = CompiledObject;

    PopulateKernelInfo();

    return CL_SUCCESS;

}

cl_int Program::link(const char*                  options,
                     void (CL_CALLBACK*           pfn_notify)(cl_program program,
                                           void*  user_data),
                     void*                        user_data,
                     const std::vector<Program*>& input_program_list,
                     cl_uint                      num_devices,
                     const cl_device_id           *device_list)
{
    assert(input_program_list.empty() == false);

    std::vector<std::string>    input_obj_files;
    std::vector<std::string>    input_libs;
    std::vector<llvm::Module*>  input_llvm_modules;
    std::string                 export_symbols;
    /* Check if each input program is a valid Compiled program or library
     * and extract the compiled object files or libraries from devicedependents
     * in the input programs to pass on for linking */
    for (Program* pr : input_program_list)
    {
        if (pr->type() != CompiledObject &&
            pr->type() != Library)
            return CL_INVALID_PROGRAM;

        /* The device list is guaranteed to have at least 1 device */
        const DeviceDependent& dep = pr->deviceDependent(pobj(device_list[0]));
        if (dep.linked_module != nullptr)
        {
            if (verifyModule(*dep.linked_module))
            {
                std::cout << ">> ERROR: input_program module is broken " << std::endl;
                return CL_LINK_PROGRAM_FAILURE;
            }
            else
            {
                input_llvm_modules.push_back(dep.linked_module);
            }
        }

        if (dep.is_library) input_libs.push_back(dep.unlinked_binary);
        else                input_obj_files.push_back(dep.unlinked_binary);

        if (!pr->KernelNames().empty())
        {
            if(!export_symbols.empty()) export_symbols += ";";
            export_symbols += pr->KernelNames();
        }
    }

    p_state = InProgress;

    /* Create deviceDependent structures only for the root devices */
    if (!p_device_dependent.size())
    {
        setDevices(num_devices, device_list);
    }

    for (DeviceDependent& dep : p_device_dependent)
    {
        std::string opts(options ? options : "");
        std::string outfile;

        if (!dep.compiler->Link(input_obj_files, input_libs, opts,
                                export_symbols, outfile))
        {
            p_state = Failed;
            return CL_BUILD_PROGRAM_FAILURE;
        }

        if (!ReadBinaryIntoString(outfile, dep.unlinked_binary))
        {
            p_state = Failed;
            return CL_LINK_PROGRAM_FAILURE;
        }

        /* If existing module is empty, create a new one in the
         * program's context */
        if (dep.linked_module == nullptr && !outfile.empty())
        {
            dep.linked_module = new llvm::Module(outfile.c_str(),
                                                 *p_llvmcontext);
            if (verifyModule(*dep.linked_module))
            {
                p_state = Failed;
                return CL_LINK_PROGRAM_FAILURE;
            }
        }

        /* Link input llvm modules into this dep's module */
        if(!LinkLLVMModules(dep.linked_module,
                            input_llvm_modules,
                            *p_llvmcontext))
        {
            p_state = Failed;
            return CL_LINK_PROGRAM_FAILURE;
        }

        /* Write out the bitcode in the unlinked_bc_binary for later use */
        llvm::raw_string_ostream str_ostream(dep.unlinked_bc_binary);
        llvm::WriteBitcodeToFile(dep.linked_module, str_ostream);
        str_ostream.flush();

        if (opts.find("-create-library") != std::string::npos)
        {
            dep.is_library = true;
            p_state = Archived;
            p_type  = Library;

            /* Remove the temporary .a file as it has already been read into
             * a string in memory */
            unlink(outfile.c_str());

        }
        else
        {
            dep.is_native_binary = true;
            dep.native_binary_filename = new char[outfile.size() + 1];
            strcpy(dep.native_binary_filename, outfile.c_str());

            // Now that the LLVM module is built, build the device-specific
            // representation
            if (!dep.program->build(dep.linked_module, &dep.unlinked_binary,
                                    dep.native_binary_filename))
            {
                if (pfn_notify)
                    pfn_notify((cl_program)this, user_data);
                p_state = Failed;
                return CL_LINK_PROGRAM_FAILURE;
            }
            p_state = Linked;
            p_type  = Binary;
        }

    }

    if (pfn_notify)
        pfn_notify((cl_program)this, user_data);

    PopulateKernelInfo();

    return CL_SUCCESS;
}

bool LinkLLVMModules(llvm::Module* linked_module,
                     const std::vector<llvm::Module*>& modules,
                     llvm::LLVMContext& llvmcontext)
{
    if (modules.empty()) return false;

    for(int i=0; i<modules.size(); i++)
    {
        /* Extract the module's bitcode into a string */
        std::string module_bc;
        llvm::raw_string_ostream ostream(module_bc);
        llvm::WriteBitcodeToFile(modules[i], ostream);
        ostream.str();

        /* Read the module back into current context using its bitcode string */
        llvm::Module *next_module = BitcodeToLLVMModule(module_bc, llvmcontext);

        /* Link with the base module */
        if(llvm::Linker::LinkModules(linked_module, next_module))
        {
            std::cout << ">> ERROR: link module failed" << std::endl;
            return false;
        }
    }

    if (verifyModule(*linked_module))
    {
        std::cout << ">> ERROR: linked_module is broken " << std::endl;
        return false;
    }

    return true;
}
#endif

cl_int Program::build(const char* options,
                      void (CL_CALLBACK* pfn_notify)(cl_program program,
                              void* user_data),
                      void* user_data, cl_uint num_devices,
                      const cl_device_id *device_list)
{
    if (p_type == Binary && p_state == Linked) return CL_SUCCESS;

    // Source: If we've already built this program and are re-building
    // (for example, with different user options) then clear out the
    // device dependent information in preparation for building again.
    if (p_type == Source && p_state == Built) resetDeviceDependent();

    // Binary: if we are building program from pre-built binaries,
    // check if device_list is valid
    if (p_type == Binary)
    {
        for (cl_uint i = 0; i < num_devices; i++)
        {
            if (std::find(p_device_list.begin(), p_device_list.end(),
                          pobj(device_list[i])) == p_device_list.end())
                return CL_INVALID_DEVICE;
        }
    }

    p_state = InProgress;

    /* Create deviceDependent structures only for the root devices */
    if (!p_device_dependent.size())
    {
        setDevices(num_devices, device_list);
    }

    for (DeviceDependent& dep : p_device_dependent)
    {
#ifndef _SYS_BIOS
        // Do we need to compile the source for each device ?
        if (p_type == Source)
        {
            std::string opts(options ? options : "");
            std::string outfile;

            if (!dep.compiler->CompileAndLink(p_source, opts, outfile))
            {
                p_state = Failed;
                return CL_BUILD_PROGRAM_FAILURE;
            }

            ReadBinaryIntoString(outfile, dep.unlinked_binary);

            dep.native_binary_filename = new char[outfile.size() + 1];
            strcpy(dep.native_binary_filename, outfile.c_str());

            dep.is_native_binary = true;

            // Extract LLVM bitcode from char array
            std::string bitcode;
            dep.program->ExtractMixedBinary(dep.unlinked_binary, bitcode);

            // Parse bitcode into a Module
            dep.linked_module = BitcodeToLLVMModule(bitcode, *p_llvmcontext);

            if (dep.linked_module == nullptr)
            {
                p_state = Failed;
                return CL_BUILD_PROGRAM_FAILURE;
            }
        }
#endif


        // Now that the LLVM module is built, build the device-specific
        // representation
        if (!dep.program->build(dep.linked_module, &dep.unlinked_binary,
                                dep.native_binary_filename))
        {
            if (pfn_notify)
                pfn_notify((cl_program)this, user_data);

            p_state = Failed;
            return CL_BUILD_PROGRAM_FAILURE;
        }
    }

    if (pfn_notify)
        pfn_notify((cl_program)this, user_data);

    p_state = Built;

    PopulateKernelInfo();

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
    llvm::SmallVector<cl_device_id, 4> devices;

    union {
        size_t size_t_var;
        cl_uint cl_uint_var;
        cl_context cl_context_var;
    };

    switch (param_name)
    {
        case CL_PROGRAM_REFERENCE_COUNT:
            SIMPLE_ASSIGN(cl_uint, references());
            break;

        case CL_PROGRAM_NUM_DEVICES:
            // Use devices associated with any built kernels, otherwise use
            // the devices associated with the program context
            if (p_device_list.size()!=0)
               { SIMPLE_ASSIGN(cl_uint, p_device_list.size()); }
            else
               return ((Context *)parent())->info(CL_CONTEXT_NUM_DEVICES,
                           param_value_size, param_value, param_value_size_ret);
            break;

        case CL_PROGRAM_DEVICES:
            // Use devices associated with any built kernels, otherwise use
            // the devices associated with the program context
            if (p_device_list.size()!=0)
            {
               for (size_t i=0; i<p_device_list.size(); ++i)
               {
                  devices.push_back(desc(p_device_list[i]));
               }

               value = devices.data();
               value_length = devices.size() * sizeof(cl_device_id);
           }
           else
              return ((Context *)parent())->info(CL_CONTEXT_DEVICES,
                           param_value_size, param_value, param_value_size_ret);
           break;

        case CL_PROGRAM_CONTEXT:
	        SIMPLE_ASSIGN(cl_context, desc((Context *)parent()));
            break;

        case CL_PROGRAM_SOURCE:
            MEM_ASSIGN(p_source.size() + 1, p_source.c_str());
            break;

        case CL_PROGRAM_BINARY_SIZES:
            for (size_t i=0; i<p_device_list.size(); ++i)
            {
                const DeviceDependent &dep = deviceDependent(p_device_list[i]);

                size_t binary_size  = 0;
#ifndef _SYS_BIOS
                if (p_state != Built)
                {
                    binary_size = sizeof(BinaryHeader) +
                                    dep.unlinked_binary.size() +
                                    dep.unlinked_bc_binary.size();
                }
                else
                {
#endif
                    binary_size = dep.unlinked_binary.size();
#ifndef _SYS_BIOS
                }
#endif
                binary_sizes.push_back(binary_size);
            }

            value = binary_sizes.data();
            value_length = binary_sizes.size() * sizeof(size_t);
            break;

        case CL_PROGRAM_BINARIES:
        {
            // Special case : param_value points to an array of p_device_list.size()
            // application-allocated unsigned char* pointers. Check it's good
            // and std::memcpy the data

            unsigned char **binaries = (unsigned char **)param_value;
            value_length = p_device_list.size() * sizeof(unsigned char *);

            if (param_value && param_value_size >= value_length)
                for (size_t i=0; i<p_device_list.size(); ++i)
                {
                    const DeviceDependent &dep = deviceDependent(p_device_list[i]);
                    unsigned char *dest = binaries[i];

                    if (!dest) continue;

                    size_t pos = 0;
#ifndef _SYS_BIOS
                    if (p_state != Built)
                    {
                        /* Copy binary header */
                        BinaryHeader header;
                        header.program_type     = p_type;
                        header.program_state    = p_state;
                        header.binary_length    = dep.unlinked_binary.size();
                        header.bc_binary_length = dep.unlinked_bc_binary.size();

                        std::memcpy(dest, &header, sizeof(BinaryHeader));
                        pos += sizeof(BinaryHeader);
                    }
#endif
                    /* Copy unlinked binary */
                    std::memcpy(dest + pos, dep.unlinked_binary.data(),
                                dep.unlinked_binary.size());
#ifndef _SYS_BIOS
                    if (p_state != Built)
                    {
                        /* Copy unlinked bc binary */
                        pos += dep.unlinked_binary.size();
                        if (!dep.unlinked_bc_binary.empty())
                            std::memcpy(dest + pos,
                                        dep.unlinked_bc_binary.data(),
                                        dep.unlinked_bc_binary.size());
                    }
#endif
                }

            if (param_value_size_ret)
                *param_value_size_ret = value_length;

            return CL_SUCCESS;
        }
        case CL_PROGRAM_NUM_KERNELS:
        {
            SIMPLE_ASSIGN(size_t, p_num_kernels);
            break;
        }
        case CL_PROGRAM_KERNEL_NAMES:
        {
            MEM_ASSIGN(p_kernel_names.size() + 1, p_kernel_names.c_str());
            break;
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
        cl_build_status        cl_build_status_var;
        cl_program_binary_type cl_program_binary_type_var;
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
                case Compiled:
                case Linked:
                case Built:
                case Archived:
                    SIMPLE_ASSIGN(cl_build_status, CL_BUILD_SUCCESS);
                    break;
                case Failed:
                    SIMPLE_ASSIGN(cl_build_status, CL_BUILD_ERROR);
                    break;
                case InProgress:
                    SIMPLE_ASSIGN(cl_build_status, CL_BUILD_IN_PROGRESS);
                    break;
            }
            break;

        case CL_PROGRAM_BUILD_OPTIONS:
#ifndef _SYS_BIOS
            value = dep.compiler->options().c_str();
            value_length = dep.compiler->options().size() + 1;
#endif
            break;

        case CL_PROGRAM_BUILD_LOG:
#ifndef _SYS_BIOS
            value = dep.compiler->log().c_str();
            value_length = dep.compiler->log().size() + 1;
#endif
            break;

        case CL_PROGRAM_BINARY_TYPE:
            switch (p_type)
            {
                case Invalid:
                case Source:
                case BuiltIn:
                    SIMPLE_ASSIGN(cl_program_binary_type,
                                  CL_PROGRAM_BINARY_TYPE_NONE);
                    break;
                case CompiledObject:
                    SIMPLE_ASSIGN(cl_program_binary_type,
                                  CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT);
                    break;
                case Library:
                    SIMPLE_ASSIGN(cl_program_binary_type,
                                  CL_PROGRAM_BINARY_TYPE_LIBRARY);
                    break;
                case Binary:
                    SIMPLE_ASSIGN(cl_program_binary_type,
                                  CL_PROGRAM_BINARY_TYPE_EXECUTABLE);
                    break;
            }
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

#ifndef _SYS_BIOS
static bool ReadBinaryIntoString(const std::string &outfile,
                                       std::string &binary_str)
{
    bool success = true;

    try
    {
        int   length;
        char *buffer = NULL;

        std::ifstream is;
        is.open(outfile.c_str(), std::ios::binary);
        is.seekg(0, std::ios::end);
        length = is.tellg();

        is.seekg(0, std::ios::beg);
        buffer = new char[length];

        is.read(buffer, length);
        is.close();

        binary_str.assign(buffer, length);

        delete [] buffer;
    }
    catch(const std::exception &e)
    { std::cerr << e.what() << std::endl;  success = false; }

    return success;
}
#endif


static llvm::Module *BitcodeToLLVMModule(const std::string &bitcode,
                                         llvm::LLVMContext &llvmcontext)
{
    const llvm::StringRef s_data(bitcode);
    const llvm::StringRef s_name("<binary>");

    std::unique_ptr<llvm::MemoryBuffer> buffer =
                     llvm::MemoryBuffer::getMemBuffer(s_data, s_name, false);

    if (!buffer)
        return nullptr;

    // Make a module of it
    llvm::ErrorOr<llvm::Module *> ModuleOrErr =
                                    parseBitcodeFile(buffer->getMemBufferRef(),
                                                     llvmcontext);

    if (std::error_code ec = ModuleOrErr.getError())
        return nullptr;

    llvm::Module *m = ModuleOrErr.get();

    return m;
}
