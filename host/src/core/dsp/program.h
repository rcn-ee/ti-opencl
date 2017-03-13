/******************************************************************************
 * Copyright (c) 2013-2016, Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef __DSP_PROGRAM_H__
#define __DSP_PROGRAM_H__

#include "device.h"
#include "../deviceinterface.h"
#include "dynamic_loader_interface.h"

namespace llvm
{
    class ExecutionEngine;
    class Module;
}

namespace Coal
{

class DSPDevice;
class Program;

class DSPProgram : public DeviceProgram
{
    public:
        DSPProgram(DSPDevice *device, Program *program);
        ~DSPProgram();

        // Disable default constructor, copy constuction and assignment
        DSPProgram()                             =delete;
        DSPProgram(const DSPProgram&)            =delete;
        DSPProgram& operator=(const DSPProgram&) =delete;

        bool linkStdLib() const;
        const char* outfile_name() const;
        void createOptimizationPasses(llvm::PassManager *manager,
                                      bool optimize, bool hasBarrier=false);
        bool build(llvm::Module *module, std::string *binary_str, 
                   char *binary_filename=NULL);
        bool ExtractMixedBinary(const std::string &binary_str,
                                      std::string &bitcode);
        void WriteNativeOut(const std::string &native);


        DSPDevicePtr mem_l2_section_extent(uint32_t& size) const;
        DSPDevicePtr query_symbol(const char *symname);
        DSPDevicePtr data_page_ptr();
        bool load();
        bool unload();
        bool is_loaded() const;
        DSPDevicePtr LoadAddress() const;

        DSPDevice *GetDevice() const { return p_device; }
        tiocl::DynamicLoader *GetDynamicLoader() const { return p_dl; }
        bool IsPrintInfoEnabled() const { return p_info; }

    private:
        DSPDevice    *p_device;
        Program      *p_program;
        llvm::Module *p_module;
        std::string   p_outfile;
        std::string  *p_nativebin;
        bool          p_loaded;
        bool          p_keep_files;
        bool          p_cache_kernels;
        bool          p_debug;
        bool          p_info;
        DSPDevicePtr  p_ocl_local_overlay_start;
        tiocl::DynamicLoader  *p_dl;
};
}
#endif
