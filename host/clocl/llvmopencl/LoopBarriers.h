// Header for LoopBarriers.cc function pass.
// 
// Copyright (c) 2011 Universidad Rey Juan Carlos
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <set>

#ifndef POCL_LOOP_BARRIERS_H
#define POCL_LOOP_BARRIERS_H

#include "CompilerWarnings.h"
IGNORE_COMPILER_WARNING("-Wunused-parameter")

#include "llvm/Analysis/LoopPass.h"

POP_COMPILER_DIAGS

namespace pocl {
  class LoopBarriers : public llvm::LoopPass {
    
  public:
    static char ID;
    
  LoopBarriers() : LoopPass(ID) {}
    
    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
    virtual bool runOnLoop(llvm::Loop *L, llvm::LPPassManager &LPM);

  private:
    llvm::DominatorTree *DT;

    bool ProcessLoop(llvm::Loop *L, llvm::LPPassManager &LPM);
  };
}

#endif
