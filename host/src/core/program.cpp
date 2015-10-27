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
#include "util.h"

#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
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
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Linker.h>
#include <llvm/PassManager.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Function.h>
#ifndef _SYS_BIOS
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/IPO.h>
#endif
#include <llvm/IR/Instructions.h>
#include <llvm/Support/InstIterator.h>
#include <llvm/Transforms/Scalar.h>
#include <SimplifyShuffleBIFCall.h>
#ifndef _SYS_BIOS
#include <runtime/stdlib.c.bc.embed.h>
#endif
#include "dsp/genfile_cache.h"

/*-----------------------------------------------------------------------------
* temporary for source file cacheing, remove from product releases
*----------------------------------------------------------------------------*/
//#include "dsp/source_cache.h"
//source_cache * source_cache::pInstance = 0;

using namespace Coal;

Program::Program(Context *ctx)
: Object(Object::T_Program, ctx), p_type(Invalid), p_state(Empty)
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
        if (dep.native_binary_filename) delete [] dep.native_binary_filename;

        p_device_dependent.pop_back();
    }
}

void Program::setDevices(cl_uint num_devices, DeviceInterface * const*devices)
{
    p_device_dependent.resize(num_devices);

    for (cl_uint i=0; i<num_devices; ++i)
    {
        DeviceDependent &dep = p_device_dependent[i];

        dep.device                 = devices[i];
        dep.program                = dep.device->createDeviceProgram(this);
        dep.is_native_binary       = false;
        dep.native_binary_filename = NULL;
        dep.linked_module          = 0;
#ifndef _SYS_BIOS
        dep.compiler               = new Compiler(dep.device);
#endif
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
#ifndef _SYS_BIOS
    return dep.compiler->options();
#else
    return "error";
#endif

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

            if (func->getName().str() == name)

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

        Kernel *kernel = createKernel(kernels[i]->getName().str(), &result);


        if (result == CL_SUCCESS) rs.push_back(kernel);
        else                      delete kernel;
    }

    return rs;
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
                             DeviceInterface * const*device_list)
{
    // Set device infos
    setDevices(num_devices, device_list);

    // Load the data
    for (cl_uint i=0; i<num_devices; ++i)
    {
        DeviceDependent &dep = deviceDependent(device_list[i]);
        dep.unlinked_binary = std::string((const char *)data[i], lengths[i]);
        dep.is_native_binary = true;
#ifdef _SYS_BIOS

        dep.program->WriteNativeOut(&dep.unlinked_binary);
#endif

        /*--------------------------------------------------------------------
        * Loaded binary is either native code with LLVM bitcode embedded,
        *                  or     LLVM bitcode itself
        *--------------------------------------------------------------------*/

        std::string bitcode;
        if (! dep.program->ExtractMixedBinary(&dep.unlinked_binary, &bitcode, 
                                              NULL))
        {
            bitcode = dep.unlinked_binary;
            dep.is_native_binary = false;
        }
        const llvm::StringRef s_data(bitcode);
        const llvm::StringRef s_name("<binary>");

        llvm::MemoryBuffer *buffer = llvm::MemoryBuffer::getMemBuffer(
                                                        s_data, s_name, false);

        if (!buffer)
            return CL_OUT_OF_HOST_MEMORY;

        // Make a module of it
        dep.linked_module = ParseBitcodeFile(buffer, *p_llvmcontext);

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
    // If we've already built this program and are re-building
    // (for example, with different user options) then clear out the
    // device dependent information in preparation for building again.

    if( p_state == Built) resetDeviceDependent();

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
#ifndef _SYS_BIOS
        // Do we need to compile the source for each device ?
        if (p_type == Source)
        {
            bool do_clocl_compile = (getenv("TI_OCL_SRC_USE_ONLINE") == NULL);
            bool do_cache_kernels = (getenv("TI_OCL_CACHE_KERNELS")  != NULL);
            bool do_keep_files    = (getenv("TI_OCL_KEEP_FILES")     != NULL);
            bool do_debug         = (getenv("TI_OCL_DEBUG")          != NULL);

            cl_device_type devtype;
            device_list[i]->info(CL_DEVICE_TYPE, sizeof(devtype), &devtype, 0);
            if (devtype != CL_DEVICE_TYPE_ACCELERATOR) do_clocl_compile = false;

            if (do_debug) do_cache_kernels = false;

            if (do_clocl_compile)
            {
                char outfile[32];
                bool found_cached_file = false;

                #define STRINGIZE(x) #x
                #define STRINGIZE2(x) STRINGIZE(x)
                std::string product_version(STRINGIZE2(_PRODUCT_VERSION));
                std::string tmp_options(options ? options : "");
                tmp_options += product_version;

                if (do_cache_kernels)
                {
                    std::string cached_file = genfile_cache::instance()->lookup(
                                                         p_source, tmp_options);
                    if (! cached_file.empty())
                    {
                        strcpy(outfile, cached_file.c_str());
                        found_cached_file = true;
                    }
                }

                if (! found_cached_file)
                {   // Begin clocl compilation
                    char  name_out[] = "/tmp/openclXXXXXX";
                    char  srcfile[32];
                    char  logfile[32];
                    char *clocl_command = new char[(options?strlen(options):0)
                                                   + 64];
                    strcpy(clocl_command, "clocl -d -I. ");

                    // Pass compile options to clocl and compiler object 
		    // (so that they are available for build info requests)
		    // TO DO: Investigate pushing clocl call into compile()
		    if (options) 
                    { 
                       dep.compiler->set_options(options);
		       strcat(clocl_command, options); 
		       strcat(clocl_command, " "); 
		    }

                    if (do_keep_files) strcat(clocl_command, "-k ");
                    if (do_debug)      strcat(clocl_command, "-g ");

                    int  fOutfile = mkstemp(name_out);
                    strcpy(srcfile, name_out);
                    strcat(srcfile, ".cl");
                    strcpy(outfile, name_out);
                    strcat(outfile, ".out");
                    strcpy(logfile, name_out);
                    strcat(logfile, ".log");
                    strcat(clocl_command, srcfile);
                    strcat(clocl_command, " > ");
                    strcat(clocl_command, logfile);

                    /*--------------------------------------------------------
                    * put source in a tmp file, run clocl, read tmp output in
                    *--------------------------------------------------------*/
                    std::ofstream src_out(srcfile);
                    src_out << p_source;
                    src_out.close();
                    
                    int ret_code = system(clocl_command);

                    std::ifstream log_in(logfile);
                    std::string log_line;
                    if (log_in.is_open())
                    {
                        while (getline(log_in, log_line))
                        {
                            dep.compiler->appendLog(log_line);
                            dep.compiler->appendLog("\n");
                            std::cout << log_line << std::endl;
                        }
                        log_in.close();
                    }
                    delete [] clocl_command;
                    close(fOutfile);
                    if (! do_keep_files && !do_debug) unlink(srcfile);
		    unlink(logfile);
                    unlink(name_out);

		    // Check for system() call failure or clocl compile failure
		    if (ret_code == -1 || WEXITSTATUS(ret_code) == 0xFF) 
		       return CL_BUILD_PROGRAM_FAILURE;

                    if (do_cache_kernels)
                        genfile_cache::instance()->remember(outfile, p_source,
                                                            tmp_options);
                }   // End clocl compilation
                
                std::ifstream bin_in(outfile, std::ios::binary);
                bin_in.seekg (0, std::ios::end);
                int bin_len = bin_in.tellg();
                bin_in.seekg (0, std::ios::beg);
                char *bin_data = new char [bin_len];
                bin_in.read(bin_data, bin_len);
                bin_in.close();

                dep.native_binary_filename = new char[strlen(outfile)+1];
                strcpy(dep.native_binary_filename, outfile);
                
                dep.unlinked_binary = std::string((const char *)bin_data,
                                                                bin_len);
                dep.is_native_binary = true;
                if (bin_data != NULL)  delete [] bin_data;
                
                /*-------------------------------------------------------------
                * make module from native code with LLVM bitcode embedded
                *------------------------------------------------------------*/
                std::string bitcode;
                dep.program->ExtractMixedBinary(&dep.unlinked_binary, &bitcode,
                                                NULL);
                const llvm::StringRef s_data(bitcode);
                const llvm::StringRef s_name("<binary>");
                llvm::MemoryBuffer *buffer = llvm::MemoryBuffer::getMemBuffer(
                                                        s_data, s_name, false);
                if (!buffer) return CL_OUT_OF_HOST_MEMORY;
                
                // Make a module of it
                dep.linked_module = ParseBitcodeFile(buffer, *p_llvmcontext);
                if (!dep.linked_module)
                    return CL_BUILD_PROGRAM_FAILURE;
            }
            else
            {
                // Load source
                const llvm::StringRef s_data(p_source);
                const llvm::StringRef s_name("<source>");

                llvm::MemoryBuffer *buffer = llvm::MemoryBuffer::getMemBuffer(
                                                               s_data, s_name);

                // Compile
                if (! dep.compiler->compile(options ? options : std::string(),
                                            buffer, p_llvmcontext) )
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
        }

        // Link p_linked_module with the stdlib if the device needs that
        if (! dep.is_native_binary && dep.program->linkStdLib())
        {
            // Load the stdlib bitcode
            const llvm::StringRef s_data(embed_stdlib_c_bc,
                                         sizeof(embed_stdlib_c_bc) - 1);
            const llvm::StringRef s_name("stdlib.bc");
            std::string errMsg;

            llvm::MemoryBuffer *buffer = llvm::MemoryBuffer::getMemBuffer(
                                                        s_data, s_name, false);

            if (!buffer)
                return CL_OUT_OF_HOST_MEMORY;

            llvm::Module *stdlib = ParseBitcodeFile(buffer, *p_llvmcontext,
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

        if (! dep.is_native_binary)
        {
            // Get list of kernels to strip other unused functions
            std::vector<const char *> api;
            std::vector<std::string> api_s; // Needed to keep valid data in api
            const std::vector<llvm::Function *> &kernels = kernelFunctions(dep);
         
            for (size_t j=0; j<kernels.size(); ++j)
            {
                std::string s = kernels[j]->getName().str();
                api_s.push_back(s);
                api.push_back(s.c_str());
            }
         
            // determine if module has barrier() function calls
            bool hasBarrier = containsBarrierCall(*dep.linked_module);
         
            // Optimize code
            llvm::PassManager *manager = new llvm::PassManager();
         
            // Common passes (primary goal : remove unused stdlib functions)
            manager->add(llvm::createTypeBasedAliasAnalysisPass());
            manager->add(llvm::createBasicAliasAnalysisPass());
            manager->add(llvm::createInternalizePass(api));
            manager->add(llvm::createIPSCCPPass());
            manager->add(llvm::createGlobalOptimizerPass());
            manager->add(llvm::createConstantMergePass());
            manager->add(llvm::createAlwaysInlinerPass());
         
            dep.program->createOptimizationPasses(manager, 
                                       dep.compiler->optimize(), hasBarrier);
            manager->add(new tiocl::TIOpenCLSimplifyShuffleBIFCall());
         
            manager->add(llvm::createGlobalDCEPass());
            manager->add(llvm::createCFGSimplificationPass());
            manager->add(llvm::createLoopSimplifyPass());  // for llp6x
         
            manager->run(*dep.linked_module);
            delete manager;
        }

        // Now that the LLVM module is built, build the device-specific
        // representation
        if (!dep.program->build(dep.linked_module, &dep.unlinked_binary,
                                dep.native_binary_filename))
        {
            if (pfn_notify)
                pfn_notify((cl_program)this, user_data);

            return CL_BUILD_PROGRAM_FAILURE;
        }
#endif
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
	    // Use devices associated with any built kernels, otherwise use 
            // the devices associated with the program context
	    if (p_device_dependent.size() != 0)
	       { SIMPLE_ASSIGN(cl_uint, p_device_dependent.size()); }
	    else
	       return ((Context *)parent())->info(CL_CONTEXT_NUM_DEVICES, 
			   param_value_size, param_value, param_value_size_ret);
	    break;

        case CL_PROGRAM_DEVICES:
	    // Use devices associated with any built kernels, otherwise use 
            // the devices associated with the program context
	    if (p_device_dependent.size() != 0)
	    {
	       for (size_t i=0; i<p_device_dependent.size(); ++i)
	       {
		  const DeviceDependent &dep = p_device_dependent[i];
		 
		  devices.push_back(dep.device);
	       }

	       value = devices.data();
	       value_length = devices.size() * sizeof(DeviceInterface *);
	   }
	   else
	      return ((Context *)parent())->info(CL_CONTEXT_DEVICES,  
			   param_value_size, param_value, param_value_size_ret);
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

            if (param_value && param_value_size >= value_length)
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
