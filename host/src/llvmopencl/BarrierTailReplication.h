// Header for BarrierTailReplication.cc function pass.
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

#ifndef POCL_BARRIER_TAIL_REPLICATION
#define POCL_BARRIER_TAIL_REPLICATION

#include "config.h"
#if (defined LLVM_3_1 || defined LLVM_3_2)
#include "llvm/Function.h"
#else
#include "llvm/IR/Function.h"
#endif

#if LLVM_VERSION_MAJOR <= 3 && LLVM_VERSION_MINOR <=4 
#include "llvm/Analysis/Dominators.h"
#else
#include "llvm/IR/Dominators.h"
#endif
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <map>
#include <set>

namespace pocl {
  class Workgroup;

  class BarrierTailReplication : public llvm::FunctionPass {

  public:
    static char ID;

    BarrierTailReplication(): FunctionPass(ID) {}

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
    virtual bool runOnFunction(llvm::Function &F);
    
  private:
    typedef std::set<llvm::BasicBlock *> BasicBlockSet;
    typedef std::vector<llvm::BasicBlock *> BasicBlockVector;
    typedef std::map<llvm::Value *, llvm::Value *> ValueValueMap;

    llvm::DominatorTree *DT;
#if LLVM_VERSION_MAJOR <= 3 && LLVM_VERSION_MINOR >=6 
    llvm::DominatorTreeWrapperPass *DTP;
#endif
    llvm::LoopInfo *LI;

    bool ProcessFunction(llvm::Function &F);
    bool FindBarriersDFS(llvm::BasicBlock *bb,
                         BasicBlockSet &processed_bbs);
    bool ReplicateJoinedSubgraphs(llvm::BasicBlock *dominator,
                                  llvm::BasicBlock *subgraph_entry,
                                  BasicBlockSet &processed_bbs);

    llvm::BasicBlock* ReplicateSubgraph(llvm::BasicBlock *entry,
                                        llvm::Function *f);
    void FindSubgraph(BasicBlockVector &subgraph,
                      llvm::BasicBlock *entry);
    void ReplicateBasicBlocks(BasicBlockVector &new_graph,
                              llvm::ValueToValueMapTy &reference_map,
                              BasicBlockVector &graph,
                              llvm::Function *f);
    void UpdateReferences(const BasicBlockVector &graph,
                          llvm::ValueToValueMapTy &reference_map);

    bool CleanupPHIs(llvm::BasicBlock *BB);

    friend class pocl::Workgroup;
  };
}

#endif
