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
 * \file cpu/program.h
 * \brief CPU program
 */

#ifndef __CPU_PROGRAM_H__
#define __CPU_PROGRAM_H__

#include "../deviceinterface.h"

namespace llvm
{
    class ExecutionEngine;
    class Module;
}

namespace Coal
{

class CPUDevice;
class Program;

/**
 * \brief CPU program
 *
 * This class implements the \c Coal::DeviceProgram interface for CPU
 * acceleration.
 *
 * It's main purpose is to initialize a \c llvm::JIT object to run LLVM bitcode,
 * in \c initJIT().
 */
class CPUProgram : public DeviceProgram
{
    public:
        /**
         * \brief Constructor
         * \param device CPU device to which this program is attached
         * \param program \c Coal::Program that will be run
         */
        CPUProgram(CPUDevice *device, Program *program);
        ~CPUProgram();

        bool linkStdLib() const;
        void createOptimizationPasses(llvm::PassManager *manager, bool optimize);
        bool build(llvm::Module *module);

        /**
         * \brief Initialize an LLVM JIT
         *
         * This function creates a \c llvm::JIT object to run this program on
         * the CPU. A few implementation details :
         *
         * - The JIT is set not to resolve unknown symbols using \c dlsym().
         *   This way, a malicious kernel cannot execute arbitrary code on
         *   the host by declaring \c libc functions and calling them.
         * - All the unknown function names are passed to \c getBuiltin() to
         *   get native built-in implementations.
         *
         * \return true if success, false otherwise
         */
        bool initJIT();
        llvm::ExecutionEngine *jit() const; /*!< \brief Current LLVM execution engine */

    private:
        CPUDevice *p_device;
        Program *p_program;

        llvm::ExecutionEngine *p_jit;
        llvm::Module *p_module;
};

}

#endif
