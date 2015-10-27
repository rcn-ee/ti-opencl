// LLVM function pass that adds implicit barriers to branches where it sees
// beneficial (and legal).
// 
// Copyright (c) 2013 Pekka Jääskeläinen / TUT
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
#include "llvm/Config/llvm-config.h"
#if LLVM_VERSION_MAJOR <= 3 && LLVM_VERSION_MINOR <=3
#  include <llvm/IR/Constants.h>
#endif

#include "config.h"
#include "ImplicitConditionalBarriers.h"
#include "Barrier.h"
#include "BarrierBlock.h"
#include "Workgroup.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#if LLVM_VERSION_MAJOR <= 3 && LLVM_VERSION_MINOR <3
#include "llvm/Constants.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#else
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#endif

#include <iostream>
#include "VariableUniformityAnalysis.h"

//#define DEBUG_COND_BARRIERS

using namespace llvm;
using namespace pocl;

namespace {
  static
  RegisterPass<ImplicitConditionalBarriers> X("implicit-cond-barriers",
                                              "Adds implicit barriers to branches.");
}

char ImplicitConditionalBarriers::ID = 0;

void
ImplicitConditionalBarriers::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.addRequired<PostDominatorTree>();
  AU.addPreserved<PostDominatorTree>();
#if LLVM_VERSION_MAJOR <= 3 && LLVM_VERSION_MINOR <=4
  AU.addRequired<DominatorTree>();
  AU.addPreserved<DominatorTree>();
  #else
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addPreserved<DominatorTreeWrapperPass>();
  #endif
  AU.addPreserved<VariableUniformityAnalysis>();
}

/**
 * Finds a predecessor that does not come from a back edge.
 *
 * This is used to include loops in the conditional parallel region.
 */
BasicBlock*
ImplicitConditionalBarriers::firstNonBackedgePredecessor(
    llvm::BasicBlock *bb) {

#if LLVM_VERSION_MAJOR <= 3 && LLVM_VERSION_MINOR <=4
    DominatorTree *DT = &getAnalysis<DominatorTree>();
    #else
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    #endif

    pred_iterator I = pred_begin(bb), E = pred_end(bb);
    if (I == E) return NULL;
    while (DT->dominates(bb, *I) && I != E) ++I;
    if (I == E) return NULL;
    else return *I;
}

bool
ImplicitConditionalBarriers::runOnFunction(Function &F) {
{
  if (!Workgroup::isKernelToProcess(F))
    return false;

  if (!Workgroup::hasWorkgroupBarriers(F))
    return false;
  
  PDT = &getAnalysis<PostDominatorTree>();

  typedef std::vector<BasicBlock*> BarrierBlockIndex;
  BarrierBlockIndex conditionalBarriers;
  for (Function::iterator i = F.begin(), e = F.end(); i != e; ++i) {
    BasicBlock *b = i;
    if (!Barrier::hasBarrier(b)) continue;

    // Unconditional barrier postdominates the entry node.
    if (PDT->dominates(b, &F.getEntryBlock())) continue;

#ifdef DEBUG_COND_BARRIERS
    std::cerr << "### found a conditional barrier";
    b->dump();
#endif
    conditionalBarriers.push_back(b);
  }

  if (conditionalBarriers.size() == 0) return false;

  bool changed = false;

  for (BarrierBlockIndex::const_iterator i = conditionalBarriers.begin();
       i != conditionalBarriers.end(); ++i) {
    BasicBlock *b = *i;
    // Trace upwards from the barrier until one encounters another
    // barrier or the split point that makes the barrier conditional. 
    // In case of the latter, add a new barrier to both branches of the split point. 

    // BB before which to inject the barrier.
    BasicBlock *pos = b;
    if (pred_begin(b) == pred_end(b)) {
      b->dump();
      assert (pred_begin(b) == pred_end(b));
    }
    BasicBlock *pred = firstNonBackedgePredecessor(b);

    while (!isa<BarrierBlock>(pred) && PDT->dominates(b, pred)) {

#ifdef DEBUG_COND_BARRIERS
      std::cerr << "### looking at BB " << pred->getName().str() << std::endl;
#endif
      pos = pred;
      // If our BB post dominates the given block, we know it is not the
      // branching block that makes the barrier conditional.
      pred = firstNonBackedgePredecessor(pred);

      if (pred == b) break; // Traced across a loop edge, skip this case.

    }

    if (isa<BarrierBlock>(pos)) continue;
    // Inject a barrier at the beginning of the BB and let the CanonicalizeBarrier
    // to clean it up (split to a separate BB).

    // mri-q of parboil breaks in case injected at the beginning
    // TODO: investigate. It might related to the alloca-converted
    // PHIs. It has a loop that is autoconverted to a b-loop and the
    // conditional barrier is inserted after the loop short cut check.
    Barrier::Create(pos->getFirstNonPHI());
#ifdef DEBUG_COND_BARRIERS
    std::cerr << "### added an implicit barrier to the BB" << std::endl;
    pos->dump();
#endif

    changed = true;
  }

//  F.dump();
//  F.viewCFGOnly();

  return changed;
}

}


