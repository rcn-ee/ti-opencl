/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include "options.h"

#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>
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

std::string get_ocl_dsp();

Compiler::Compiler()
: p_module(0), p_optimize(true), p_log_stream(p_log),
  p_log_printer(0)
{
}

Compiler::~Compiler()
{

}

bool Compiler::compile(const std::string &options,
                                llvm::MemoryBuffer *source,
                                std::string filename)
{
    bool use_pch = access("/usr/share/ti/opencl/clc.h.pch", F_OK) != -1;

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
    if (opt_debug)
    {
        codegen_opts.setDebugInfo(clang::CodeGenOptions::FullDebugInfo);
        codegen_opts.MainFileName = filename;
        codegen_opts.OptimizationLevel = 0;
        p_optimize = false;
        use_pch    = false;
        lang_opts.Optimize  = false;
    }
    else 
    { 
        codegen_opts.setDebugInfo(clang::CodeGenOptions::NoDebugInfo);
        codegen_opts.OptimizationLevel = 2;
        lang_opts.Optimize  = true;
    }

    codegen_opts.AsmVerbose = true;

    // level >= 1 is too much for the pocl transformations.
    // TODO: codegen_opts.OptimizationLevel = 0;

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

    // Set lang options
    lang_opts.NoBuiltin = true;
    lang_opts.OpenCL = true;
    lang_opts.CPlusPlus = false;
    lang_opts.MathErrno = false;

    // Set target options
    // For 6X, use the 'spir' target as it implements opencl specs
    target_opts.Triple = "spir-unknown-unknown-unknown";

    // Currently, llp6x does not handle fused multiply and add
    // llvm intrinsics (llvm.fmuladd.*). Disable generating these
    // intrinsics using clang -ffp-contract=off option
    codegen_opts.setFPContractMode(clang::CodeGenOptions::FPC_Off);

    // Parse the user options
    std::istringstream options_stream(options);
    std::string token;
    bool Werror = false, inI = false, inD = false;

    /*-------------------------------------------------------------------------
    * Add OpenCL C header path as a default location for searching for headers
    *------------------------------------------------------------------------*/
    header_opts.AddPath(get_ocl_dsp(), clang::frontend::Angled, false, false);

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

        if (token == "-I")
        {
            inI = true;
        }
        else if (token.compare(0,2,"-I") == 0) //Handle -Ixxx (no space between)
        {
           header_opts.AddPath(token.substr(2), clang::frontend::Angled, false,
                                                                         false);
        }
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
            use_pch = false;
        }
        else if (token == "-cl-opt-disable")
        {
            p_optimize = false;
            use_pch    = false;
            lang_opts.Optimize  = false;
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
            use_pch    = false;
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

    if (!opt_builtin)
    {
        if (use_pch)
        {
            prep_opts.ImplicitPCHInclude = "/usr/share/ti/opencl/clc.h.pch";
            prep_opts.DisablePCHValidation = true;
        }
        else prep_opts.Includes.push_back("clc.h");

        prep_opts.Includes.push_back("dsp_c.h");
        prep_opts.Includes.push_back("dsp.h");
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
    frontend_opts.Inputs.push_back(clang::FrontendInputFile(filename.c_str(), clang::IK_OpenCL));
    //prep_opts.addRemappedFile(filename.c_str(), source);

    // Compile
    llvm::OwningPtr<clang::CodeGenAction> act(
        new clang::EmitLLVMOnlyAction(&llvm::getGlobalContext())
    );

    if (!p_compiler.ExecuteAction(*act))
    {
        // DEBUG
        std::cout << log() << std::endl;
        return false;
    }

    p_log_stream.flush();
    p_module = act->takeModule();

    // uncomment to debug the llvm IR
    // p_module->dump();  

    // Cleanup
    //prep_opts.eraseRemappedFile(prep_opts.remapped_file_buffer_end());

    return true;
}

// Hard code the list of supported OpenCL extensions. (Need to be in sync with
//    OpenCL runtime!)
// Standard requires that each supported extension has a macro definition with
// the same name as the extension
void Compiler::add_macrodefs_for_supported_opencl_extensions
                                        (clang::PreprocessorOptions &prep_opts)
{
    char extensions[] = "cl_khr_byte_addressable_store"
                          " cl_khr_global_int32_base_atomics"
                          " cl_khr_global_int32_extended_atomics"
                          " cl_khr_local_int32_base_atomics"
                          " cl_khr_local_int32_extended_atomics"
                          " cl_ti_msmc_buffers"
                          " cl_ti_clmalloc";

    // Create macro definitions from the extension names
    std::istringstream extensions_stream(extensions);
    std::string token;

    while (extensions_stream >> token)
       prep_opts.addMacroDef(token);

    if (getenv("TI_OCL_ENABLE_FP64") != NULL)
       prep_opts.addMacroDef("cl_khr_fp64");
}

const std::string &Compiler::log() const
{
    return p_log;
}

const std::string &Compiler::options() const
{
    return p_options;
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
