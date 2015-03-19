/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
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
 * \file compiler.cpp
 * \brief Compiler wrapper around Clang
 */

#include "compiler.h"
#include "deviceinterface.h"

#include <cstring>
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
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>

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
                                llvm::MemoryBuffer *source)
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
    codegen_opts.DebugInfo = false;
    codegen_opts.AsmVerbose = true;
    codegen_opts.OptimizationLevel = 3;

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

    // Set target options
    target_opts.Triple = llvm::sys::getHostTriple();
    //target_opts.Triple = "i386-unknown-linux-gnu";

    // Set invocation options
    invocation.setLangDefaults(clang::IK_OpenCL);

    // Parse the user options
    std::istringstream options_stream(options);
    std::string token;
    bool Werror = false, inI = false, inD = false;

    while (options_stream >> token)
    {
        if (inI)
        {
            // token is an include path
            header_opts.AddPath(token, clang::frontend::Angled, true, false, false);
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
        else if (token == "-D")
        {
            inD = true;
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

    // Create the diagnostics engine
    p_log_printer = new clang::TextDiagnosticPrinter(p_log_stream, diag_opts);
    p_compiler.createDiagnostics(0, NULL, p_log_printer);

    if (!p_compiler.hasDiagnostics())
        return false;

    p_compiler.getDiagnostics().setWarningsAsErrors(Werror);

    // Feed the compiler with source
    frontend_opts.Inputs.push_back(std::make_pair(clang::IK_OpenCL, "program.cl"));

    //ASW  TODO cleanup
#if 0
    prep_opts.addRemappedFile("program.cl", source);
#else
    std::string srcc = source->getBuffer();
    srcc = p_device->sourceTranslation(srcc);

    const llvm::StringRef s_data(srcc);
    const llvm::StringRef s_name("<source>");
    llvm::MemoryBuffer *buffer = 
    llvm::MemoryBuffer::getMemBuffer(s_data, s_name);

    prep_opts.addRemappedFile("program.cl", buffer);
#endif

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

    // Cleanup
    prep_opts.eraseRemappedFile(prep_opts.remapped_file_buffer_end());

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
