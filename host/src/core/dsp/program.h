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
#ifndef __DSP_PROGRAM_H__
#define __DSP_PROGRAM_H__

#include "device.h"
#include "../deviceinterface.h"
#include <vector>

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
      struct seg_desc
      {
          seg_desc(DSPDevicePtr p, int s, uint32_t f) : 
                     ptr(p), size(s), flags(f) {}
          DSPDevicePtr ptr;
          unsigned     size;
          uint32_t     flags;
      };
  
      typedef std::vector<seg_desc> segment_list;

    public:
        DSPProgram(DSPDevice *device, Program *program);
        ~DSPProgram();

        bool linkStdLib() const;
        const char* outfile_name() const;
#ifndef _SYS_BIOS
        void createOptimizationPasses(llvm::PassManager *manager,
                                      bool optimize, bool hasBarrier=false);

        bool build(llvm::Module *module, std::string *binary_str,
                   char *binary_filename=NULL);
#endif
        bool ExtractMixedBinary(std::string *binary_str,
                                std::string *bitcode, std::string *native);

        void WriteNativeOut(std::string *native);
        void ReadEmbeddedBinary(std::string *binary_str);

        DSPDevicePtr query_symbol(const char *symname);
        DSPDevicePtr data_page_ptr();
        bool load();
        bool is_loaded() const;
        DSPDevicePtr program_load_addr() const;

        int l2_allocated() const;

    private:
        DSPDevice    *p_device;
        Program      *p_program;
        llvm::Module *p_module;
        int           p_program_handle;
        char          p_outfile[32];
#ifdef _SYS_BIOS
         std::string  *p_nativeout;
#endif
        bool          p_loaded;
        segment_list  p_segments_written;
        bool          p_keep_files;
        bool          p_cache_kernels;
        bool          p_debug;
};
}
#endif
