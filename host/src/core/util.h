/******************************************************************************
 * Copyright (c) 2012-2014, Texas Instruments Incorporated - http://www.ti.com/
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
/**
 * \file core/util.h
 * \brief misc utils
 */

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/DebugInfo.h>

#ifndef _UTIL_H
#define _UTIL_H

// Parse first line in a file, read integer immediately following a string
uint32_t parse_file_line_value(const char *fname, const char *sname,
                               uint32_t default_val);

// OpenCL/LLVM utils
bool          isKernelFunction(llvm::Function &F);
bool          getReqdWGSize(llvm::Function &F, int wgsizes[3]);
bool          containsBarrierCall(llvm::Module &M);
bool          canLocalsFitInReg(llvm::BasicBlock *bb);
bool          canLocalsFitInReg(llvm::Function &F);

llvm::MDNode* getDebugInfo(llvm::Function &F, unsigned int &scope_line_num);
unsigned int  findFirstDebugLine(llvm::BasicBlock *bb);
unsigned int  findLastDebugLine(llvm::BasicBlock *bb);

int           getKernelConfigGEPInstIndex(llvm::Instruction *instr);
int           getKernelConfigLoadInstIndex(llvm::Instruction *instr);

#endif // _UTIL_H

