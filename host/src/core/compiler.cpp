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
 * \brief Compiler wrapper around Clang
 */

#include "compiler.h"
#include "deviceinterface.h"

#include <cstring>
#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/LangStandard.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/MemoryBuffer.h> // ASW
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <sys/stat.h>

std::string get_ocl_dsp();

using namespace Coal;

Compiler::Compiler(DeviceInterface *device)
: p_device(device), p_module(0), p_optimize(true), p_log_stream(p_log),
  p_log_printer(0)
{
}

Compiler::~Compiler()
{

}

bool Compiler::compile(const std::string &options,
                       llvm::MemoryBuffer *source,
                       llvm::LLVMContext *llvmcontext)
{
    /* Set options */
    p_options = options;

    clang::CodeGenOptions &codegen_opts = p_compiler.getCodeGenOpts();
    clang::DiagnosticOptions &diag_opts = p_compiler.getDiagnosticOpts();
    clang::FrontendOptions &frontend_opts = p_compiler.getFrontendOpts();
    clang::HeaderSearchOptions &header_opts = p_compiler.getHeaderSearchOpts();
    clang::LangOptions &lang_opts = p_compiler.getLangOpts();
    clang::TargetOptions &target_opts = p_compiler.getTargetOpts();
    clang::PreprocessorOptions &prep_opts = p_compiler.getPreprocessorOpts();
    clang::CompilerInvocation &invocation = p_compiler.getInvocation();

    // Set codegen options
    codegen_opts.setDebugInfo(clang::CodeGenOptions::NoDebugInfo);
    codegen_opts.AsmVerbose = true;

    // level >= 1 is too much for the pocl transformations.
    // TODO: codegen_opts.OptimizationLevel = 0;
    codegen_opts.OptimizationLevel = 2;

    // Set diagnostic options
    diag_opts.Pedantic = true;
    diag_opts.ShowColumn = true;
    diag_opts.ShowLocation = true;
    diag_opts.ShowCarets = false;
    diag_opts.ShowFixits = true;
    diag_opts.ShowColors = false;
    diag_opts.ErrorLimit = 19;
    diag_opts.MessageLength = 0;

    // Set frontend options
    frontend_opts.ProgramAction = clang::frontend::EmitLLVMOnly;
    frontend_opts.DisableFree = true;

    // Set header search options
    header_opts.Verbose = false;
    header_opts.UseBuiltinIncludes = false;
    header_opts.UseStandardSystemIncludes = false;
    header_opts.UseStandardCXXIncludes = false;

    // Set preprocessor options
    prep_opts.RetainRemappedFileBuffers = true;

    //prep_opts.ImplicitPCHInclude = "/usr/share/ti/opencl/clc.h.pch";
    //prep_opts.DisablePCHValidation = true;
    //prep_opts.AllowPCHWithCompilerErrors= true;
    //prep_opts.UsePredefines = 1;

    prep_opts.Includes.push_back("clc.h");
    prep_opts.Includes.push_back(p_device->builtinsHeader());

    // Set lang options
    lang_opts.NoBuiltin = true;
    lang_opts.OpenCL = true;
    lang_opts.CPlusPlus = false;
    lang_opts.MathErrno = false;

    // Set target options
    cl_device_type devtype;
    p_device->info(CL_DEVICE_TYPE, sizeof(devtype), &devtype, 0);

    if (devtype == CL_DEVICE_TYPE_CPU)
       target_opts.Triple = "spir-unknown-unknown-unknown";

#if 0
    // Originally: target_opts.Triple = llvm::sys::getHostTriple();
#if defined(__i386__) && defined(DSPC868X)
       target_opts.Triple = "i386-pc-linux-gnu";
#elif defined(__x86_64__) && defined(DSPC868X)
       target_opts.Triple = "x86_64-unknown-linux-gnu";
#elif defined(__arm__) || !defined(DSPC868X)
       target_opts.Triple = "armv7-unknown-linux-gnueabihf";
#else
       target_opts.Triple = "i386-unknown-linux-gnu";
#endif
#endif

    else // devtype != CL_DEVICE_TYPE_CPU
    {
       // For 6X, use the 'spir' target, since it implements opencl specs
       target_opts.Triple = "spir-unknown-unknown-unknown";

       // Currently, llp6x does not handle fused multiply and add
       // llvm intrinsics (llvm.fmuladd.*). Disable generating these
       // intrinsics using clang -ffp-contract=off option
       codegen_opts.setFPContractMode(clang::CodeGenOptions::FPC_Off);
    }

    // Parse the user options
    std::istringstream options_stream(options);
    std::string token;
    bool Werror = false, inI = false, inD = false;

    /*-------------------------------------------------------------------------
    * Add OpenCL C header path as a default location for searching for headers
    *------------------------------------------------------------------------*/
    std::string header_path(get_ocl_dsp());
    header_opts.AddPath(header_path, clang::frontend::Angled, false, false);

    while (options_stream >> token)
    {
        if (inI)
        {
            // token is an include path
            header_opts.AddPath(token, clang::frontend::Angled, false, false);
            inI = false;
            continue;
        }
        else if (inD)
        {
            // token is name or name=value
            prep_opts.addMacroDef(token);
        }

	//Handle -I xxx or -Ixxx. Assuming no other -I option prefix
        if (token == "-I")
        {
            inI = true;
        }
	else if (token.compare(0,2,"-I") == 0) 
	{
	   header_opts.AddPath(token.substr(2), clang::frontend::Angled, false,
                                                                         false);
	}
	//Handle -D xxx or -Dxxx. Assuming no other -D option prefix
        else if (token == "-D")
        {
            inD = true;
        }
	else if (token.compare(0,2,"-D") == 0) //Handle -Dxxx (no space between)
	{
	   prep_opts.addMacroDef(token.substr(2));
	}
        else if (token == "-cl-single-precision-constant")
        {
            lang_opts.SinglePrecisionConstants = true;
        }
        else if (token == "-cl-opt-disable")
        {
            p_optimize = false;
            codegen_opts.OptimizationLevel = 0;
        }
        else if (token == "-cl-mad-enable")
        {
            codegen_opts.LessPreciseFPMAD = true;
        }
        else if (token == "-cl-unsafe-math-optimizations")
        {
            codegen_opts.UnsafeFPMath = true;
        }
        else if (token == "-cl-finite-math-only")
        {
            codegen_opts.NoInfsFPMath = true;
            codegen_opts.NoNaNsFPMath = true;
        }
        else if (token == "-cl-fast-relaxed-math")
        {
            codegen_opts.UnsafeFPMath = true;
            codegen_opts.NoInfsFPMath = true;
            codegen_opts.NoNaNsFPMath = true;
            lang_opts.FastRelaxedMath = true;
        }
        else if (token == "-w")
        {
            diag_opts.IgnoreWarnings = true;
        }
        else if (token == "-Werror")
        {
            Werror = true;
        }
    }

    add_macrodefs_for_supported_opencl_extensions(prep_opts);  

    // Set invocation options
    //invocation.setLangDefaults(lang_opts,clang::IK_OpenCL);
    invocation.setLangDefaults(lang_opts,clang::IK_OpenCL, clang::LangStandard::lang_opencl12);

    // Create the diagnostics engine
    p_log_printer = new clang::TextDiagnosticPrinter(p_log_stream, &diag_opts);
    p_compiler.createDiagnostics(p_log_printer);

    if (!p_compiler.hasDiagnostics())
        return false;

    p_compiler.getDiagnostics().setWarningsAsErrors(Werror);

    // Feed the compiler with source
    frontend_opts.Inputs.push_back(clang::FrontendInputFile("program.cl", clang::IK_OpenCL));

    //ASW  TODO cleanup
#if 0
    prep_opts.addRemappedFile("program.cl", source);
#else


    std::string srcc(source->getBuffer());
    srcc += "\n\n";

    const llvm::StringRef s_data(srcc);
    const llvm::StringRef s_name("<source>");
    llvm::MemoryBuffer *buffer = 
    llvm::MemoryBuffer::getMemBuffer(s_data, s_name);

    prep_opts.addRemappedFile("program.cl", buffer);
#endif

    //timespec t0, t1;
    //clock_gettime(CLOCK_MONOTONIC, &t0);
    // Compile
    llvm::OwningPtr<clang::CodeGenAction> act(
                   new clang::EmitLLVMOnlyAction(llvmcontext)
    );

    if (!p_compiler.ExecuteAction(*act))
    {
        // DEBUG
        std::cout << log() << std::endl;
        return false;
    }
    //clock_gettime(CLOCK_MONOTONIC, &t1);
    //printf("clang time: %6.4f secs\n", 
       //(float)t1.tv_sec-t0.tv_sec+(t1.tv_nsec-t0.tv_nsec)/1e9);

    p_log_stream.flush();
    p_module = act->takeModule();

    // uncomment to debug the llvm IR
    // p_module->dump();  

    // Cleanup
    prep_opts.eraseRemappedFile(prep_opts.remapped_file_buffer_end());

    return true;
}

// Query the device to get list of supported OpenCL extensions.  Standard
// requires that each supported extension has a macro definition with the
// same name as the extension
void Compiler::add_macrodefs_for_supported_opencl_extensions
                                        (clang::PreprocessorOptions &prep_opts)
{
    // Get the extensions string for the device
    size_t size;
    p_device->info(CL_DEVICE_EXTENSIONS, 0, NULL, &size);

    char *extensions = new char[size + 1];
    memset( extensions, CHAR_MIN, sizeof(char)*(size+1) );

    p_device->info(CL_DEVICE_EXTENSIONS, sizeof(char)*size, extensions, NULL);

    // Create macro definitions from the extension names
    std::istringstream extensions_stream(extensions);
    std::string token;

    while (extensions_stream >> token)
       prep_opts.addMacroDef(token);

    delete [] extensions;
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

bool Compiler::optimize() const
{
    return p_optimize;
}

llvm::Module *Compiler::module() const
{
    return p_module;
}

void Compiler::appendLog(const std::string &log)
{
    p_log += log;
}
