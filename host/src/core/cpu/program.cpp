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
 * \file cpu/program.cpp
 * \brief CPU program
 */

#include "program.h"
#include "device.h"
#include "kernel.h"
#include "builtins.h"

#include "../program.h"

#if 0 // JIT compilation disabled
#include <llvm/PassManager.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/Support/ErrorHandling.h>
#endif

#include <string>
#include <iostream>

using namespace Coal;
using namespace llvm;


CPUProgram::CPUProgram(CPUDevice *device, Program *program)
: DeviceProgram(), p_device(device), p_program(program), p_jit(0)
{

}

CPUProgram::~CPUProgram()
{
#if 0
    if (p_jit)
    {
        // Dont delete the module
        p_jit->removeModule(p_module);

        delete p_jit;
    }
#endif
}

bool CPUProgram::linkStdLib() const
{
    return true;
}

void CPUProgram::createOptimizationPasses(llvm::PassManager *manager,
                                          bool optimize, bool hasBarrier)
{
}

bool CPUProgram::build(llvm::Module *module, std::string *binary_str,
                       char *binary_filename)
{
    // Nothing to build
    p_module = module;

    return true;
}

bool CPUProgram::initJIT()
{
    return false;
#if 0
    if (p_jit)
        return true;

    if (!p_module)
        return false;

    // Create the JIT
    std::string err;
    p_jit = llvm::EngineBuilder(std::unique_ptr<Module>(p_module))
                            .setErrorStr(&err)
#if defined (__arm__)
    // uncomment to try the MCJIT for ARM
    //                        .setMCJITMemoryManager(
    //                              llvm::make_unique<SectionMemoryManager>())
#endif
                            .create();

    if (!p_jit)
    {
        std::cout << "Unable to create a JIT: " << err << std::endl;
        return false;
    }

    p_jit->DisableSymbolSearching(true);    // Avoid an enormous security hole (a kernel calling system())
    p_jit->InstallLazyFunctionCreator(&getBuiltin);

    return true;
#endif
}

llvm::ExecutionEngine *CPUProgram::jit() const
{
    return p_jit;
}
