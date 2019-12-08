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

#ifndef _SYS_BIOS
bool Compiler::Compile(const string &source,
                       const map<string, string> &input_headers,
                       const string &options,
                             string &objfile, string &bc_objfile)
{
    p_options = options;

    cl_device_type devtype;
    p_device->info(CL_DEVICE_TYPE, sizeof(devtype), &devtype, 0);

    if (devtype & CL_DEVICE_TYPE_ACCELERATOR)
    {
        return CompileForDSP(source, input_headers, options, objfile, bc_objfile);
    }
    else
    {
        return false;
    }
}

bool Compiler::Link(const vector<string>& input_obj_files,
                    const vector<string>& input_libs,
                    const string& options,
                    const string& export_symbols,
                          string& outfile)
{
    p_options = options;

    cl_device_type devtype;
    p_device->info(CL_DEVICE_TYPE, sizeof(devtype), &devtype, 0);

    if (devtype & CL_DEVICE_TYPE_ACCELERATOR)
    {
        return LinkForDSP(input_obj_files, input_libs, options,
                          export_symbols, outfile);
    }
    else
    {
        return false;
    }
}
#endif


bool Compiler::CompileAndLink(const string &source,
                              const string &options,
                                    string &outfile)
{
    p_options = options;

    cl_device_type devtype;
    p_device->info(CL_DEVICE_TYPE, sizeof(devtype), &devtype, 0);

    // No support for CPU (ARM) kernel compile - will be added later
    if (devtype & CL_DEVICE_TYPE_ACCELERATOR)
        return CompileAndLinkForDSP(source, options, outfile);
    else
        return false;
}


#ifndef _SYS_BIOS
/* Given a filesystem path, make sure the directory structure exists
 * If some directories in the path dont exist, create them in order
 * to create the entire path */
bool MakePath(const string& path)
{
    /* If no directories are specified in path, nothing to create */
    if (path.find("/") == string::npos) return true;

    /* Get the position where the directory names end */
    size_t path_dir_end_pos = path.find_last_of("/");
    string current_path(path.substr(0, path_dir_end_pos));
    /* Make directory path */
    string command = "mkdir -p ";
    command += current_path;

    int ret_code = system(command.c_str());
    if (ret_code != 0) return false;

    return true;
}

/* Given a file path, remove the last directory in it, assuming the file
 * has already been removed */
void RemoveLastDirInPath(const string& path)
{
    /* Get the position where the directory names end */
    size_t path_dir_end_pos = path.find_last_of("/");
    string current_path(path.substr(0, path_dir_end_pos));
    /* Remove directory path */
    unlink(current_path.c_str());
    rmdir(current_path.c_str());
}

/* Given a list of input header file paths, remove them from the filesystem */
void RemoveIHPaths(const vector<string>& ih_file_paths)
{
    if (!ih_file_paths.empty())
    {
        /* Remove each embedded header file that was created */
        for(const string& ih_file_path : ih_file_paths)
        {
            unlink(ih_file_path.c_str());
        }

        /* Remove each dir that was created for the embedded header files */
        for(const string& ih_file_path : ih_file_paths)
        {
            RemoveLastDirInPath(ih_file_path);
        }
    }
}

// Online compile for Acccelerator device (DSP) implemented via
// system call of clocl
bool Compiler::CompileForDSP(const string &source,
                             const map<string, string> &input_header_src,
                             const string &options,
                                   string &objfile,
                                   string &bc_objfile)
{

    // put each include source in a tmp file, in a tmp directory
    char  ih_dir_name_out[] = "/tmp/opencl-ihXXXXXX";
    char* fIhOutDir = mkdtemp(ih_dir_name_out);
    string ih_dir(fIhOutDir);
    vector<string> ih_file_paths;

    if (!input_header_src.empty())
    {
        for (pair<string, string> ih_file : input_header_src)
        {
            string ih_file_path(ih_dir);
            ih_file_path += "/";
            ih_file_path += ih_file.first;
            if (!MakePath(ih_file_path))
            {
                cout << "ERROR >> Unable to create directories in path: "
                     << ih_file_path << endl;
                /* Remove previously created paths */
                RemoveIHPaths(ih_file_paths);
                rmdir(ih_dir.c_str());
                return false;
            }
            ofstream ih_file_out(ih_file_path.c_str());
            ih_file_out << ih_file.second;
            ih_file_out.close();
            ih_file_paths.push_back(ih_file_path);
        }
    }

    // Begin clocl compilation
    string clocl_command("clocl -d -l ");
    clocl_command += "-I";
    clocl_command += ih_dir;
    clocl_command += " -I. ";

    clocl_command.reserve(options.size() + 64);

    // Add in options
    clocl_command += options;
    clocl_command += " ";

    if (do_keep_files) clocl_command += "-k ";
    if (do_debug)      clocl_command += "-g ";
    if (do_symbols)    clocl_command += "-s ";

    char  name_out[] = "/tmp/openclXXXXXX";
    int  fOutfile    = mkstemp(name_out);

    string srcfile(name_out);
    size_t name_out_size = srcfile.size();
    srcfile.reserve(name_out_size + 4);
    srcfile     += ".cl";

    objfile     = name_out;
    bc_objfile  = name_out;
    objfile     += ".obj";
    bc_objfile  += "_bc.obj";

    string logfile(name_out);
    logfile.reserve(name_out_size + 5);
    logfile += ".log";

    clocl_command +=  srcfile;
    clocl_command +=  " > ";
    clocl_command += logfile;

    // put source in a tmp file, run clocl, read tmp output in
    ofstream src_out(srcfile.c_str());
    src_out << source;
    src_out.close();

    int ret_code = system(clocl_command.c_str());

    ifstream log_in(logfile.c_str());
    string log_line;
    if (log_in.is_open())
    {
        while (getline(log_in, log_line))
        {
            appendLog(log_line);
            appendLog("\n");
            cout << log_line << endl;
        }
        log_in.close();
    }
    close(fOutfile);

    if (!do_keep_files && !do_debug) unlink(srcfile.c_str());
    unlink(logfile.c_str());
    unlink(name_out);

    RemoveIHPaths(ih_file_paths);

    /* Remove the embedded header tmp dir */
    unlink(fIhOutDir);
    rmdir(ih_dir.c_str());

    // Check for system() call failure or clocl compile failure
    if (ret_code != 0) return false;

    return true;
}


/**
 * Write native binary into file, create tmporary filename in p_outfile
 */
bool Compiler::WriteBinaryOut(string&       outfile_path,
                              const string& binary_str,
                              const string& outdir,
                              const string& outfile_ext)
{
    assert (binary_str.empty() == false);
    outfile_path = outdir;

    try
    {
        outfile_path += "/";
        char outfile_name[] = "opencl-binXXXXXX";
        int fOutfile  = mkstemp(outfile_name);
        outfile_path += string(outfile_name);
        outfile_path += outfile_ext;

        ofstream outfile(outfile_path.c_str(), ios::out | ios::binary);
        outfile.write(binary_str.data(), binary_str.size());
        outfile.close();
        close(fOutfile);
        unlink(outfile_name);
    }
    catch(...)
    {
        cout << ">> ERROR: Binary write out failure" << endl;
        return false;
    }
    return true;
}

/* Create a file with --export symbol entries given those entries in
 * a semicolon delimited string as input */
bool CreateExportSymbolsFile(const string& export_symbols,
                             const string& export_symbols_filename)
{
    assert(export_symbols.empty() == false);
    assert(export_symbols_filename.empty() == false);

    try
    {
        ofstream     outfile(export_symbols_filename.c_str());
        stringstream expsyms(export_symbols);
        string symbol;

        while(getline(expsyms, symbol, ';'))
        {
            string line("--export ");
            line += symbol;
            outfile << line << '\n';
        }

        outfile.close();
    }
    catch(...)
    {
        cout << ">> ERROR: Export Symbol File("
             << export_symbols_filename
             << ") " << "creation failure" << endl;
        return false;
    }

    return true;
}

// Online link for Acccelerator device (DSP) implemented via
// system call of clocl
bool Compiler::LinkForDSP(const vector<string>& input_obj_files,
                          const vector<string>& input_libs,
                          const string& options,
                          const string& export_symbols,
                                string& outfile)
{
    bool create_library = false;
    bool export_syms_file_created = false;
    if (options.find("-create-library") != string::npos) create_library = true;

    // Extract each object file source in a tmp file, in a tmp directory
    char  obj_dir_name_out[] = "/tmp/opencl-objXXXXXX";
    char* ObjOutDir = mkdtemp(obj_dir_name_out);
    string obj_dir(ObjOutDir);
    vector<string> obj_file_binary_paths;

    for (const string& lib_str : input_libs)
    {
        string lib_file_path;
        if(!WriteBinaryOut(lib_file_path, lib_str, obj_dir, ".a"))
        {
            unlink(obj_dir_name_out);
            rmdir(obj_dir.c_str());
            return false;
        }
        obj_file_binary_paths.push_back(lib_file_path);
    }

    for (const string& obj_file_str : input_obj_files)
    {
        string obj_file_path;
        if(!WriteBinaryOut(obj_file_path, obj_file_str, obj_dir, ".obj"))
        {
            unlink(obj_dir_name_out);
            rmdir(obj_dir.c_str());
            return false;
        }
        obj_file_binary_paths.push_back(obj_file_path);
    }

    /* Create export.syms file */
    char   exp_sym_name_out[]      = "/tmp/opencl-expsymXXXXXX";
    int    fExpSymfile             = mkstemp(exp_sym_name_out);
    string export_symbols_filename = string(exp_sym_name_out);

    if (!create_library && !export_symbols.empty())
    {
       if(CreateExportSymbolsFile(export_symbols, export_symbols_filename))
       {
            export_syms_file_created = true;
       }
    }

    // Begin clocl link
    string clocl_command("clocl ");
    clocl_command.reserve(options.size() + 64);

    char name_out[] = "/tmp/openclXXXXXX";
    int  fOutfile   = mkstemp(name_out);

    outfile = string(name_out);

    /* Handle possible link options */
    if (create_library)
    {
        clocl_command += "-create-library ";
        outfile       += ".a";
    }
    else
    {
        if (export_syms_file_created)
        {
            clocl_command += "--export-syms ";
            clocl_command += export_symbols_filename;
            clocl_command += " ";
        }
        clocl_command += "--link ";
        outfile       += ".out";
    }

    clocl_command += outfile;
    for (string& obj_file_binary_path : obj_file_binary_paths)
    {
        clocl_command += " ";
        clocl_command += obj_file_binary_path;
    }

    string logfile(name_out);
    logfile       += ".log";
    clocl_command +=  " > ";
    clocl_command += logfile;

    // put source in a tmp file, run clocl, read tmp output in
    int ret_code = system(clocl_command.c_str());

    ifstream log_in(logfile.c_str());
    string log_line;
    if (log_in.is_open())
    {
        while (getline(log_in, log_line))
        {
            appendLog(log_line);
            appendLog("\n");
            cout << log_line << endl;
        }
        log_in.close();
    }
    close(fOutfile);

    unlink(logfile.c_str());
    for (const string& obj_file_binary_path : obj_file_binary_paths)
    {
        unlink(obj_file_binary_path.c_str());
    }
    rmdir(obj_dir.c_str());
    unlink(name_out);

    /* Remove export.syms file */
    close(fExpSymfile);
    unlink(exp_sym_name_out);

    // Check for system() call failure or clocl compile failure
    if (ret_code != 0) return false;

    return true;
}
#endif // #ifndef _SYS_BIOS


// Online compile for Acccelerator device (DSP) implemented via
// system call of clocl
bool Compiler::CompileAndLinkForDSP(const string &source,
                                    const string &options,
                                          string &outfile)
{

#define STRINGIZE(x) #x
#define STRINGIZE2(x) STRINGIZE(x)
    const string product_version(STRINGIZE2(_PRODUCT_VERSION));
    string options_with_version(options);
    options_with_version += product_version;

    // Check if the kernel has already been compiled and cached with the same
    // options and compiler version
    if (do_cache_kernels)
    {
        string cached_file = genfile_cache::instance()->lookup(
            source, options_with_version);
        if (! cached_file.empty())
        {
            outfile = cached_file;
            return true;
        }
    }

    // Begin clocl compilation
    string clocl_command("clocl -d -I. ");
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
    ofstream src_out(srcfile.c_str());
    src_out << source;
    src_out.close();

    int ret_code = system(clocl_command.c_str());

    ifstream log_in(logfile.c_str());
    string log_line;
    if (log_in.is_open())
    {
        while (getline(log_in, log_line))
        {
            appendLog(log_line);
            appendLog("\n");
            cout << log_line << endl;
        }
        log_in.close();
    }
    close(fOutfile);
    if (! do_keep_files && !do_debug) unlink(srcfile.c_str());
    unlink(logfile.c_str());
    unlink(name_out);

    // Check for system() call failure or clocl compile failure
    if (ret_code != 0)
        return false;

    if (do_cache_kernels)
        genfile_cache::instance()->remember(outfile.c_str(), source,
                                            options_with_version);

    return true;
}

const string &Compiler::log() const
{
    return p_log;
}

const string &Compiler::options() const
{
    return p_options;
}

void Compiler::set_options(const string &options)
{
   p_options = options;
}

void Compiler::appendLog(const string &log)
{
    p_log += log;
}
