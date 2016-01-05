// LLVM function pass to create loops that run all the work items 
// in a work group while respecting barrier synchronization points.
// 
// Copyright (c) 2012-2014 Pekka Jääskeläinen / Tampere University of Technology
// Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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

#define DEBUG_TYPE "workitem-loops"

#include "WorkitemLoops.h"
#include "Workgroup.h"
#include "Barrier.h"
#include "Kernel.h"
#include "config.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/CommandLine.h"
#ifdef LLVM_3_1
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/TypeBuilder.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/ValueSymbolTable.h"
#elif defined LLVM_3_2
#include "llvm/IRBuilder.h"
#include "llvm/TypeBuilder.h"
#include "llvm/DataLayout.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/ValueSymbolTable.h"
#else
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueSymbolTable.h"
#endif
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <llvm/Support/InstIterator.h>
#include "WorkitemHandlerChooser.h"

#include <iostream>
#include <map>
#include <sstream>
#include <vector>

//#define DUMP_CFGS

#include "DebugHelpers.h"

//#define DEBUG_WORK_ITEM_LOOPS

#include "VariableUniformityAnalysis.h"
#include "../core/util.h"

#define CONTEXT_ARRAY_ALIGN 64

using namespace llvm;
using namespace pocl;

namespace {
  static
  RegisterPass<WorkitemLoops> X("workitemloops", 
                                "Workitem loop generation pass");
}

char WorkitemLoops::ID = 0;

void
WorkitemLoops::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.addRequired<DominatorTree>();
  AU.addRequired<PostDominatorTree>();
  AU.addRequired<LoopInfo>();
// TODO - Removed due to compilation error
#if 0
#ifdef LLVM_3_1
  AU.addRequired<TargetData>();
#else
  AU.addRequired<DataLayout>();
#endif
#endif

  AU.addRequired<VariableUniformityAnalysis>();
  AU.addPreserved<VariableUniformityAnalysis>();

  AU.addRequired<pocl::WorkitemHandlerChooser>();
  AU.addPreserved<pocl::WorkitemHandlerChooser>();

}

bool
WorkitemLoops::runOnFunction(Function &F)
{
  if (!Workgroup::isKernelToProcess(F))
    return false;

  if (getAnalysis<pocl::WorkitemHandlerChooser>().chosenHandler() != 
      pocl::WorkitemHandlerChooser::POCL_WIH_LOOPS)
    return false;

  int wgsizes[3];
  if (getReqdWGSize(F, wgsizes))
    if (wgsizes[0] == 1 && wgsizes[1] == 1 && wgsizes[2] == 1)
      return false;

  DT = &getAnalysis<DominatorTree>();
  LI = &getAnalysis<LoopInfo>();
  PDT = &getAnalysis<PostDominatorTree>();
  VUA = &getAnalysis<VariableUniformityAnalysis>();

  tempInstructionIndex = 0;

#if 0
  std::cerr << "### original:" << std::endl;
  chopBBs(F, *this);
  F.viewCFG();
#endif
//  F.viewCFGOnly();

  bool changed = ProcessFunction(F);

#ifdef DUMP_CFGS
  dumpCFG(F, F.getName().str() + "_after_wiloops.dot", 
          original_parallel_regions);
#endif

#if 0
  std::cerr << "### after:" << std::endl;
  F.viewCFG();
#endif

  changed |= fixUndominatedVariableUses(DT, F);

  // YUAN TODO: Causing significant opt6x time increase, need to investigate
  // changed |= removeBarrierCalls(&F);

#if 0
  /* Split large BBs so we can print the Dot without it crashing. */
  changed |= chopBBs(F, *this);
  F.viewCFG();
#endif
  contextArrays.clear();
  tempInstructionIds.clear();
  intraRegionAllocas.clear();
  WGInvariantMap.clear();

  return changed;
}

std::pair<llvm::BasicBlock *, llvm::BasicBlock *>
WorkitemLoops::CreateLoopAround
(ParallelRegion &region,
 llvm::BasicBlock *entryBB, llvm::BasicBlock *exitBB, 
 bool peeledFirst, llvm::Value *localIdVar, size_t LocalSizeForDim,
 int dim, bool regLocals,
 bool addIncBlock, llvm::Value *lsizeDim) 
{
  assert (localIdVar != NULL);

  /*

    Generate a structure like this for each loop level (x,y,z):

    for.init:

    ; if peeledFirst is false:
    store i32 0, i32* %_local_id_x, align 4

    ; if peeledFirst is true (assume the 0,0,0 iteration has been executed earlier)
    ; assume _local_id_x_first is is initialized to 1 in the peeled pregion copy
    store _local_id_x_first, i32* %_local_id_x, align 4
    store i32 0, %_local_id_x_first

    br label %for.body

    for.body: 

    ; the parallel region code here

    br label %for.inc

    for.inc:

    ; Separated inc and cond check blocks for easier loop unrolling later on.
    ; Can then chain N times for.body+for.inc to unroll.

    %2 = load i32* %_local_id_x, align 4
    %inc = add nsw i32 %2, 1

    store i32 %inc, i32* %_local_id_x, align 4
    br label %for.cond

    for.cond:

    ; loop header, compare the id to the local size
    %0 = load i32* %_local_id_x, align 4
    %cmp = icmp ult i32 %0, i32 123
    br i1 %cmp, label %for.body, label %for.end

    for.end:

    OPTIMIZE: Use a separate iteration variable across all the loops to iterate the context 
    data arrays to avoid needing multiplications to find the correct location, and to 
    enable easy vectorization of loading the context data when there are parallel iterations.
  */     

  llvm::BasicBlock *loopBodyEntryBB = entryBB;
  llvm::LLVMContext &C = loopBodyEntryBB->getContext();
  llvm::Function *F = loopBodyEntryBB->getParent();
  loopBodyEntryBB->setName(std::string("pregion_for_entry.") + entryBB->getName().str());

  assert (exitBB->getTerminator()->getNumSuccessors() == 1);

  llvm::BasicBlock *oldExit = exitBB->getTerminator()->getSuccessor(0);

  llvm::BasicBlock *forInitBB = 
    BasicBlock::Create(C, "pregion_for_init", F, loopBodyEntryBB);

#if 0 // pocl original
  llvm::BasicBlock *loopEndBB = 
    BasicBlock::Create(C, "pregion_for_end", F, exitBB);

  llvm::BasicBlock *forCondBB = 
    BasicBlock::Create(C, "pregion_for_cond", F, exitBB);
#else // TI
  llvm::BasicBlock *forCondBB =
    BasicBlock::Create(C, "pregion_for_cond", F, oldExit);
  llvm::BasicBlock *loopEndBB =
    BasicBlock::Create(C, "pregion_for_end", F, oldExit);
#endif

  DT->runOnFunction(*F);

  /*---------------------------------------------------------------------------
   * Init Block
   *-------------------------------------------------------------------------*/
  //  F->viewCFG();
  /* Fix the old edges jumping to the region to jump to the basic block
     that starts the created loop. Back edges should still point to the
     old basic block so we preserve the old loops. */
  BasicBlockVector preds;
  llvm::pred_iterator PI = 
    llvm::pred_begin(entryBB), 
    E = llvm::pred_end(entryBB);

  for (; PI != E; ++PI)
    {
      llvm::BasicBlock *bb = *PI;
      preds.push_back(bb);
    }    

  for (BasicBlockVector::iterator i = preds.begin();
       i != preds.end(); ++i)
    {
      llvm::BasicBlock *bb = *i;
      /* Do not fix loop edges inside the region. The loop
         is replicated as a whole to the body of the wi-loop.*/
      if (DT->dominates(loopBodyEntryBB, bb))
        continue;
      bb->getTerminator()->replaceUsesOfWith(loopBodyEntryBB, forInitBB);
    }

  IRBuilder<> builder(forInitBB);

  #define MAXGENINSTRS 32
  llvm::Instruction *lcinit[MAXGENINSTRS], *lccond[MAXGENINSTRS];
  int lcinit_count = 0, lccond_count = 0;

#if 0 // pocl original
  if (peeledFirst)
    {
      lcinit[lcinit_count++] =
      builder.CreateStore(builder.CreateLoad(localIdXFirstVar), localIdVar);
      lcinit[lcinit_count++] =
      builder.CreateStore
        (ConstantInt::get(IntegerType::get(C, size_t_width), 0), localIdXFirstVar);
    }
  else
    {
      lcinit[lcinit_count++] =
      builder.CreateStore
        (ConstantInt::get(IntegerType::get(C, size_t_width), 0), localIdVar);
    }
#endif

  lcinit[lcinit_count++] =
  builder.CreateBr(loopBodyEntryBB);

  /*---------------------------------------------------------------------------
   * loopBodyEntry entry Block
   *-------------------------------------------------------------------------*/
  builder.SetInsertPoint(& loopBodyEntryBB->front());
  llvm::Type  *Int32 = IntegerType::get(C, 32);
  llvm::Value *zero  = ConstantInt::get(Int32, 0);
  llvm::Value *one   = ConstantInt::get(Int32, 1);

  PHINode *phi = builder.CreatePHI(Int32, 2);
  phiDim[dim]  = phi;
  if (dim == 0 && peeledFirst)
    phi->addIncoming(phiXFirst[1] ? phiXFirst[1] :
                     (phiXFirst[2] ? phiXFirst[2] : one), forInitBB);
  else
    phi->addIncoming(zero, forInitBB);
  if (phiXFirst[dim] != NULL)
  {
    phiXFirst[dim]->insertAfter(phi);
    phiXFirst[dim]->addIncoming(dim == 2 ? one :
                       (phiXFirst[dim+1] ? phiXFirst[dim+1] : one), forInitBB);
    phiXFirst[dim]->addIncoming(zero, forCondBB);
  }

  /*---------------------------------------------------------------------------
   * loopBodyExit Block
   *-------------------------------------------------------------------------*/
  exitBB->getTerminator()->replaceUsesOfWith(oldExit, forCondBB);

  /*---------------------------------------------------------------------------
   * forCond Block
   *-------------------------------------------------------------------------*/
#if 0 // pocl original
  if (addIncBlock)
    {
      AppendIncBlock(exitBB, localIdVar);
    }

  builder.SetInsertPoint(forCondBB);

  if (! addIncBlock)
  {
    builder.CreateStore
      (builder.CreateAdd
       (builder.CreateLoad(localIdVar),
        ConstantInt::get(IntegerType::get(C, size_t_width), 1)),
       localIdVar);
  }
  llvm::Value *cmpResult;
  if (lsizeDim == NULL)
  {
    cmpResult = 
      // builder.CreateICmpULT
      builder.CreateICmpSLT
        (builder.CreateLoad(localIdVar),
         (ConstantInt::get
          (IntegerType::get(C, size_t_width), 
           LocalSizeForDim))
        );
  }
  else
  {
    cmpResult = 
      // builder.CreateICmpULT
      builder.CreateICmpSLT
        (builder.CreateLoad(localIdVar),
         lsizeDim
        );
  }
#else  // TI
  builder.SetInsertPoint(forCondBB);
  llvm::Value *inc = builder.CreateAdd(phi, one);
  phi->addIncoming(inc, forCondBB);

  llvm::Value *cmpResult;
  if (lsizeDim == NULL)
     cmpResult = builder.CreateICmpSLT(inc,
            ConstantInt::get(Int32, LocalSizeForDim));
  else
     cmpResult = builder.CreateICmpSLT(inc, lsizeDim);
#endif
  lccond[lccond_count++] = dyn_cast<llvm::Instruction>(inc);
  lccond[lccond_count++] = dyn_cast<llvm::Instruction>(cmpResult);
      
  Instruction *loopBranch =
      builder.CreateCondBr(cmpResult, loopBodyEntryBB, loopEndBB);
  lccond[lccond_count++] = loopBranch;

  if (regLocals)
  {
    /* Add the metadata to mark a parallel loop. The metadata 
       refer to a loop-unique dummy metadata that is not merged
       automatically. */

    /* This creation of the identifier metadata is copied from
       LLVM's MDBuilder::createAnonymousTBAARoot(). */
    MDNode *Dummy = MDNode::getTemporary(C, ArrayRef<Value*>());
    MDNode *Root = MDNode::get(C, Dummy);
    // At this point we have
    //   !0 = metadata !{}            <- dummy
    //   !1 = metadata !{metadata !0} <- root
    // Replace the dummy operand with the root node itself and delete the dummy
    Root->replaceOperandWith(0, Root);
    MDNode::deleteTemporary(Dummy);
    // We now have
    //   !1 = metadata !{metadata !1} <- self-referential root

#ifdef LLVM_3_3
    loopBranch->setMetadata("llvm.loop.parallel", Root);
#else
    loopBranch->setMetadata("llvm.loop", Root);
#endif
    region.AddParallelLoopMetadata(Root);
  }

  builder.SetInsertPoint(loopEndBB);
  lccond[lccond_count++] =
  builder.CreateBr(oldExit);

  if (di_function != NULL)
  {
    if (region_begin_line != 0)
    {
      DebugLoc dloc = DebugLoc::get(region_begin_line, 0, di_function, NULL);
      for (int i = 0; i < lcinit_count; i++)
        lcinit[i]->setDebugLoc(dloc);
    }
    if (region_end_line != 0)
    {
      DebugLoc dloc = DebugLoc::get(region_end_line, 0, di_function, NULL);
      for (int i = 0; i < lccond_count; i++)
        lccond[i]->setDebugLoc(dloc);
    }
  }

  return std::make_pair(forInitBB, loopEndBB);
}

ParallelRegion*
WorkitemLoops::RegionOfBlock(llvm::BasicBlock *bb)
{
  for (ParallelRegion::ParallelRegionVector::iterator
           i = original_parallel_regions->begin(), 
           e = original_parallel_regions->end();
       i != e; ++i) 
  {
    ParallelRegion *region = (*i);
    if (region->HasBlock(bb)) return region;
  } 
  return NULL;
}

// PreAnalyze kernel function, find out dimension (borrowed from wga)
// PreCreate local sizes which are workgroup invariant
void WorkitemLoops::FindKernelDim(Function &F)
{
  maxDim = 1;
  for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I)
    if (CallInst * callInst = dyn_cast<CallInst>(&*I))
    {
      if (!callInst->getCalledFunction()) continue;
      std::string functionName(callInst->getCalledFunction()->getName());

      if (functionName == "get_local_id" || 
          functionName == "get_global_id")
      {
        Value *arg = callInst->getArgOperand(0);
        if (ConstantInt * constInt = dyn_cast<ConstantInt>(arg))
        {
          unsigned int dimIdx = constInt->getSExtValue();
          dimIdx = (MAX_DIMENSIONS-1 < dimIdx) ? MAX_DIMENSIONS-1 : dimIdx; 
          maxDim = (maxDim < dimIdx + 1) ? dimIdx+1 : maxDim;
        }

        /*-------------------------------------------------------------
        * if the work group function has a variable argument, then 
        * assume worst case and return 3 loop levels are needed.
        *------------------------------------------------------------*/
        else 
        {
          maxDim = 3;
          break;
        }
      }
    }

  llvm::Module *M = F.getParent();
  llvm::Type *Int32 = IntegerType::get(M->getContext(), 32);
  if (getReqdWGSize(F, wgsizes))
  {
    lsizeX = ConstantInt::get(Int32, wgsizes[0]);
    lsizeY = ConstantInt::get(Int32, wgsizes[1]);
    lsizeZ = ConstantInt::get(Int32, wgsizes[2]);
  }
  else
  {
    FunctionType *ft = FunctionType::get
          (/*Result=*/   Int32,
           /*Params=*/   Int32,
           /*isVarArg=*/ false);
    Function *f_localsize =
          dyn_cast<Function>(M->getOrInsertFunction("get_local_size", ft));
    SmallVector<Value *, 4> argsx, argsy, argsz;
    argsx.push_back(ConstantInt::get(Int32, 0));
    lsizeX = CallInst::Create(f_localsize, ArrayRef<Value *>(argsx));
    if (maxDim > 1)
    {
      argsy.push_back(ConstantInt::get(Int32, 1));
      lsizeY = CallInst::Create(f_localsize, ArrayRef<Value *>(argsy));
    }
    if (maxDim > 2)
    {
      argsz.push_back(ConstantInt::get(Int32, 2));
      lsizeZ = CallInst::Create(f_localsize, ArrayRef<Value *>(argsz));
    }
  }
}

bool
WorkitemLoops::ProcessFunction(Function &F)
{
  Kernel *K = cast<Kernel> (&F);
  Initialize(K);

#if 0  // TODO: do something for reqd_work_group_size
  unsigned workItemCount = LocalSizeX*LocalSizeY*LocalSizeZ;

  if (workItemCount == 1)
    {
      K->addLocalSizeInitCode(LocalSizeX, LocalSizeY, LocalSizeZ);
      ParallelRegion::insertLocalIdInit(&F.getEntryBlock(), 0, 0, 0);
      return true;
    }
#endif

  di_function = getDebugInfo(F, function_scope_line);
  FindKernelDim(F);

  original_parallel_regions =
    K->getParallelRegions(LI);

#ifdef DUMP_CFGS
  F.dump();
  dumpCFG(F, F.getName().str() + "_before_wiloops.dot", 
          original_parallel_regions);
#endif

  IRBuilder<> builder(F.getEntryBlock().getFirstInsertionPt());

#if 0 // pocl original
  localIdXFirstVar = 
    builder.CreateAlloca
    (IntegerType::get(F.getContext(), size_t_width), 0, ".pocl.local_id_x_init");
#endif

  //  F.viewCFGOnly();

#if 0
  std::cerr << "### Original" << std::endl;
  F.viewCFGOnly();
#endif

#if 0
  for (ParallelRegion::ParallelRegionVector::iterator
           i = original_parallel_regions->begin(), 
           e = original_parallel_regions->end();
       i != e; ++i) 
  {
    ParallelRegion *region = (*i);
    region->InjectRegionPrintF();
    region->InjectVariablePrintouts();
  }
#endif

  /* TI: Hoist uniform instrs to the entry block */
  hoistWGInvariantInstrToEntry(&F);

  /* TI: Find intra-region and cross-region allocas. Do not
     privatize intra-region allocas. */
  findIntraRegionAllocas(&F);

  /* Count how many parallel regions share each entry node to
     detect diverging regions that need to be peeled. */
  std::map<llvm::BasicBlock*, int> entryCounts;

  localizeGetLocalId(&F);

  for (ParallelRegion::ParallelRegionVector::iterator
           i = original_parallel_regions->begin(), 
           e = original_parallel_regions->end();
       i != e; ++i) 
  {
    ParallelRegion *region = (*i);
#ifdef DEBUG_WORK_ITEM_LOOPS
    std::cerr << "### Adding context save/restore for PR: ";
    region->dumpNames();    
#endif
    FixMultiRegionVariables(region);
    entryCounts[region->entryBB()]++;
  }

#if 0
  std::cerr << "### After context code addition:" << std::endl;
  F.viewCFG();
#endif
  std::map<ParallelRegion*, bool> peeledRegion;
  for (ParallelRegion::ParallelRegionVector::iterator
           i = original_parallel_regions->begin(), 
           e = original_parallel_regions->end();
       i != e;  ++i) 
  {

    llvm::ValueToValueMapTy reference_map;
    ParallelRegion *original = (*i);

#ifdef DEBUG_WORK_ITEM_LOOPS
    std::cerr << "### handling region:" << std::endl;
    original->dumpNames();    
    //F.viewCFGOnly();
#endif

    /* In case of conditional barriers, the first iteration
       has to be peeled so we know which branch to execute
       with the work item loop. In case there are more than one
       parallel region sharing an entry BB, it's a diverging
       region.

       Post dominance of entry by exit does not work in case the
       region is inside a loop and the exit block is in the path
       towards the loop exit (and the function exit).
    */
    bool peelFirst =  entryCounts[original->entryBB()] > 1;

    // if not peelFirst, skip adding loops for empty region
    if (! peelFirst)
    {
      bool emptyRegion = true;
      for (ParallelRegion::iterator BI = original->begin(),
                                    BE = original->end(); BI != BE; ++BI)
      {
        if (dyn_cast<TerminatorInst>(&(*BI)->front())) continue;
        emptyRegion = false;
        break;
      }
      if (emptyRegion)  continue;
    }
    
    peeledRegion[original] = peelFirst;

    std::pair<llvm::BasicBlock *, llvm::BasicBlock *> l;
    // the original predecessor nodes of which successor
    // should be fixed if not peeling
    BasicBlockVector preds;
    ParallelRegion *loopRegion = original;

    if (peelFirst) 
      {
#ifdef DEBUG_WORK_ITEM_LOOPS
        std::cerr << "### conditional region, peeling the first iteration" << std::endl;
#endif
        ParallelRegion *replica = 
          original->replicate(reference_map, ".peeled_wi");
        replica->chainAfter(original);    
        replica->purge();
        replica->LocalizeIDLoads();
        
        l = std::make_pair(replica->entryBB(), replica->exitBB());
        loopRegion = replica;
      }
    else
      {
        llvm::pred_iterator PI = 
          llvm::pred_begin(original->entryBB()), 
          E = llvm::pred_end(original->entryBB());

        for (; PI != E; ++PI)
          {
            llvm::BasicBlock *bb = *PI;
            if (DT->dominates(original->entryBB(), bb) &&
                (RegionOfBlock(original->entryBB()) == 
                 RegionOfBlock(bb)))
              continue;
            preds.push_back(bb);
          }

#if 0
        int unrollCount;
        if (getenv("POCL_WILOOPS_MAX_UNROLL_COUNT") != NULL)
            unrollCount = atoi(getenv("POCL_WILOOPS_MAX_UNROLL_COUNT"));
        else
            unrollCount = 1;
        /* Find a two's exponent unroll count, if available. */
        while (unrollCount >= 1)
          {
            if (LocalSizeX % unrollCount == 0 &&
                unrollCount <= LocalSizeX)
              {
                break;
              }
            unrollCount /= 2;
          }

        if (unrollCount > 1) {
            ParallelRegion *prev = original;
            llvm::BasicBlock *lastBB = 
                AppendIncBlock(original->exitBB(), localIdX);
            original->AddBlockAfter(lastBB, original->exitBB());
            original->SetExitBB(lastBB);

            if (AddWIMetadata)
                original->AddIDMetadata(F.getContext(), 0);

            for (int c = 1; c < unrollCount; ++c) 
            {
                ParallelRegion *unrolled = 
                    original->replicate(reference_map, ".unrolled_wi");
                unrolled->chainAfter(prev);
                prev = unrolled;
                lastBB = unrolled->exitBB();
                if (AddWIMetadata)
                    unrolled->AddIDMetadata(F.getContext(), c);
            }
            unrolled = true;
            l = std::make_pair(original->entryBB(), lastBB);
        } else {
            l = std::make_pair(original->entryBB(), original->exitBB());
        }
#else
        l = std::make_pair(original->entryBB(), original->exitBB());
#endif 
      }

    if (di_function != NULL)
    {
      region_begin_line = findFirstDebugLine(l.first);
      if (region_begin_line != 0 && region_begin_line < function_scope_line)
        region_begin_line = function_scope_line;
      region_end_line = findLastDebugLine(l.second);
    }

#if 0 // pocl original
    l = CreateLoopAround(*original, l.first, l.second, peelFirst, localIdX,
                         LocalSizeX, true, lsizeX);
    if (maxDim > 1)
      l = CreateLoopAround(*original, l.first, l.second, false, localIdY,
                           LocalSizeY, true, lsizeY);
    if (maxDim > 2)
      l = CreateLoopAround(*original, l.first, l.second, false, localIdZ,
                           LocalSizeZ, true, lsizeZ);
#else // TI
    llvm::Type *Int32 = IntegerType::get(F.getContext(), 32);
    Value *zero = ConstantInt::get(Int32, 0);
    phiDim[0] = phiDim[1] = phiDim[2] = zero;
    phiXFirst[0] = phiXFirst[1] = phiXFirst[2] = NULL;
    if (peelFirst)
    {
      if (maxDim > 1 && wgsizes[1] != 1) phiXFirst[1] = PHINode::Create(Int32, 2);
      if (maxDim > 2 && wgsizes[2] != 1) phiXFirst[2] = PHINode::Create(Int32, 2);
    }

    bool regLocals = true;
    for (ParallelRegion::iterator BI = original->begin(),
                                  BE = original->end(); BI != BE; ++BI)
    {
      if (canLocalsFitInReg(*BI))  continue;
      regLocals = false;
      break;
    }

    l = CreateLoopAround(*loopRegion, l.first, l.second, peelFirst, localIdX,
                         LocalSizeX, 0, regLocals, false, lsizeX);
    if (maxDim > 1 && wgsizes[1] != 1)
      l = CreateLoopAround(*loopRegion, l.first, l.second, false, localIdY,
                           LocalSizeY, 1, regLocals, false, lsizeY);
    if (maxDim > 2 && wgsizes[2] != 1)
      l = CreateLoopAround(*loopRegion, l.first, l.second, false, localIdZ,
                           LocalSizeZ, 2, regLocals, false, lsizeZ);

    replaceGetLocalIdWithPhi(loopRegion, phiDim);

    Value *phiZeros[3];
    phiZeros[0] = phiZeros[1] = phiZeros[2] = zero;
    if (peelFirst)  replaceGetLocalIdWithPhi(original, phiZeros);
#endif

    /* Loop edges coming from another region mean B-loops which means 
       we have to fix the loop edge to jump to the beginning of the wi-loop 
       structure, not its body. This has to be done only for non-peeled
       blocks as the semantics is correct in the other case (the jump is
       to the beginning of the peeled iteration). */
    if (!peelFirst)
      {
        for (BasicBlockVector::iterator i = preds.begin();
             i != preds.end(); ++i)
          {
            llvm::BasicBlock *bb = *i;
            bb->getTerminator()->replaceUsesOfWith
              (original->entryBB(), l.first);
          }
      }
  }

#if 0 // pocl original
  // for the peeled regions we need to add a prologue
  // that initializes the local ids and the first iteration
  // counter
  for (ParallelRegion::ParallelRegionVector::iterator
           i = original_parallel_regions->begin(), 
           e = original_parallel_regions->end();
       i != e; ++i) 
  {
    ParallelRegion *pr = (*i);

    if (!peeledRegion[pr]) continue;
    pr->insertPrologue(0, 0, 0);
    builder.SetInsertPoint(pr->entryBB()->getFirstInsertionPt());
    builder.CreateStore
      (ConstantInt::get(IntegerType::get(F.getContext(), size_t_width), 1), 
       localIdXFirstVar);       
  }
#endif

  // Creating lsize* values have been hoisted up
  // K->addLocalSizeInitCode(LocalSizeX, LocalSizeY, LocalSizeZ);
  llvm::Instruction *inspt = F.getEntryBlock().getFirstNonPHI();
  if (Instruction *getsizeX = llvm::dyn_cast<Instruction>(lsizeX))
    inspt->getParent()->getInstList().insert(inspt, getsizeX);
  if (maxDim > 1)
    if (Instruction *getsizeY = llvm::dyn_cast<Instruction>(lsizeY))
      inspt->getParent()->getInstList().insert(inspt, getsizeY);
  if (maxDim > 2)
    if (Instruction *getsizeZ = llvm::dyn_cast<Instruction>(lsizeZ))
      inspt->getParent()->getInstList().insert(inspt, getsizeZ);

  // pocl original ParallelRegion::insertLocalIdInit(&F.getEntryBlock(), 0, 0, 0);

#if 0
  F.viewCFG();
#endif

  return true;
}

/*
 * Add context save/restore code to variables that are defined in 
 * the given region and are used outside the region.
 *
 * Each such variable gets a slot in the stack frame. The variable
 * is restored from the stack whenever it's used.
 *
 */
void
WorkitemLoops::FixMultiRegionVariables(ParallelRegion *region)
{
  InstructionIndex instructionsInRegion;
  InstructionVec instructionsToFix;
  InstructionVec rematerialize_list;

  /* Construct an index of the region's instructions so it's
     fast to figure out if the variable uses are all
     in the region. */
  for (BasicBlockVector::iterator i = region->begin();
       i != region->end(); ++i)
    {
      llvm::BasicBlock *bb = *i;
      for (llvm::BasicBlock::iterator instr = bb->begin();
           instr != bb->end(); ++instr) 
        {
          llvm::Instruction *instruction = instr;
          instructionsInRegion.insert(instruction);
        }
    }

  /* Find all the instructions that define new values and
     check if they need to be context saved. */
  for (BasicBlockVector::iterator i = region->begin();
       i != region->end(); ++i)
    {
      llvm::BasicBlock *bb = *i;
      for (llvm::BasicBlock::iterator instr = bb->begin();
           instr != bb->end(); ++instr) 
        {
          llvm::Instruction *instruction = instr;

          if (ShouldNotBeContextSaved(instr)) continue;

          /* TI: do not privatize intra-region allocas */
          if (AllocaInst *alloca = dyn_cast<AllocaInst>(instruction))
            if (intraRegionAllocas.find(alloca) != intraRegionAllocas.end())
              continue;

          for (Instruction::use_iterator ui = instruction->use_begin(),
                 ue = instruction->use_end();
               ui != ue; ++ui) 
            {
              Instruction *user;
              if ((user = dyn_cast<Instruction> (*ui)) == NULL) continue;
              // If the instruction is used outside this region inside another
              // region (not in a regionless BB like the B-loop construct BBs),
              // need to context save it.
              // Allocas (private arrays) should be privatized always. Otherwise
              // we end up reading the same array, but replicating the GEP to that.
              if (isa<AllocaInst>(instruction) || 
                  (instructionsInRegion.find(user) == instructionsInRegion.end() &&
                   RegionOfBlock(user->getParent()) != NULL))
                {
                  /* TI: rematerialize values vary only with get_local_id() */
                  if (   !isa<AllocaInst>(instruction)
                      && varyOnlyWithLocalId(instruction, 0))
                  {
                    rematerialize_list.push_back(instruction);
                    break;
                  }

                  instructionsToFix.push_back(instruction);
                  break;
                }
            }
        }
    }  

  for (InstructionVec::iterator I = rematerialize_list.begin(),
                                E = rematerialize_list.end(); I != E; ++I)
  {
    InstructionVec user_list;
    for (Instruction::use_iterator ui = (*I)->use_begin(),
                                   ue = (*I)->use_end(); ui != ue; ++ui)
    {
      Instruction *user;
      if ((user = dyn_cast<Instruction> (*ui)) == NULL) continue;
      if (instructionsInRegion.find(user) == instructionsInRegion.end() &&
          RegionOfBlock(user->getParent()) != NULL)
        user_list.push_back(user);
    }

    for (InstructionVec::iterator UI = user_list.begin(),
                                  UE = user_list.end(); UI != UE; ++UI)
      rematerializeUse(*I, *UI);
  }

  /* Finally, fix the instructions. */
  for (InstructionVec::iterator i = instructionsToFix.begin();
       i != instructionsToFix.end(); ++i)
    {
#ifdef DEBUG_WORK_ITEM_LOOPS
      std::cerr << "### adding context/save restore for" << std::endl;
      (*i)->dump();
#endif 
      llvm::Instruction *instructionToFix = *i;
      AddContextSaveRestore(instructionToFix);
    }
}

llvm::Instruction *
WorkitemLoops::AddContextSave
(llvm::Instruction *instruction, llvm::Instruction *alloca)
{

  if (isa<AllocaInst>(instruction))
    {
      /* If the variable to be context saved is itself an alloca,
         we have created one big alloca that stores the data of all the 
         work-items and return pointers to that array. Thus, we need
         no initialization code other than the context data alloca itself. */
      return NULL;
    }

  /* Save the produced variable to the array. */
  BasicBlock::iterator definition = dyn_cast<Instruction>(instruction);

  ++definition;
  while (isa<PHINode>(definition)) ++definition;

  IRBuilder<> builder(definition); 
  std::vector<llvm::Value *> gepArgs;

  /* Reuse the id loads earlier in the region, if possible, to
     avoid messy output with lots of redundant loads. */
  ParallelRegion *region = RegionOfBlock(instruction->getParent());
  assert ("Adding context save outside any region produces illegal code." && 
          region != NULL);

// linearize index computation for store into alloca
// alloca[idz * sizey*sizex + idy * sizex + idx]
  Function *F = instruction->getParent()->getParent();
  Value *linear_index = genLinearIndex(builder, F);
  gepArgs.push_back(linear_index);

  return builder.CreateStore(instruction, builder.CreateGEP(alloca, gepArgs));
}

llvm::Instruction *
WorkitemLoops::AddContextRestore
(llvm::Value *val, llvm::Instruction *alloca, llvm::Instruction *before, 
 bool isAlloca)
{
  assert (val != NULL);
  assert (alloca != NULL);
  IRBuilder<> builder(alloca);
  if (before != NULL) 
    {
      builder.SetInsertPoint(before);
    }
  else if (isa<Instruction>(val))
    {
      builder.SetInsertPoint(dyn_cast<Instruction>(val));
      before = dyn_cast<Instruction>(val);
    }
  else 
    {
      assert (false && "Unknown context restore location!");
    }

  
  std::vector<llvm::Value *> gepArgs;

  /* Reuse the id loads earlier in the region, if possible, to
     avoid messy output with lots of redundant loads. */
  ParallelRegion *region = RegionOfBlock(before->getParent());
  assert ("Adding context save outside any region produces illegal code." && 
          region != NULL);

// linearize alloca loads
//       idz * _local_size_x * _local_size_y + idy * _local_size_x + idx
  Function *F = before->getParent()->getParent();
  Value *linear_index = genLinearIndex(builder, F);
  gepArgs.push_back(linear_index);

  llvm::Instruction *gep = 
    dyn_cast<Instruction>(builder.CreateGEP(alloca, gepArgs));
  if (isAlloca) {
    /* In case the context saved instruction was an alloca, we created a
       context array with pointed-to elements, and now want to return a pointer 
       to the elements to emulate the original alloca. */
    return gep;
  }           
  return builder.CreateLoad(gep);
}

/**
 * Returns the context array (alloca) for the given Value, creates it if not
 * found.
 */
llvm::Instruction *
WorkitemLoops::GetContextArray(llvm::Instruction *instruction)
{
  
  /*
   * Unnamed temp instructions need a generated name for the
   * context array. Create one using a running integer.
   */
  std::ostringstream var;
  var << ".";

  if (std::string(instruction->getName().str()) != "")
    {
      var << instruction->getName().str();
    }
  else if (tempInstructionIds.find(instruction) != tempInstructionIds.end())
    {
      var << tempInstructionIds[instruction];
    }
  else
    {
      tempInstructionIds[instruction] = tempInstructionIndex++;
      var << tempInstructionIds[instruction];
    }

  var << ".pocl_context";
  std::string varName = var.str();

  if (contextArrays.find(varName) != contextArrays.end())
    return contextArrays[varName];

  IRBuilder<> builder(instruction->getParent()->getParent()->getEntryBlock().getFirstInsertionPt());

  llvm::Type *elementType;
  if (isa<AllocaInst>(instruction))
    {
      /* If the variable to be context saved was itself an alloca,
         create one big alloca that stores the data of all the 
         work-items and directly return pointers to that array.
         This enables moving all the allocas to the entry node without
         breaking the parallel loop.
         Otherwise we would rely on a dynamic alloca to allocate 
         unique stack space to all the work-items when its wiloop
         iteration is executed. */
      elementType = 
        dyn_cast<AllocaInst>(instruction)->getType()->getElementType();
    } 
  else 
    {
      elementType = instruction->getType();
    }

// parameterize alloca to be based on _local_size_{x,y,z}
  llvm::Value *wgsize = lsizeX;
  if (maxDim > 1) wgsize = builder.CreateMul(wgsize, lsizeY);
  if (maxDim > 2) wgsize = builder.CreateMul(wgsize, lsizeZ);

// Change boolean i1 type to i32 type, TI used 4-byte int for boolean!!!
// TODO: what if boolean is in a struct???
  llvm::Type *Int1  = llvm::Type::getInt1Ty (instruction->getContext());
  llvm::Type *Int32 = llvm::Type::getInt32Ty(instruction->getContext());
  if (elementType == Int1 || (elementType->isArrayTy() &&
                              elementType->getArrayElementType() == Int1))
    wgsize = builder.CreateMul(wgsize, ConstantInt::get(Int32, 4));

  llvm::Instruction *alloca =
    builder.CreateAlloca(elementType, wgsize, varName);

  contextArrays[varName] = alloca;
  return alloca;
}


/**
 * Adds context save/restore code for the value produced by the
 * given instruction.
 *
 * TODO: add only one restore per variable per region.
 * TODO: add only one load of the id variables per region. 
 * Could be done by having a context restore BB in the beginning of the
 * region and a context save BB at the end.
 * TODO: ignore work group variables completely (the iteration variables)
 * The LLVM should optimize these away but it would improve
 * the readability of the output during debugging.
 * TODO: rematerialize some values such as extended values of global 
 * variables (especially global id which is computed from local id) or kernel 
 * argument values instead of allocating stack space for them
 */
void
WorkitemLoops::AddContextSaveRestore
(llvm::Instruction *instruction) {

  /* Allocate the context data array for the variable. */
  llvm::Instruction *alloca = GetContextArray(instruction);
  llvm::Instruction *theStore = AddContextSave(instruction, alloca);

  InstructionVec uses;
  /* Restore the produced variable before each use to ensure the correct context
     copy is used.
     
     We could add the restore only to other regions outside the 
     variable defining region and use the original variable in the defining
     region due to the SSA virtual registers being unique. However,
     alloca variables can be redefined also in the same region, thus we 
     need to ensure the correct alloca context position is written, not
     the original unreplicated one. These variables can be generated by
     volatile variables, private arrays, and due to the PHIs to allocas
     pass.
  */

  /* Find out the uses to fix first as fixing them invalidates
     the iterator. */
  for (Instruction::use_iterator ui = instruction->use_begin(),
         ue = instruction->use_end();
       ui != ue; ++ui) 
    {
      Instruction *user;
      if ((user = dyn_cast<Instruction> (*ui)) == NULL) continue;
      if (user == theStore) continue;
      uses.push_back(user);
    }

  for (InstructionVec::iterator i = uses.begin(); i != uses.end(); ++i)
    {
      Instruction *user = *i;
      Instruction *contextRestoreLocation = user;
      /* If the user is in a block that doesn't belong to a region,
         the variable itself must be a "work group variable", that is,
         not dependent on the work item. Most likely an iteration
         variable of a for loop with a barrier. */
      if (RegionOfBlock(user->getParent()) == NULL) continue;

      PHINode* phi = dyn_cast<PHINode>(user);
      if (phi != NULL)
        {
          /* In case of PHI nodes, we cannot just insert the context 
             restore code before it in the same basic block because it is 
             assumed there are no non-phi Instructions before PHIs which 
             the context restore code constitutes to. Add the context
             restore to the incomingBB instead.

             There can be values in the PHINode that are incoming
             from another region even though the decision BB is within the region. 
             For those values we need to add the context restore code in the 
             incoming BB (which is known to be inside the region due to the
             assumption of not having to touch PHI nodes in PRentry BBs).
          */          

          /* PHINodes at region entries are broken down earlier. */
          assert ("Cannot add context restore for a PHI node at the region entry!" &&
                  RegionOfBlock(phi->getParent())->entryBB() != phi->getParent());
#ifdef DEBUG_WORK_ITEM_LOOPS
          std::cerr << "### adding context restore code before PHI" << std::endl;
          user->dump();
          std::cerr << "### in BB:" << std::endl;
          user->getParent()->dump();
#endif
          BasicBlock *incomingBB = NULL;
          for (unsigned incoming = 0; incoming < phi->getNumIncomingValues(); 
               ++incoming)
            {
              Value *val = phi->getIncomingValue(incoming);
              BasicBlock *bb = phi->getIncomingBlock(incoming);
              if (val == instruction) incomingBB = bb;
            }
          assert (incomingBB != NULL);
          contextRestoreLocation = incomingBB->getTerminator();
        }
      llvm::Value *loadedValue = 
        AddContextRestore
        (user, alloca, contextRestoreLocation, isa<AllocaInst>(instruction));
      user->replaceUsesOfWith(instruction, loadedValue);
#ifdef DEBUG_WORK_ITEM_LOOPS
      std::cerr << "### done, the user was converted to:" << std::endl;
      user->dump();
#endif
    }
}

bool
WorkitemLoops::ShouldNotBeContextSaved(llvm::Instruction *instr)
{
    /*
      _local_id loads should not be replicated as it leads to
      problems in conditional branch case where the header node
      of the region is shared across the branches and thus the
      header node's ID loads might get context saved which leads
      to egg-chicken problems. 
    */
  if (isa<BranchInst>(instr)) return true;

    llvm::LoadInst *load = dyn_cast<llvm::LoadInst>(instr);
    if (load != NULL &&
        (load->getPointerOperand() == localIdZ ||
         load->getPointerOperand() == localIdY ||
         load->getPointerOperand() == localIdX))
      return true;

#if 0 // TI
    VariableUniformityAnalysis &VUA = 
      getAnalysis<VariableUniformityAnalysis>();
#endif

    /* In case of uniform variables (same for all work-items),
       there is no point to create a context array slot for them,
       but just use the original value everywhere. 

       Allocas are problematic: they include the de-phi induction
       variables of the b-loops. In those case each work item 
       has a separate loop iteration variable in the LLVM IR but
       which is really a parallel region loop invariant. But
       because we cannot separate such loop invariant variables
       at this point sensibly, let's just replicate the iteration
       variable to each work item and hope the latter optimizations
       reduce them back to a single induction variable outside the
       parallel loop.   
    */
    if (!VUA->shouldBePrivatized(instr->getParent()->getParent(), instr)) {
#ifdef DEBUG_WORK_ITEM_LOOPS
      std::cerr << "### based on VUA, not context saving:";
      instr->dump();
#endif     
      return true;
    } 

    return false;
}

llvm::BasicBlock *
WorkitemLoops::AppendIncBlock
(llvm::BasicBlock* after, llvm::Value *localIdVar)
{
  llvm::LLVMContext &C = after->getContext();

  llvm::BasicBlock *oldExit = after->getTerminator()->getSuccessor(0);
  assert (oldExit != NULL);

  llvm::BasicBlock *forIncBB = 
    BasicBlock::Create(C, "pregion_for_inc", after->getParent());

  after->getTerminator()->replaceUsesOfWith(oldExit, forIncBB);

  IRBuilder<> builder(oldExit);

  builder.SetInsertPoint(forIncBB);
  /* Create the iteration variable increment */
  builder.CreateStore
    (builder.CreateAdd
     (builder.CreateLoad(localIdVar),
      ConstantInt::get(IntegerType::get(C, size_t_width), 1)),
     localIdVar);

  builder.CreateBr(oldExit);

  return forIncBB;
}

void
WorkitemLoops::replaceGetLocalIdWithPhi(ParallelRegion *region, Value* phis[3])
{
  CallInst *call;
  std::list<CallInst *> localid_calls;
  for (ParallelRegion::iterator i = region->begin(),
                                e = region->end(); i != e; ++i)
  {
    BasicBlock* bb = *i;
    for (BasicBlock::iterator ii = bb->begin(), ee = bb->end();
         ii != ee; ii++)
    {
      if ((call = dyn_cast<CallInst>(ii)) == NULL) continue;
      if (call->getCalledFunction() == NULL)       continue;
      std::string name(call->getCalledFunction()->getName());
      if (name == "get_local_id") localid_calls.push_back(call);
    }
  }
  if (localid_calls.empty()) return;

  Type *Int32 = IntegerType::get(localid_calls.front()->getContext(), 32);
  Value *zero = ConstantInt::get(Int32, 0);

  for (std::list<CallInst *>::iterator I = localid_calls.begin(),
                                       E = localid_calls.end(); I != E; ++I)
  {
    call = *I;
    Value *localid_val;
    Value *arg = call->getArgOperand(0);
    if (ConstantInt * constInt = dyn_cast<ConstantInt>(arg))
    {
      int dim = constInt->getSExtValue();
      if (dim >= 0 && dim <= 2) localid_val = phis[dim];
      else                      localid_val = zero;
    }
    else
    {
      // not constant arg: return (arg == 2) ? ivz : (arg == 1 ? ivy : ivx)
      Value *one  = ConstantInt::get(Int32, 1);
      Value *two  = ConstantInt::get(Int32, 2);
      Value *cyx  = new ICmpInst(call, ICmpInst::ICMP_EQ, arg, one);
      Value *syx  = SelectInst::Create(cyx, phis[1], phis[0], "", call);
      Value *czyx = new ICmpInst(call, ICmpInst::ICMP_EQ, arg, two);
      localid_val = SelectInst::Create(czyx, phis[2], syx);
    }

    BasicBlock::iterator BI(call);
    ReplaceInstWithValue(call->getParent()->getInstList(), BI, localid_val);
  }
}

Value *
WorkitemLoops::genLinearIndex(IRBuilder<> &builder, Function *F)
{
  llvm::Module *M = F->getParent();
  llvm::Type *Int32 = IntegerType::get(M->getContext(), 32);
  FunctionType *ft = FunctionType::get
        (/*Result=*/   Int32,
         /*Params=*/   Int32,
         /*isVarArg=*/ false);
  Function *f_localid =
        dyn_cast<Function>(M->getOrInsertFunction("get_local_id", ft));
  Value *lidX = builder.CreateCall(f_localid, ConstantInt::get(Int32, 0));
  Value *lidY=NULL, *lidZ=NULL;

  if (maxDim > 1)
    lidY = builder.CreateCall(f_localid, ConstantInt::get(Int32, 1));
  if (maxDim > 2)
    lidZ = builder.CreateCall(f_localid, ConstantInt::get(Int32, 2));

  llvm::Value *linear_index = lidX;
  if (maxDim > 1)
    linear_index = builder.CreateAdd(linear_index,
                              builder.CreateMul(lidY, lsizeX) );
  if (maxDim > 2)
    linear_index = builder.CreateAdd(linear_index,
                              builder.CreateMul(lidZ,
                                        builder.CreateMul(lsizeY, lsizeX)) );
  return linear_index;
}

/*****************************************************************************
* Remove Barrier() calls after inserting loops
*****************************************************************************/
bool
WorkitemLoops::removeBarrierCalls(Function *F)
{
  std::vector<CallInst *> call_list;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    if (CallInst *callInst = dyn_cast<CallInst>(&*I))
    {
      if (!callInst->getCalledFunction()) continue;
      std::string functionName(callInst->getCalledFunction()->getName());
      if (functionName == "barrier")  call_list.push_back(callInst);
    }
  if (call_list.empty()) return false;

  std::vector<CallInst *>::iterator I, E;
  for (I = call_list.begin(), E = call_list.end(); I != E; ++I)
    (*I)->eraseFromParent();

  return true;
}

/*****************************************************************************
* Recreate get_local_id() at its uses, so that they get correct loop
* index variable in each parallel region without privatization
* Do not propagate if def and use are in same block
*****************************************************************************/
void
WorkitemLoops::localizeGetLocalId(Function *F)
{
  std::list<CallInst *> worklist;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    if (CallInst * call = dyn_cast<CallInst>(&*I))
    {
      if (! call->getCalledFunction()) continue;
      std::string name(call->getCalledFunction()->getName());
      if (name == "get_local_id")
        if (dyn_cast<ConstantInt>(call->getArgOperand(0)))
          worklist.push_back(call);
    }

  if (worklist.empty()) return;

  for (std::list<CallInst *>::iterator I = worklist.begin(),
                                       E = worklist.end(); I != E; ++I)
  {
    CallInst *call = *I;
    BasicBlock *B = call->getParent();
    ParallelRegion *region = RegionOfBlock(call->getParent());
    std::list<Instruction *> replist;

    for (Value::use_iterator UI = call->use_begin(),
                             UE = call->use_end(); UI != UE; ++UI)
    {
      if (Instruction *inst = dyn_cast<Instruction>(*UI))
      {
        if (   inst->getParent() == B
            || RegionOfBlock(inst->getParent()) == region) continue;
        replist.push_back(inst);
      }
    }

    for (std::list<Instruction *>::iterator RI = replist.begin(),
                                            RE = replist.end(); RI != RE; ++RI)
    {
        Instruction *inst = *RI;
        Instruction *dup = call->clone();
        dup->insertBefore(inst);
        inst->replaceUsesOfWith(call, dup);
    }
  }
}

/*****************************************************************************
* varyOnlyWithLocalId(), check the components that value is derived from,
* such that the compenent is either uniform value, or get_local_id()
* TODO: hoist uniform values to entry block
*****************************************************************************/
#define MAX_PROFITABLE_DEPTH 3
bool
WorkitemLoops::varyOnlyWithLocalId(Instruction *instr, int depth)
{
  if (depth > MAX_PROFITABLE_DEPTH)  return false;

  if (   isa<TerminatorInst>(instr)
      || isa<PHINode>(instr)
      || isa<LoadInst>(instr)
      || isa<StoreInst>(instr)
      || isa<LandingPadInst>(instr)
      || isa<AtomicCmpXchgInst>(instr)
      || isa<AtomicRMWInst>(instr)
      || isa<FenceInst>(instr)
      || isa<ExtractElementInst>(instr)
      || isa<InsertElementInst>(instr)
      || isa<ExtractValueInst>(instr)
      || isa<InsertValueInst>(instr)
      || isa<VAArgInst>(instr)
     )  return false;

  // Privatized or not, alloca is already handled
  if (isa<AllocaInst>(instr))  return true;

  if (CallInst *callInst = dyn_cast<CallInst>(instr))
  {
    if (!callInst->getCalledFunction()) return false;
    std::string functionName(callInst->getCalledFunction()->getName());
    return (functionName == "get_local_id") ? true : false;
  }

  //Function *F = instr->getParent()->getParent();
  for (unsigned int op = 0; op < instr->getNumOperands(); ++op)
  {
    Value *op_val = instr->getOperand(op);
    // if (VUA->isUniform(F, op_val)) continue;
    if (isWGInvariant(op_val)) continue;

    // TODO: dynamic programming can be applied here
    if (Instruction *op_instr = dyn_cast<Instruction>(op_val))
    {
      if (! varyOnlyWithLocalId(op_instr, depth+1))  return false;
    }
    else  return false;
  }

  return true;
}

/*****************************************************************************
* rematerializeUse()
* clone instr and all its non-uniform components, replace its use in user
*****************************************************************************/
void
WorkitemLoops::rematerializeUse(Instruction *instr, Instruction *user)
{
  Instruction *dup = instr->clone();
  dup->insertBefore(user);
  user->replaceUsesOfWith(instr, dup);

  //Function *F = instr->getParent()->getParent();
  for (unsigned int op = 0; op < dup->getNumOperands(); ++op)
  {
    Value *op_val = dup->getOperand(op);
    // if (VUA->isUniform(F, op_val)) continue;
    if (isWGInvariant(op_val)) continue;
    if (isa<AllocaInst>(op_val)) continue;

    // TODO: dynamic programming can be applied here
    if (Instruction *op_instr = dyn_cast<Instruction>(instr->getOperand(op)))
      rematerializeUse(op_instr, dup);
  }
}

/*****************************************************************************
* findIntraRegionAllocas()
* Find allocas whose users are either all inside one region or none inside
* Process:
* 1. Collect list of allocas
* 2. for each alloca
* 2.0 if alloca is event_t type, remember in do-not-privatize-index
* 2.1 collect list of user blocks
* 2.2 for each region
* 2.2.1 check either all user blocks are in region or none are in region
* 2.3 if intra-region, remember in do-not-privatize-index,
*     and set isUniform to false so that loads from it can be checked
*****************************************************************************/
// #define OPENCL_EVENT_T_TYPE_NAME  "%opencl.event_t"
#define OPENCL_EVENT_T_TYPE_NAME  "__ocl_event"
void
WorkitemLoops::findIntraRegionAllocas(llvm::Function *F)
{
  std::list<AllocaInst *> worklist;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    if (AllocaInst * alloca = dyn_cast<AllocaInst>(&*I))
      worklist.push_back(alloca);

  for (std::list<AllocaInst *>::iterator I = worklist.begin(),
                                         E = worklist.end(); I != E; ++I)
  {
    AllocaInst *alloca = *I;

    if (Type *allocated_type = alloca->getAllocatedType())
    {
      std::string type_name;
      llvm::raw_string_ostream str_ostream(type_name);
      allocated_type->print(str_ostream);
      str_ostream.flush();
      if (type_name.find(OPENCL_EVENT_T_TYPE_NAME) != std::string::npos)
      {
        intraRegionAllocas.insert(alloca);
        continue;
      }
    }

    std::set<BasicBlock *> userSet;
    for (Value::use_iterator UI = alloca->use_begin(),
                             UE = alloca->use_end(); UI != UE; ++UI)
      if (Instruction *instr = dyn_cast<Instruction>(*UI))
        userSet.insert(instr->getParent());

    bool is_intra = true;
    for (ParallelRegion::ParallelRegionVector::iterator
             RI = original_parallel_regions->begin(),
             RE = original_parallel_regions->end();
         RI != RE; ++RI)
    {
      bool onein  = false;
      bool oneout = false;
      ParallelRegion *region = (*RI);
      for (std::set<BasicBlock *>::iterator BI = userSet.begin(),
                                            BE = userSet.end(); BI != BE; ++BI)
      {
        BasicBlock *B = *BI;
        if (region->HasBlock(B)) { onein  = true; if (oneout) break; }
        else                     { oneout = true; if (onein)  break; }
      }
      is_intra = (onein && !oneout) || (!onein && oneout);
      if (! is_intra) break;
    }
    if (is_intra)
    {
      intraRegionAllocas.insert(alloca);
      VUA->setUniform(F, alloca, false);
    }
  }
}

/*****************************************************************************
* hoistWGInvariantInstrToEntry()
* Hoist WG Invariant instructions to the entry block
*****************************************************************************/
void MoveWGInvariant(Instruction *instr, Instruction *insb4)
{
  if (instr->getParent() == insb4->getParent())  return;

  instr->moveBefore(insb4);
  for (unsigned int op = 0; op < instr->getNumOperands(); ++op)
  {
    if (Instruction *op_instr = dyn_cast<Instruction>(instr->getOperand(op)))
      MoveWGInvariant(op_instr, instr);
  }
}

void
WorkitemLoops::hoistWGInvariantInstrToEntry(Function *F)
{
  std::list<Instruction *> worklist;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    if (Instruction * instr = dyn_cast<Instruction>(&*I))
      if (isWGInvariant(instr))  worklist.push_back(instr);

  Instruction *insb4 = F->getEntryBlock().getTerminator();
  for (std::list<Instruction *>::iterator I = worklist.begin(),
                                          E = worklist.end(); I != E; ++I)
    MoveWGInvariant((*I), insb4);
}

bool
WorkitemLoops::isWGInvariant(Value *v)
{
  std::map<Value *, bool>::const_iterator I = WGInvariantMap.find(v);
  if (I != WGInvariantMap.end()) {
    return I->second;
  }

  if (   isa<llvm::Argument>(v)
      || isa<llvm::Constant>(v)
     )
  {
    WGInvariantMap[v] = true;
    return true;
  }

  Instruction *instr = dyn_cast<Instruction>(v);
  if (! instr)
  {
    WGInvariantMap[v] = false;
    return false;
  }

  if (   getKernelConfigGEPInstIndex(instr) >= 0
      || getKernelConfigLoadInstIndex(instr) >= 0)
  {
    WGInvariantMap[v] = true;
    return true;
  }

  if (   isa<TerminatorInst>(instr)
      || isa<PHINode>(instr)
      || isa<CallInst>(instr)
      || isa<AllocaInst>(instr)
      || isa<LoadInst>(instr)
      || isa<StoreInst>(instr)
      || isa<LandingPadInst>(instr)
      || isa<AtomicCmpXchgInst>(instr)
      || isa<AtomicRMWInst>(instr)
      || isa<FenceInst>(instr)
      || isa<ExtractElementInst>(instr)
      || isa<InsertElementInst>(instr)
      || isa<ExtractValueInst>(instr)
      || isa<InsertValueInst>(instr)
      || isa<VAArgInst>(instr)
     )
  {
    WGInvariantMap[v] = false;
    return false;
  }

  if (Instruction *instr = dyn_cast<Instruction>(v))
  {
    for (unsigned int op = 0; op < instr->getNumOperands(); ++op) {
      if (!isWGInvariant(instr->getOperand(op))) {
        WGInvariantMap[v] = false;
        return false;
      }
    }

    WGInvariantMap[v] = true;
    return true;
  }

  WGInvariantMap[v] = false;
  return false;
}

