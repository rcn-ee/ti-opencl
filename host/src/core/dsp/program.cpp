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
#include "program.h"
#include "device.h"
#include "kernel.h"

#include "../program.h"

#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <elf.h>

#include "genfile_cache.h"

#include "tal/dload_impl.h"


using tiocl::DLOAD;

genfile_cache * genfile_cache::pInstance = 0;


using namespace Coal;

DSPProgram::DSPProgram(DSPDevice *device, Program *program)
: DeviceProgram(), p_device(device), p_program(program),
  p_loaded(false), p_keep_files(false), p_cache_kernels(false), p_debug(false),
  p_info(false), p_ocl_local_overlay_start(0), p_dl(nullptr)
{
    char *keep = getenv("TI_OCL_KEEP_FILES");
    if (keep) p_keep_files = true;

    char *cache = getenv("TI_OCL_CACHE_KERNELS");
    if (cache) p_cache_kernels = true;

    char *debug = getenv("TI_OCL_DEBUG");
    if (debug) { p_debug = true; p_cache_kernels = false; }

    char *info = getenv("TI_OCL_DEVICE_PROGRAM_INFO");
    if (info) { p_info = true; p_keep_files = true; }

    p_dl = new DLOAD(this);
}

DSPProgram::~DSPProgram()
{
    unload();
    delete p_dl;
    p_dl = nullptr;

    if (!p_keep_files && !p_cache_kernels) unlink(p_outfile.c_str());
}

bool DSPProgram::load()
{
    if (!p_dl->LoadProgram(p_outfile))
        return false;

    p_loaded = true;

    p_ocl_local_overlay_start = query_symbol("_ocl_local_overlay_start");

    /*-------------------------------------------------------------------------
    * Ensure that the newly populated areas are not stale in device caches
    * Send the cache Inv command.  We do not wait here.  The wait will be
    * handled by the standard wait loop in the worker thread.
    *------------------------------------------------------------------------*/
    p_device->mail_to(cacheMsg);
    return true;
}

bool DSPProgram::unload()
{
    return p_dl->UnloadProgram();
}

DSPDevicePtr DSPProgram::mem_l2_section_extent(uint32_t& size) const
{
    /*-------------------------------------------------------------------------
    * Program must be loaded to determine how much l2 space is required
    *------------------------------------------------------------------------*/
    if (!is_loaded()) const_cast<DSPProgram*>(this)->load();

    uint32_t     L2size;
    DSPDevicePtr L2start = p_device->get_L2_extent(L2size);

    if (!p_ocl_local_overlay_start) size = 0;
    else size = p_ocl_local_overlay_start - L2start;

    return L2start;
}

DSPDevicePtr DSPProgram::LoadAddress() const
{
    return p_dl->GetProgramLoadAddress();
}

bool DSPProgram::is_loaded() const
{
    return p_loaded;
}

bool DSPProgram::linkStdLib() const
{
    return false;
}

const char* DSPProgram::outfile_name() const
{
    return p_outfile.c_str();
}

DSPDevicePtr DSPProgram::data_page_ptr()
{
    return p_dl->GetDataPagePointer();
}

void DSPProgram::createOptimizationPasses(llvm::PassManager *manager,
                                          bool optimize, bool hasBarrier)
{
}

/**
 * Extract llvm bitcode and native binary from MixedBinary
 */
bool DSPProgram::ExtractMixedBinary(const std::string &binary_str,
                                          std::string &bitcode)
{
    if (binary_str.empty())  return false;
    if (strncmp(&binary_str.at(0), ELFMAG, SELFMAG) != 0)  return false;

    /*-------------------------------------------------------------------------
    * Parse ELF file format, extract ".llvmir" section into bitcode
    * Valid Assumptions: 1. cl6x only creates 32-bit ELF files (for now)
    *                    2. cl6x ELF file has the same endianness as the host
    *------------------------------------------------------------------------*/
    Elf32_Ehdr ehdr;  /* memcpy into here to guarantee proper alignment */
    memcpy(&ehdr, & binary_str.at(0), sizeof(Elf32_Ehdr));
    int        n_sects    = ehdr.e_shnum;
    int        shoff      = ehdr.e_shoff;
    int        shstr_sect = ehdr.e_shstrndx;

    Elf32_Shdr shdr;  /* memcpy into here to guarantee proper alignment */
    int        shsize     = sizeof(Elf32_Shdr);
    memcpy(&shdr, & binary_str.at(shoff + shstr_sect * shsize), shsize);
    const char      *strtab = & binary_str.at(shdr.sh_offset);

    int i;
    for (i = 0; i < n_sects; i++)
    {
        if (i == shstr_sect)  continue;
        memcpy(&shdr, & binary_str.at(shoff + i * shsize), shsize);
        if (strcmp(&strtab[shdr.sh_name], ".llvmir") == 0)  break;
    }

    if (i >= n_sects)
        return false;

    bitcode.clear();
    bitcode.append(& binary_str.at(shdr.sh_offset), shdr.sh_size);

    return true;
}


/**
 * Write native binary into file, create tmporary filename in p_outfile
 */
void DSPProgram::WriteNativeOut(const std::string &native)
{
    assert (native.empty() == false);

    try
    {
        char name_out[] = "/tmp/openclXXXXXX";
        int  fOutfile = mkstemp(name_out);
        p_outfile = name_out;
        p_outfile += ".out";

        std::ofstream outfile(p_outfile.c_str(), std::ios::out | std::ios::binary);
        outfile.write(native.data(), native.size());
        outfile.close();
        close(fOutfile);
        unlink(name_out);
    }
    catch(...) { std::cout << "ERROR: Binary write out failure" << std::endl; }
}


bool DSPProgram::build(llvm::Module *module, std::string *binary_str,
                       char *binary_filename)
{
    p_module = module;

    // Binary file has already been created by Compile::CompileAndLink
    if (binary_filename != NULL)
    {
        p_outfile = binary_filename;
        return true;
    }

    // The OpenCL runtime was provided a binary file.
    else if (binary_str != NULL)
    {
        WriteNativeOut(*binary_str);
        return true;
    }

    return false;
}

DSPDevicePtr DSPProgram::query_symbol(const char *symname)
{
    const std::string str(symname);

    return p_dl->QuerySymbol(str);
}

extern "C" {

Coal::DSPDevice *getDspDevice();

uint64_t __query_symbol(cl_program d_program, const char *sym_name)
{
    auto program = pobj(d_program);

    if (!program->isA(Coal::Object::T_Program))
        return 0;

    DSPDevice  *device = getDspDevice();
    if (device == NULL)
        return 0;

    DSPProgram *prog = (DSPProgram *)(program->deviceDependentProgram(device));

    if (!prog->is_loaded()) prog->load();

    if (!prog->is_loaded())
        return 0;

    return prog->query_symbol(sym_name);
}

} // extern "C"
