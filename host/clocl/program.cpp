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
#include <llvm/PassManager.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Utils/UnifyFunctionExitNodes.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include "wga.h"
#include "file_manip.h"
#include "options.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>


using namespace std;

/******************************************************************************
* Find the C6000 CGT installation
******************************************************************************/
#define DEFAULT_TI_CGT_INSTALL_PATH "/usr/share/ti/cgt-c6x"

const char *get_cgt_install()
{
    // First, check if TI_OCL_CGT_INSTALL is set (must be set on windows)
    const char *install = getenv("TI_OCL_CGT_INSTALL");
    if (install)
    {
        if (fs_exists(install))
            return install;
        else
        {
            std::cout << "\n The C6000 compiler installation specified by TI_OCL_CGT_INSTALL"
                         " does not exist: " << install << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        #if defined(_MSC_VER)
        std::cout << "\n TI_OCL_CGT_INSTALL must point to a C6000 compiler installation."
                  << std::endl;
        exit(EXIT_FAILURE);
        #endif
    }

    #if !defined(_MSC_VER)

    bool def_install = fs_exists(DEFAULT_TI_CGT_INSTALL_PATH);

    if (!def_install)
    {
       std::cout << "\nThe C6000 compiler lib/include installation is not "
       "located in the default\n"
       "location (/usr/share/ti/cgt-c6x).\n"
       "Use the environment variable TI_OCL_CGT_INSTALL to specify an alternate\n"
       "installation path.\n"  << std::endl;

       exit(EXIT_FAILURE);
    }
    else
       install = const_cast<char*>(DEFAULT_TI_CGT_INSTALL_PATH);

    #endif

    return install;
}

/******************************************************************************
* Find the OpenCL installation
******************************************************************************/
std::string get_ocl_dsp()
{
    string stdpath;

    // If TI_OCL_INSTALL is specified, use it as a prefix
    const char *ocl_install    = getenv("TI_OCL_INSTALL");
    const char *target_rootdir = getenv("TARGET_ROOTDIR");

    // If TI_OCL_INSTALL is specified, use it as a prefix
    if (ocl_install)         stdpath = ocl_install;
    else if (target_rootdir) stdpath = target_rootdir;

    #if !defined(_MSC_VER)
    stdpath += "/usr/share/ti/opencl";
    #else
    stdpath += "\\usr\\share\\ti\\opencl";
    #endif

    if (fs_exists(stdpath)) return stdpath;

    std::cout << "The OpenCL DSP directory " << stdpath
              << " does not exist !"         << std::endl;

    exit(EXIT_FAILURE);
}


/******************************************************************************
* run_cl6x for linking only
******************************************************************************/
bool run_cl6x_link(string& outfile, string& obj_files, string& lib_files)
{
    string command("cl6x -q ");
    const char* cgt_install = get_cgt_install();

    command += "-I"; command += cgt_install; command += "/lib ";
    command += "-I"; command += get_ocl_dsp().c_str(); command += " ";
    command += "-z ";
    command += obj_files;
    if (!lib_files.empty()) command += lib_files;
    command += "-o ";
    command += outfile;
    if (opt_expsyms)
    {
        command += " -l";
        command += file_expsyms;
    }
    command += " -ldsp.syms";

    if (opt_verbose) cout << command << endl;

    int x = system(command.c_str());
    if (x != 0) return false;

    if (!opt_debug && !opt_symbols)
    {
        string strip_command("strip6x -p ");
        strip_command += outfile;
        if (opt_verbose) cout << strip_command << endl;
        x = system(strip_command.c_str());
        if (x != 0) return false;
    }

    return true;
}

/******************************************************************************
* run_ar6x to create a library
******************************************************************************/
bool run_ar6x(string& liboutfile, string& obj_files)
{
    string command("ar6x aq ");
    command += liboutfile;
    command += " ";
    command += obj_files;

    if (opt_verbose) cout << command << endl;

    int ret_code = system(command.c_str());
    if (ret_code != 0) return false;

    return true;
}

/******************************************************************************
* run_cl6x
******************************************************************************/
int run_cl6x(string filename, string* llvm_bitcode, string addl_files)
{
    string command("cl6x --f -q --abi=eabi --use_g3 -mv6600 -mo ");

    if (!opt_alias) command += "-mt ";
    if (opt_tmpdir)
    {
        command += "-ft=";
        command += fs_get_tmp_folder() + " -fs=";
        command += fs_get_tmp_folder() + " -fr=";
        command += fs_get_tmp_folder() + " ";
    }
    if (opt_keep)   command += "-mw -k --z ";

    /*-------------------------------------------------------------------------
    * Worked around the sploop bug in the compiler.
    *------------------------------------------------------------------------*/
#if 0
    command += "--disable:sploop ";
#endif

    if (opt_debug)        command += "-o0 -g ";
    else if (opt_symbols) command += "-o3 ";
    else                  command += "-o3 --symdebug:none ";

    const char* cgt_install = get_cgt_install();

    command += "-I"; command += cgt_install; command += "/include ";
    command += "-I"; command += cgt_install; command += "/lib ";
    command += "-I"; command += get_ocl_dsp().c_str(); command += " ";

    command += "--bc_file="; command += filename; command += " ";

    /*-------------------------------------------------------------------------
    * Encode LLVM bitcode as bytes in the .llvmir section of the .asm file
    *------------------------------------------------------------------------*/
    if (llvm_bitcode != NULL)
    {
        string bitasm_name(fs_stem(filename));
        bitasm_name += "_bc.asm";
        if (opt_tmpdir) bitasm_name = fs_get_tmp_folder() + bitasm_name;

        ofstream outasmfile(bitasm_name.c_str(), ios::out);
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

    if (opt_lib)
    {
        if (opt_verbose) cout << command << endl;
        int x = system(command.c_str());
        // checking (return code != 0) works on both Linux and Windows
        if (x != 0) return false;
        return true;
    }

    string outfile(fs_replace_extension(filename, ".out"));

    command += "-z ";
    command += "-o ";
    command += outfile;
    command += " ";

    if (opt_keep)
    {
        command += "-m ";
        command += fs_replace_extension(filename, ".map");
        command += " ";
    }

    /*-------------------------------------------------------------------------
    * Any libraries or object files need to go last to resolve references
    *------------------------------------------------------------------------*/
    command += addl_files;
    command += " -ldsp.syms ";

    if (opt_verbose) cout << command << endl;
    int x = system(command.c_str());
    if (x != 0) return false;

    if (!opt_debug && !opt_symbols)
    {
        string strip_command("strip6x -p ");
        strip_command += outfile;
        if (opt_verbose) cout << strip_command << endl;
        x = system(strip_command.c_str());
        if (x != 0) return false;
    }

    return true;
}
