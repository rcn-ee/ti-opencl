// Class for kernels, a special kind of function.
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

#ifndef _POCL_KERNEL_H
#define _POCL_KERNEL_H
#include "llvm\Config\llvm-config.h"
#include "ParallelRegion.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#if LLVM_VERSION_MAJOR <= 3 && LLVM_VERSION_MINOR <=3
#include "llvm/Analysis/Dominators.h"
#else
#include "llvm/IR/Dominators.h"
#endif

namespace pocl {

  class Kernel : public llvm::Function {
  public:
    void getExitBlocks(llvm::SmallVectorImpl<BarrierBlock *> &B);
    ParallelRegion *createParallelRegionBefore(BarrierBlock *B);
    
    ParallelRegion::ParallelRegionVector* 
      getParallelRegions(llvm::LoopInfo *LI);

    void addLocalSizeInitCode(size_t LocalSizeX, size_t LocalSizeY, size_t LocalSizeZ);

    static bool isKernel(const llvm::Function &F);

    static bool classof(const Kernel *) { return true; }
    // We assume any function can be a kernel. This could be used
    // to check for metadata (but would need to be overrideable somehow
    // to honor the forced kernel name(s) parameter in command line.
    static bool classof(const llvm::Function *) { return true; }
  };

}

#endif
