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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file compiler.cpp
 * \brief Compiler wrapper
 */

#include "compiler.h"
#include "deviceinterface.h"

#include <cstring>
#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include "dsp/genfile_cache.h"

using namespace Coal;

Compiler::Compiler(DeviceInterface *device)
: p_device(device)
{
    do_cache_kernels = (getenv("TI_OCL_CACHE_KERNELS")  != NULL);
    do_keep_files    = (getenv("TI_OCL_KEEP_FILES")     != NULL);
    do_debug         = (getenv("TI_OCL_DEBUG")          != NULL);
    do_symbols       = (getenv("TI_OCL_DEVICE_PROGRAM_INFO") != NULL);
}

Compiler::~Compiler()
{}

bool Compiler::CompileAndLink(const std::string &source,
                              const std::string &options,
                                    std::string &outfile)
{
    p_options = options;

    cl_device_type devtype;
    p_device->info(CL_DEVICE_TYPE, sizeof(devtype), &devtype, 0);

    // No support for CPU (ARM) kernel compile - will be added later
    if (devtype == CL_DEVICE_TYPE_ACCELERATOR)
        return CompileAndLinkForDSP(source, options, outfile);
    else
        return false;
}

// Online compile for Acccelerator device (DSP) implemented via
// system call of clocl
bool Compiler::CompileAndLinkForDSP(const std::string &source,
                                    const std::string &options,
                                          std::string &outfile)
{

#define STRINGIZE(x) #x
#define STRINGIZE2(x) STRINGIZE(x)
    const std::string product_version(STRINGIZE2(_PRODUCT_VERSION));
    std::string options_with_version(options);
    options_with_version += product_version;

    // Check if the kernel has already been compiled and cached with the same
    // options and compiler version
    if (do_cache_kernels)
    {
        std::string cached_file = genfile_cache::instance()->lookup(
            source, options_with_version);
        if (! cached_file.empty())
        {
            outfile = cached_file;
            return true;
        }
    }

    // Begin clocl compilation
    std::string clocl_command("clocl -d -I. ");
    clocl_command.reserve(options.size() + 64);

    // Add in options
    clocl_command += options;
    clocl_command += " ";

    if (do_keep_files) clocl_command += "-k ";
    if (do_debug)      clocl_command += "-g ";
    if (do_symbols)    clocl_command += "-s ";

    char  name_out[] = "/tmp/openclXXXXXX";
    int  fOutfile = mkstemp(name_out);

    string srcfile(name_out);
    srcfile += ".cl";
    outfile = name_out;
    outfile += ".out";
    string logfile(name_out);
    logfile += ".log";

    clocl_command +=  srcfile;
    clocl_command +=  " > ";
    clocl_command += logfile;

    // put source in a tmp file, run clocl, read tmp output in
    std::ofstream src_out(srcfile.c_str());
    src_out << source;
    src_out.close();

    int ret_code = system(clocl_command.c_str());

    std::ifstream log_in(logfile.c_str());
    std::string log_line;
    if (log_in.is_open())
    {
        while (getline(log_in, log_line))
        {
            appendLog(log_line);
            appendLog("\n");
            std::cout << log_line << std::endl;
        }
        log_in.close();
    }
    close(fOutfile);
    if (! do_keep_files && !do_debug) unlink(srcfile.c_str());
    unlink(logfile.c_str());
    unlink(name_out);

    // Check for system() call failure or clocl compile failure
    if (ret_code == -1 || WEXITSTATUS(ret_code) == 0xFF)
        return false;

    if (do_cache_kernels)
        genfile_cache::instance()->remember(outfile.c_str(), source,
                                            options_with_version);

    return true;
}

const std::string &Compiler::log() const
{
    return p_log;
}

const std::string &Compiler::options() const
{
    return p_options;
}

void Compiler::set_options(const std::string &options)
{
   p_options = options;
}

void Compiler::appendLog(const std::string &log)
{
    p_log += log;
}
