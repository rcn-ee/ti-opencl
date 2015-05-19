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

#include <llvm/PassManager.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Utils/UnifyFunctionExitNodes.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include "wga.h"

#include <llvm/LinkAllPasses.h>
#include <WorkitemHandlerChooser.h>
#include <BreakConstantGEPs.h>
#include <Flatten.h>
#include <PHIsToAllocas.h>
#include <IsolateRegions.h>
#include <VariableUniformityAnalysis.h>
#include <ImplicitLoopBarriers.h>
#include <ImplicitConditionalBarriers.h>
#include <LoopBarriers.h>
#include <BarrierTailReplication.h>
#include <CanonicalizeBarriers.h>
#include <WorkItemAliasAnalysis.h>
#include <WorkitemReplication.h>
#include <WorkitemLoops.h>
#include <AllocasToEntry.h>
#include <Workgroup.h>
#include <TargetAddressSpaces.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#include <elf.h>

#include "genfile_cache.h"

genfile_cache * genfile_cache::pInstance = 0;

timespec getTime()
{
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0)
        clock_gettime(CLOCK_REALTIME, &tp);
    return tp;
}

double ts_to_double(const timespec &t)
    { return ((double)t.tv_nsec) /1000000000.0 + (double)t.tv_sec; }

double tsdiff (const timespec& start, const timespec& end)
    { return ts_to_double(end) - ts_to_double(start); }


using namespace Coal;

DSPProgram::DSPProgram(DSPDevice *device, Program *program)
: DeviceProgram(), p_device(device), p_program(program), p_program_handle(-1),
  p_loaded(false), p_keep_files(false), p_cache_kernels(false)
{
    char *keep = getenv("TI_OCL_KEEP_FILES");
    if (keep) p_keep_files = true;

    char *cache = getenv("TI_OCL_CACHE_KERNELS");
    if (cache) p_cache_kernels = true;

    char *debug = getenv("TI_OCL_DEBUG");
    if (debug) { p_debug = true; p_cache_kernels = false; }

    char *info = getenv("TI_OCL_DEVICE_PROGRAM_INFO");
    if (info) { p_info = true; p_keep_files = true; }
}

DSPProgram::~DSPProgram()
{
    p_device->unload(p_program_handle);
    if (!p_keep_files && !p_cache_kernels) unlink(p_outfile);
}

DSPProgram::segment_list *segments;

bool DSPProgram::load()
{
    segments = &p_segments_written;

    p_program_handle = p_device->load(p_outfile);
    if (!p_program_handle) return false;

    segments = NULL;
    p_loaded = true;

    /*-------------------------------------------------------------------------
    * ensure that the newly populated areas are not stale in device caches
    *------------------------------------------------------------------------*/
    if (p_info)
    {
        printf("For executable: %s\n", p_outfile);
        int   segNum = p_segments_written.size();
        for (int i=0; i < segNum; ++i)
        {
            uint32_t flags = p_segments_written[i].flags & 
                       (DLOAD_SF_executable | DLOAD_SF_writable);

            const char *seg_desc;
            switch (flags)
            {
                case 0:                   seg_desc = "Read Only"; break;
                case DLOAD_SF_executable: seg_desc = "Executable"; break;
                case DLOAD_SF_writable:   seg_desc = "Writable"; break;
                default:                  seg_desc = "Writable & Executable"; break;
            }

               printf("\t%s segment loaded to 0x%08x with size 0x%x\n", 
                   seg_desc, p_segments_written[i].ptr, p_segments_written[i].size);
        }
        printf("\n");
    }

    /*-------------------------------------------------------------------------
    * Send the cache Inv command.  We do not wait here.  The wait will be 
    * handled by the standard wait loop in the worker thread.
    *------------------------------------------------------------------------*/
    p_device->mail_to(cacheMsg);
    return true;
}

DSPDevicePtr DSPProgram::program_load_addr() const
{
    DSPDevicePtr load_addr = 0;

    for (int i = 0; i < p_segments_written.size(); ++i)
        if (p_segments_written[i].flags & DLOAD_SF_executable)
            if (load_addr == 0) load_addr = p_segments_written[i].ptr;
            else return 0;  // if multiple, punt

    return load_addr;
}

/******************************************************************************
* L2_ALLOCATED
******************************************************************************/
int DSPProgram::l2_allocated() const
{
    /*-------------------------------------------------------------------------
    * Program must be loaded to determine how much l2 space is required
    *------------------------------------------------------------------------*/
    if (!is_loaded()) const_cast<DSPProgram*>(this)->load();

    int allocated = 0;

    for (int i = 0; i < p_segments_written.size(); ++i)
        if (p_segments_written[i].ptr >= 0x00800000 && 
            p_segments_written[i].ptr <  0x00900000)
             allocated += p_segments_written[i].size;

    return allocated;
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
    return p_outfile;
}

DSPDevicePtr DSPProgram::data_page_ptr() 
{
    DSPDevicePtr p;

    if (!is_loaded()) load();

    DLOAD_get_static_base(p_device->dload_handle(), p_program_handle,  &p);
    return p;
}

void DSPProgram::createOptimizationPasses(llvm::PassManager *manager,
                                          bool optimize, bool hasBarrier)
{
    if (hasBarrier)
    {
        manager->add(    llvm::createPromoteMemoryToRegisterPass());
        manager->add(new llvm::DominatorTree());
        manager->add(new pocl::WorkitemHandlerChooser());
        manager->add(new       BreakConstantGEPs());   // from pocl
        //       add(new       GenerateHeader());      // no need
        manager->add(new pocl::Flatten());
        manager->add(    llvm::createAlwaysInlinerPass());
        manager->add(    llvm::createGlobalDCEPass());
        manager->add(    llvm::createCFGSimplificationPass());
        manager->add(    llvm::createLoopSimplifyPass());
        manager->add(new pocl::PHIsToAllocas());
        manager->add(    llvm::createRegionInfoPass());
        manager->add(new pocl::IsolateRegions());
        manager->add(new pocl::VariableUniformityAnalysis()); // TODO
        manager->add(new pocl::ImplicitLoopBarriers());
        // manager->add(new pocl::ImplicitConditionalBarriers()); // pocl 0.9: clang -O2 crashes shoc spmv, disable for now
        manager->add(new pocl::LoopBarriers());
        manager->add(new pocl::BarrierTailReplication());
        manager->add(new pocl::CanonicalizeBarriers());
        manager->add(new pocl::IsolateRegions());
        manager->add(new pocl::WorkItemAliasAnalysis());
        //       add(new pocl::WorkitemReplication()); // no need
        manager->add(new pocl::WorkitemLoops());
        manager->add(new pocl::AllocasToEntry());
        //       add(new pocl::Workgroup());           // no need
        manager->add(new pocl::TargetAddressSpaces());
    }

    if (optimize)
    {
        /*
         * Inspired by code from "The LLVM Compiler Infrastructure"
         */
        manager->add(llvm::createDeadArgEliminationPass());
        manager->add(llvm::createInstructionCombiningPass());
        manager->add(llvm::createFunctionInliningPass());
        manager->add(llvm::createPruneEHPass());   // Remove dead EH info.
        manager->add(llvm::createGlobalOptimizerPass());
        manager->add(llvm::createGlobalDCEPass()); // Remove dead functions.
        manager->add(llvm::createArgumentPromotionPass());
        manager->add(llvm::createInstructionCombiningPass());
        manager->add(llvm::createJumpThreadingPass());

        //ASW TODO maybe turn off re: pete.  might gen bad xlator input 
        //manager->add(llvm::createScalarReplAggregatesPass());

        manager->add(llvm::createFunctionAttrsPass()); // Add nocapture.
        manager->add(llvm::createGlobalsModRefPass()); // IP alias analysis.
        manager->add(llvm::createLICMPass());      // Hoist loop invariants.
        manager->add(llvm::createGVNPass());       // Remove redundancies.
        manager->add(llvm::createMemCpyOptPass()); // Remove dead memcpys.
        manager->add(llvm::createDeadStoreEliminationPass());
        manager->add(llvm::createInstructionCombiningPass());
        manager->add(llvm::createJumpThreadingPass());
        manager->add(llvm::createCFGSimplificationPass());
    }

    manager->add(llvm::createUnifyFunctionExitNodesPass());
    manager->add(llvm::createTIOpenclWorkGroupAggregationPass(hasBarrier));

    /*-------------------------------------------------------------------------
    * Borrow the pocl alloca hoister for the TI simplistic WGA pass as well
    *------------------------------------------------------------------------*/
    if (!hasBarrier)
        manager->add(new pocl::AllocasToEntry());
}


std::string process_cl6x_options(std::string options)
{
    std::istringstream options_stream(options);
    std::string token;
    std::string result;

    while (options_stream >> token)
    {
        if ((token.find(".obj")  != std::string::npos) ||
            (token.find(".dll")  != std::string::npos) ||
            (token.find(".ae66") != std::string::npos) ||
            (token.find(".a66")  != std::string::npos) ||
            (token.find(".out")  != std::string::npos) ||
            (token.find(".lib")  != std::string::npos) ||
            (token.find(".o")    != std::string::npos) ||
            (token.find(".o66")  != std::string::npos) ||
            (token.find(".oe66") != std::string::npos) ||
            (token.find(".a")    != std::string::npos) ||
            (token.find(".cmd")  != std::string::npos))  
                result += token + " ";
    }
    return result;
}

/******************************************************************************
* Find the C6000 CGT installation
******************************************************************************/
#define DEFAULT_TI_CGT_INSTALL_PATH "/usr/share/ti/cgt-c6x"

const char *get_cgt_install()
{
    const char *install = getenv("TI_OCL_CGT_INSTALL");
    if (install) return install;

    bool def_install = access(DEFAULT_TI_CGT_INSTALL_PATH, F_OK|R_OK) != -1;

    if (!def_install)
    {
       std::cout << "\nThe C6000 compiler lib/include installation is not "
       "located in the default\n"
       "location (/usr/share/ti/cgt-c6x).\n"
       "Use the environment variable TI_OCL_CGT_INSTALL to specify an alternate\n"
       "installation path.\n"  << std::endl;
      
       abort();
    }
    else return DEFAULT_TI_CGT_INSTALL_PATH;
}


/******************************************************************************
* Find the OpenCL installation
******************************************************************************/
std::string get_ocl_dsp()
{
    std::string stdpath("/usr/share/ti/opencl");

    const char *ocl_install = getenv("TARGET_ROOTDIR");
    if (ocl_install) stdpath = ocl_install + stdpath;

    struct stat st;
    stat(stdpath.c_str(), &st);
    if (S_ISDIR(st.st_mode)) return stdpath.c_str();

    std::cout << "The OpenCL DSP directory " << stdpath
              << " does not exist !"         << std::endl;
    abort();
}


/******************************************************************************
* run_cl6x
******************************************************************************/
static int run_cl6x(char *filename, std::string *llvm_bitcode, 
                    bool keep_files, bool debug, bool info, std::string options)
{
    std::string command("cl6x --f -q --abi=eabi --use_g3 -mv6600 -mt -mo "
                        "-ft=/tmp -fs=/tmp -fr=/tmp ");

    if (keep_files) command += "-mw -k --z ";

    /*-------------------------------------------------------------------------
    * Turned off for now to workaround a timing bug. Plan to re-enable later
    *------------------------------------------------------------------------*/
    command += "--disable:sploop ";

    if (debug) command += "-o0 -g ";
    else       command += "-o3 --symdebug:none ";

    char *no_sp = getenv("TI_OCL_SOFTWARE_PIPELINE_OFF");
    if (no_sp) command += "-mu ";

    const char *cgt_install = get_cgt_install();

    command += "-I"; command += cgt_install; command += "/include ";
    command += "-I"; command += cgt_install; command += "/lib ";
    command += "-I"; command += get_ocl_dsp().c_str(); command += " ";

    command += "--bc_file="; command += filename; command += " "; 

    /*-------------------------------------------------------------------------
    * Encode LLVM bitcode as bytes in the .llvmir section of the .asm file
    *------------------------------------------------------------------------*/
    if (llvm_bitcode != NULL)
    {
        char bitasm_name[32];
        strcpy(bitasm_name, filename);
        strcat(bitasm_name, "_bc.asm");
        std::ofstream outasmfile(bitasm_name, std::ios::out);
        outasmfile << "\t.sect \".llvmir\"\n" << "\t.retain";
        int nbytes = llvm_bitcode->size();
        for (int i = 0; i < nbytes; i++)
            if (i % 10 == 0)
               outasmfile << "\n\t.byte " << (int) llvm_bitcode->at(i);
            else
               outasmfile << ", " << (int) llvm_bitcode->at(i);
        outasmfile.close();

        command += bitasm_name; command += " ";
    }

    command += "-z -ldsp.syms -o "; 
    command += filename; command += ".out ";

    if (keep_files) 
        { command += "-m "; command += filename; command += ".map "; }

    /*-------------------------------------------------------------------------
    * Any libraries or object files need to go last to resolve references
    *------------------------------------------------------------------------*/
    command += process_cl6x_options(options); 

        //timespec t0, t1;
        //clock_gettime(CLOCK_MONOTONIC, &t0);
    int x = system(command.c_str());
        //clock_gettime(CLOCK_MONOTONIC, &t1);
        //printf("cl6x time: %6.4f secs\n", 
        //   (float)t1.tv_sec-t0.tv_sec+(t1.tv_nsec-t0.tv_nsec)/1e9);

    if (!debug && !info)
    {
        std::string strip_command("strip6x -p ");
        strip_command += filename; strip_command += ".out";
        x = system(strip_command.c_str());
    }
}

/**
 * Extract llvm bitcode and native binary from MixedBinary
 */
bool DSPProgram::ExtractMixedBinary(std::string *binary_str,
                             std::string *bitcode, std::string *native)
{
    if (binary_str == NULL)  return false;
    if (strncmp(&binary_str->at(0), ELFMAG, SELFMAG) != 0)  return false;

    /*-------------------------------------------------------------------------
    * Parse ELF file format, extract ".llvmir" section into bitcode
    * Valid Assumptions: 1. cl6x only creates 32-bit ELF files (for now)
    *                    2. cl6x ELF file has the same endianness as the host
    *------------------------------------------------------------------------*/
    if (bitcode != NULL)
    {
        Elf32_Ehdr ehdr;  /* memcpy into here to guarantee proper alignment */
        memcpy(&ehdr, & binary_str->at(0), sizeof(Elf32_Ehdr));
        int        n_sects    = ehdr.e_shnum;
        int        shoff      = ehdr.e_shoff;
        int        shstr_sect = ehdr.e_shstrndx;

        Elf32_Shdr shdr;  /* memcpy into here to guarantee proper alignment */
        int        shsize     = sizeof(Elf32_Shdr);
        memcpy(&shdr, & binary_str->at(shoff + shstr_sect * shsize), shsize);
        char      *strtab = & binary_str->at(shdr.sh_offset);

        int        i;
        for (i = 0; i < n_sects; i++)
        {
            if (i == shstr_sect)  continue;
            memcpy(&shdr, & binary_str->at(shoff + i * shsize), shsize);
            if (strcmp(&strtab[shdr.sh_name], ".llvmir") == 0)  break;
        }
        if (i >= n_sects)  return false;

        bitcode->clear();
        bitcode->append(& binary_str->at(shdr.sh_offset), shdr.sh_size);
    }

    /*-------------------------------------------------------------------------
    * Return the c6x ELF file in binary_str as native binary
    *------------------------------------------------------------------------*/
    if (native != NULL)
    {
        native->clear();
        native->append(*binary_str);
    }

    return true;
}


/**
 * Write native binary into file, create tmporary filename in p_outfile
 */
void DSPProgram::WriteNativeOut(std::string *native)
{
    try
    {
        char name_out[] = "/tmp/openclXXXXXX";
        int  fOutfile = mkstemp(name_out);
        strcpy(p_outfile, name_out);
        strcat(p_outfile, ".out");

        std::ofstream outfile(p_outfile, std::ios::out | std::ios::binary);
        outfile.write(native->data(), native->size());
        outfile.close();
        close(fOutfile);
        unlink(name_out);
    }
    catch(...) { std::cout << "ERROR: Binary write out failure" << std::endl; }
}

/**
 * Native binary is stored in file, filename in p_outfile
 * Input:  binary_str contains only the bitcode
 * Output: binary_str contains c6x ELF file with bitcode in ".llvmir" section
 */
void DSPProgram::ReadEmbeddedBinary(std::string *binary_str)
{
    if (binary_str == NULL)  return;

    int   length;
    char *buffer = NULL;

    try
    {
        std::ifstream is;
        is.open(p_outfile, std::ios::binary);
        is.seekg(0, std::ios::end);
        length = is.tellg();
        is.seekg(0, std::ios::beg);
        buffer = new char[length];
        is.read(buffer, length);
        is.close();
 
        binary_str->clear();
        binary_str->append(buffer, length);
        delete [] buffer;
    }
    catch(...) { std::cout << "ERROR: Binary read in failure" << std::endl; }
}

bool DSPProgram::build(llvm::Module *module, std::string *binary_str,
                       char *binary_filename)
{
    p_module = module;

    /*------------------------------------------------------------------------
    * If binary_filename is not NULL, then no need to rebuild
    *------------------------------------------------------------------------*/
    if (binary_filename != NULL)
    {
        strcpy(p_outfile, binary_filename);
        return true;
    }

    /*------------------------------------------------------------------------
    * The input binary_str could be any of the following:
    * 1. Mixed C6x binary embedded with LLVM bitcode, extract C6x native 
    *    binary and return.  There is no need to rebuild from LLVM module.
    * 2. LLVM bitcode, proceed to the regular build: 
    *    2.1 return a corresponding cached c6x binary, if found
    *    2.2 invoke c6x compiler toolchain, embed LLVM bitcode, build
    *    In either case, put c6x binary in binary_str when return
    *------------------------------------------------------------------------*/
    std::string native;
    if (ExtractMixedBinary(binary_str, NULL, &native))
    {
        WriteNativeOut(&native);
        return true;
    }

    if (p_cache_kernels)
    {
        string cached_outfile = genfile_cache::instance()->lookup
            (p_module, p_program->deviceDependentCompilerOptions(p_device));

        if (!cached_outfile.empty())
        { 
            strcpy(p_outfile, cached_outfile.c_str()); 
            ReadEmbeddedBinary(binary_str);
            return true; 
        }
    }

    char name_template[] =  "/tmp/openclXXXXXX";
    int pFile = mkstemp(name_template);

    strcpy(p_outfile, name_template);
    strcat(p_outfile, ".out");

    if (pFile != -1)
    {
        if (p_keep_files)
        {
            //write out the source as well

            std::string filename(name_template);
            filename += ".cl";
            std::ofstream out(filename.c_str());
            out << p_program->source();
            out.close();
        }

        llvm::raw_fd_ostream ostream(pFile, false);
        llvm::WriteBitcodeToFile(p_module, ostream);
        ostream.flush();

        string xformed_binary_str;
        llvm::raw_string_ostream str_ostream(xformed_binary_str);
        llvm::WriteBitcodeToFile(p_module, str_ostream);
        str_ostream.flush();
        run_cl6x(name_template, &xformed_binary_str, p_keep_files, p_debug, p_info,
                   p_program->deviceDependentCompilerOptions(p_device));

        char objfile[32]; 
        if (!p_keep_files)
        {
            unlink(name_template);

            strcpy(objfile, name_template); 
            strcat(objfile, ".obj");
            unlink(objfile);
        }
        else
        {
            strcpy(objfile, name_template); 
            strcat(objfile, ".objc");
            unlink(objfile);

            strcpy(objfile, name_template); 
            strcat(objfile, "_bc.objc");
            unlink(objfile);
        }

        if (binary_str != NULL)
        {
            strcpy(objfile, name_template); 
            strcat(objfile, "_bc.asm");
            unlink(objfile);

            strcpy(objfile, name_template); 
            strcat(objfile, "_bc.obj");
            unlink(objfile);
        }

        if (p_cache_kernels)
            genfile_cache::instance()->remember(p_outfile, p_module, 
                    p_program->deviceDependentCompilerOptions(p_device));

        ReadEmbeddedBinary(binary_str);
    }

    if (pFile != -1) close(pFile);

    return true;
}

DSPDevicePtr DSPProgram::query_symbol(const char *symname)
{
    DSPDevicePtr addr;

    bool found = DLOAD_query_symbol(p_device->dload_handle(), p_program_handle,
                                    symname, &addr);

    return (found) ? addr : 0;
}

extern "C" {

Coal::DSPDevice *getDspDevice();

uint64_t __query_symbol(cl_program program, const char *sym_name)
{
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
