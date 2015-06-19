/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include "wga.h"
#include <iostream>
#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IntrinsicInst.h>
#include "llvm/IR/CFG.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/IR/DebugInfo.h"
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/LoopInfo.h>
#include "../util.h"
#include "boost/assign/std/set.hpp"
#include <stdio.h>

using namespace std;
using namespace boost::assign;

namespace llvm
{

/******************************************************************************
* createTIOpenclWorkGroupAggregation
******************************************************************************/
Pass *createTIOpenclWorkGroupAggregationPass(bool is_pocl_mode) 
{
    TIOpenclWorkGroupAggregation *fp = new TIOpenclWorkGroupAggregation(
                                                                 is_pocl_mode);
    return fp;
}

/**************************************************************************
* Constructor
**************************************************************************/
TIOpenclWorkGroupAggregation::TIOpenclWorkGroupAggregation(bool pocl_mode) : 
    FunctionPass(ID), is_pocl_mode(pocl_mode), di_function(NULL)
{
    for (unsigned int i = 0; i < MAX_DIMENSIONS; ++i) IVPhi[i] = 0;
}

/**************************************************************************
* Get index variable
* 1. wga mode, only one loop nest inserted: return IVPhi[]
**************************************************************************/
llvm::Value* TIOpenclWorkGroupAggregation::get_IV(Function &F, CallInst *call)
{
    Value *arg   = call->getArgOperand(0);
    Type  *Int32 = Type::getInt32Ty(F.getContext());
    Value *zero  = ConstantInt::get(Int32, 0); 

    if (ConstantInt * constInt = dyn_cast<ConstantInt>(arg))
    {
        int32_t dim = constInt->getSExtValue();
        return (dim >= 0 && dim <= 2) ? IVPhi[dim] : zero;
    }

    llvm::Value *ivx = IVPhi[0];
    llvm::Value *ivy = IVPhi[1];
    llvm::Value *ivz = IVPhi[2];
    // not constant arg: return (arg == 2) ? ivz : (arg == 1 ? ivy : ivx)
    Value       *one = ConstantInt::get(Int32, 1); 
    Value       *two = ConstantInt::get(Int32, 2); 
    llvm::Value *cyx = new ICmpInst(call, ICmpInst::ICMP_EQ, arg, one);
    llvm::Value *syx = SelectInst::Create(cyx, ivy, ivx, "", call);
    llvm::Value *czyx = new ICmpInst(call, ICmpInst::ICMP_EQ, arg, two);
    return SelectInst::Create(czyx, ivz, syx, "", is_pocl_mode ? NULL : call);
}

/**************************************************************************
* runOnFunction(Function &F) 
**************************************************************************/
bool TIOpenclWorkGroupAggregation::runOnFunction(Function &F) 
{
    bool changed = false;

    /*-------------------------------------------------------------------------
    * Fix (u)long to float/double implicit conversions, use convert_*() BIFs
    *------------------------------------------------------------------------*/
    changed = implicit_long_conv_use_bif(F);

    /*-------------------------------------------------------------------------
    * Skip non-kernel functions
    *------------------------------------------------------------------------*/
    if (! isKernelFunction(F))  return changed;

    /*-------------------------------------------------------------------------
    * Obtain Debug Information (func scope line number) (when debug is on)
    *------------------------------------------------------------------------*/
    di_end_scope_line = 0;
    if ((di_function = getDebugInfo(F, di_scope_line_num)) != NULL)
    {
        BasicBlock *ex = findExitBlock(F);
        for (BasicBlock::iterator B = ex->begin(), E = ex->end(); B != E; ++B)
            if ((di_end_scope_line = (*B).getDebugLoc().getLine()) != 0) break;
    }

    /*-------------------------------------------------------------------------
    * Determine how many dimensions are referenced using OpenCL getXXX 
    * functions, and record them all for later rewrite.
    *------------------------------------------------------------------------*/
    int dims = 0;
    if (!is_pocl_mode)  dims = findNeededLoopNest(F);

    /*-------------------------------------------------------------------------
    * Collect mem instructions, to be added with llvm.mem.loop_parallel_access
    *------------------------------------------------------------------------*/
    if (!is_pocl_mode && dims > 0)  collect_mem_for_metadata(F);

    /*-------------------------------------------------------------------------
    * Add a loop nest for each dimension referenced that requires a workitem
    * id.
    *------------------------------------------------------------------------*/
    getReqdWGSize(F, wgsizes);
    if (!is_pocl_mode)
    {
        bool reg_locals = canLocalsFitInReg(F);
        for (int i = 0; i < dims; ++i) add_loop(F, i, reg_locals, wgsizes[i]);
    }

    /*-------------------------------------------------------------------------
    * Rewrite __first_wi references
    *------------------------------------------------------------------------*/
    if (!is_pocl_mode) changed |= rewrite_first_wi(F, dims);

    /*-------------------------------------------------------------------------
    * Add mem instructions with with llvm.mem.loop_parallel_access
    *------------------------------------------------------------------------*/
    if (!is_pocl_mode && dims > 0)  add_loop_mem_metadata(F);

    /*-------------------------------------------------------------------------
    * rewrite the alloca() generated during pocl llvm work-group aggregation
    *------------------------------------------------------------------------*/
    if (is_pocl_mode)  changed |= rewrite_allocas(F);

    /*-------------------------------------------------------------------------
    * rewrite the OpenCL getXXX dimension query functions to reference the info
    * packet for the workgroup.  Return true if we modified the function.
    *------------------------------------------------------------------------*/
    changed |= rewrite_ocl_funcs(F);

    /*-------------------------------------------------------------------------
    * Hoist work group invariant code, e.g. loads of kernel_config_l2
    *------------------------------------------------------------------------*/
    changed |= hoist_wg_invariant_code(F);

    return changed; 
}

/******************************************************************************
* getAnalysisUsage(AnalysisUsage &Info) const
******************************************************************************/
void TIOpenclWorkGroupAggregation::getAnalysisUsage(AnalysisUsage &Info) const
{
    /*-------------------------------------------------------------------------
    * This will ensure that all returns go through a single exit node, which 
    * our WGA loop generation algorithm depends on.
    *------------------------------------------------------------------------*/
    Info.addRequired<UnifyFunctionExitNodes>();
}

/******************************************************************************
* Find and replace __first_wi references with a test for the first WI.
******************************************************************************/
bool TIOpenclWorkGroupAggregation::rewrite_first_wi(Function &F, int dims)
{
    Module *M                  = F.getParent();
    llvm::Type*  Int32         = llvm::IntegerType::getInt32Ty(M->getContext());

    M->getOrInsertGlobal("__first_wi", Int32);
    GlobalVariable* __first_wi = M->getNamedGlobal("__first_wi"); 
    Value *zero                = ConstantInt::get(Int32, 0); 

    std::list<LoadInst*> first_wi_node_list;

    for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I)
        if (LoadInst * load = dyn_cast<LoadInst>(&*I))
            if (load->getPointerOperand() == __first_wi)
                first_wi_node_list.push_back(load);

    if (first_wi_node_list.empty()) return false;

    for (std::list<LoadInst *>::iterator I = first_wi_node_list.begin();
         I != first_wi_node_list.end(); ++I)
    {
         LoadInst * load = *I;
         Instruction *or_tree = BinaryOperator::Create(Instruction::Or, 
                  IVPhi[0], IVPhi[1], "", load);
         or_tree = BinaryOperator::Create(Instruction::Or, or_tree, IVPhi[2], 
                  "", load);
         Instruction *cmp = CmpInst::Create (Instruction::ICmp, 
                  CmpInst::ICMP_EQ, or_tree, zero, "", load);
         cmp = CastInst::CreateIntegerCast(cmp, Int32, false, "", load);

         load->replaceAllUsesWith(cmp);
    }

    return true;
}

/**************************************************************************
* findNeededLoopNest(Function &F) 
**************************************************************************/
unsigned int TIOpenclWorkGroupAggregation::findNeededLoopNest(Function &F) 
{
    unsigned int maxDim = 0;

    for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I)
        if (CallInst * callInst = dyn_cast<CallInst>(&*I))
        {
            if (!callInst->getCalledFunction()) continue;
            string functionName(callInst->getCalledFunction()->getName());

            if (functionName == "get_local_id" || 
                functionName == "get_global_id")
            {
                Value *arg = callInst->getArgOperand(0);
                if (ConstantInt * constInt = dyn_cast<ConstantInt>(arg))
                {
                    unsigned int dimIdx = constInt->getSExtValue();
                    dimIdx = min(MAX_DIMENSIONS-1, dimIdx); 
                    maxDim = max(maxDim, dimIdx + 1);
                }

                /*-------------------------------------------------------------
                * if the work group function has a variable argument, then 
                * assume worst case and return 3 loop levels are needed.
                *------------------------------------------------------------*/
                else return 3;
            }
        }

    return maxDim;
}

/**************************************************************************
* createLoadGlobal
*     Create an aligned 32 bit load from a global address.
**************************************************************************/
Instruction* TIOpenclWorkGroupAggregation::createLoadGlobal
    (int32_t idx, Module* M, Instruction *before, const char *name)
{
    llvm::Type     *Int32 = llvm::IntegerType::getInt32Ty(M->getContext());
    llvm::ArrayType *type = ArrayType::get(Int32, 32);

    M->getOrInsertGlobal("kernel_config_l2", type);
    GlobalVariable* global = M->getNamedGlobal("kernel_config_l2"); 

    std::vector<Value*> indices;
    indices.push_back(ConstantInt::get(Int32, 0));
    indices.push_back(ConstantInt::get(Int32, idx));

    Constant* gep = ConstantExpr::getInBoundsGetElementPtr (global, indices);
    LoadInst* ld  = new LoadInst(gep, name, before);

    ld->setAlignment(4);
    return ld;
}

/******************************************************************************
* findDim
******************************************************************************/
unsigned int TIOpenclWorkGroupAggregation::findDim(class CallInst* call)
{
    Value *arg = call->getArgOperand(0);

    if (ConstantInt * constInt = dyn_cast<ConstantInt>(arg))
        return constInt->getSExtValue();
    return 100;  // who knows
}

/**************************************************************************
* rewrite allocas to _wg_alloca(sizeinbytes)
**************************************************************************/
bool TIOpenclWorkGroupAggregation::rewrite_allocas(Function &F) 
{
    int wi_alloca_size = 0;
    Module *M = F.getParent();
    AllocaInst *alloca;

    std::vector<AllocaInst *> allocas;
    for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I)
        if ((alloca = dyn_cast<AllocaInst>(&*I)) != NULL)
            allocas.push_back(alloca);
    if (allocas.empty()) return false;

    DataLayout dataLayout(M);
    Type *Int1  = Type::getInt1Ty (M->getContext());
    Type *Int32 = Type::getInt32Ty(M->getContext());

    // initialize wg_alloca_start and wg_alloca_size
    // wg_alloca_size  = load(packetaddr+offset);
    // wg_alloca_start = load(packetaddr+offset) + __core_num()*wg_alloca_size;
    Instruction *inspt = F.getEntryBlock().getFirstNonPHI();
    FunctionType *core_num_ft = FunctionType::get(/*Result=*/   Int32,
                                                  /*isVarArg=*/ false);
    Function *core_num = dyn_cast<Function>(
                         M->getOrInsertFunction("__core_num", core_num_ft));
    Instruction *f_core_num = CallInst::Create(core_num, "", inspt);
    Instruction *wg_alloca_size = createLoadGlobal(15, M, inspt);
    Instruction *shift = BinaryOperator::Create(Instruction::Mul, f_core_num,
                                                wg_alloca_size, "", inspt);
    Instruction *start = createLoadGlobal(14, M, inspt);
    Instruction *wg_alloca_addr = BinaryOperator::Create(
                                    Instruction::Add, start, shift, "", inspt);
    if (di_function != NULL)
    {
        DebugLoc dloc = DebugLoc::get(di_scope_line_num, 0, di_function, NULL);
        f_core_num->setDebugLoc(dloc);
        wg_alloca_size->setDebugLoc(dloc);
        shift->setDebugLoc(dloc);
        start->setDebugLoc(dloc);
        wg_alloca_addr->setDebugLoc(dloc);
    }

    for (std::vector<AllocaInst *>::iterator I = allocas.begin();
         I != allocas.end(); ++I)
    {
        alloca = *I;

        // get number of elements, element type size, compute total size
        Value *numElems = alloca->getArraySize();
        if (llvm::dyn_cast<llvm::ConstantInt>(numElems)) continue;

        Type *elementType = alloca->getAllocatedType();
        // getTypeSizeInBits(), what about uchar3 type?
        uint64_t esBytes = dataLayout.getTypeStoreSize(elementType);
        Value *esize = ConstantInt::get(Int32, (uint32_t) esBytes); 
        Instruction *alloca_size = BinaryOperator::Create(
                               Instruction::Mul, esize, numElems, "", alloca);

        // Inline _wg_alloca(size) implementation
        //     tmp = wg_alloca_addr;
        //     wg_alloca_addr = (wg_alloca_addr + size + 0x7) & 0xFFFFFFF8;
        //     return tmp;
        Instruction *curr_alloca_addr = wg_alloca_addr;
        wg_alloca_addr = BinaryOperator::Create(Instruction::Add,
                                      wg_alloca_addr, alloca_size, "", alloca);
        wg_alloca_addr = BinaryOperator::Create(Instruction::Add,
                                      wg_alloca_addr, 
                                      ConstantInt::get(Int32, (uint32_t) 7), 
                                      "", alloca);
        wg_alloca_addr = BinaryOperator::Create(Instruction::And,
                                      wg_alloca_addr, 
                                 ConstantInt::get(Int32, (uint32_t)0xFFFFFFF8),
                                      "", alloca);

        // cast to alloca type
        Instruction * new_alloca = new IntToPtrInst(curr_alloca_addr,
                                                    alloca->getType());

        // replace AllocaInst with new _wg_alloca()
        ReplaceInstWithInst(alloca, new_alloca);

        // accumulate element type size
        unsigned align = dataLayout.getPrefTypeAlignment(elementType);
        // LLVM boolean is i1 (one byte), TI translates it to i32 (4 bytes)
        // TODO: what if bool is in a struct???
        if (elementType == Int1 || (elementType->isArrayTy() &&
                                    elementType->getArrayElementType() == Int1))
            esBytes *= 4;
        // TI runtime _wg_alloca always returns addresses at 8-byte alignment
        if (align < 8) align = 8;
        wi_alloca_size = (wi_alloca_size + align - 1) & (~(align-1));
        wi_alloca_size += esBytes;
    }

    // put total orig_wi_size into attributes data in the function
    char *s_wi_alloca_size = new char[32];  // we have to leak this
    snprintf(s_wi_alloca_size, 32, "_wi_alloca_size=%d", wi_alloca_size);
    F.addFnAttr(StringRef(s_wi_alloca_size));

    return true;
}

/**************************************************************************
* rewrite_ocl_funcs
**************************************************************************/
bool TIOpenclWorkGroupAggregation::rewrite_ocl_funcs(Function &F) 
{
    CallInst *call;
    Module *M = F.getParent();
    std::vector<CallInst *> wi_calls;
    for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I)
    {
        if ((call = dyn_cast<CallInst>(&*I)) == NULL)           continue;
        if (call->getCalledFunction() == NULL)                  continue;
        string name(call->getCalledFunction()->getName());
        if (name != "get_local_id" && name != "get_local_size") continue;
        wi_calls.push_back(call);
    }
    if (wi_calls.empty()) return false;
    
    LLVMContext &ctx = F.getContext();
    Type  *Int32 = Type::getInt32Ty(F.getContext());
    Value *zero  = ConstantInt::get(Int32, 0);
    std::vector<CallInst *>::iterator I, E;
    for (I = wi_calls.begin(), E = wi_calls.end(); I != E; ++I)
    {
        call = *I;
        string name(call->getCalledFunction()->getName());

        if (name == "get_local_id") 
        {
            DebugLoc psn = call->getDebugLoc();
            DIScope scope(psn.getScope(ctx));
#if 0
            std::cout << "Replace get_local_id at " 
             << scope.getFilename().str()
             << ":" 
             << psn.getLine()
             << ":" 
             << psn.getCol()
             << std::endl;
#endif

            if (is_pocl_mode)
            {
                // pocl: reqd_work_group_size(1,1,1) case
                call->replaceAllUsesWith(zero);
                call->eraseFromParent();
            }
            else
            {
                BasicBlock::iterator BI(call);
                ReplaceInstWithValue(call->getParent()->getInstList(), BI,
                                     get_IV(F, call));
            }
        }
        else if (name == "get_local_size")
        {
            // remaining get_local_size() are generated by pocl,
            // arguments guaranteed to be constants: 0, 1, or 2
            ReplaceInstWithInst(call,
                                createLoadGlobal(4+findDim(call), M));
        }
    }
    return true;
}

/******************************************************************************
* hoist_wg_invariant_code
* e.g. move loads of kernel_config_l2 to entry block
******************************************************************************/
bool
TIOpenclWorkGroupAggregation::hoist_wg_invariant_code(Function &F)
{
    Module              *M = F.getParent();
    BasicBlock      *entry = &(F.getEntryBlock());
    llvm::Type      *Int32 = llvm::IntegerType::getInt32Ty(M->getContext());

#define MAX_KERNEL_CONFIG_L2_ENTRIES	32
    Value  *geps[MAX_KERNEL_CONFIG_L2_ENTRIES];
    Value *loads[MAX_KERNEL_CONFIG_L2_ENTRIES];
    for (int i = 0; i < MAX_KERNEL_CONFIG_L2_ENTRIES; i++)
        geps[i] = loads[i] = NULL;
    if (wgsizes[0] > 0 || wgsizes[1] > 0 || wgsizes[2] > 0)
    {
        loads[4] = ConstantInt::get(Int32, wgsizes[0]);
        loads[5] = ConstantInt::get(Int32, wgsizes[1]);
        loads[6] = ConstantInt::get(Int32, wgsizes[2]);
    }

    /* Two forms: LoadInst w/ GetElementPtrExpr, LoadInst w/ GetElementPtrInst
    */
    std::vector<Instruction *> move_list, delete_list;
    for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I)
    {
        Instruction *instr = (&*I);
        Value **geps_loads;
        int indexVal = -1;
        if ((indexVal = getKernelConfigGEPInstIndex(instr)) >= 0)
          geps_loads = geps;
        else if ((indexVal = getKernelConfigLoadInstIndex(instr)) >= 0)
          geps_loads = loads;
        if (indexVal < 0)  continue;

        if (geps_loads[indexVal] == NULL)
        {
            geps_loads[indexVal] = instr;
            if (instr->getParent() != entry)  move_list.push_back(instr);
        }
        else
        {
            instr->replaceAllUsesWith(geps_loads[indexVal]);
            delete_list.push_back(instr);
        }
    }

    if (move_list.empty() && delete_list.empty())  return false;

    Instruction* inspt = entry->getFirstNonPHI();
    std::vector<Instruction *>::iterator I, E;
    for (I = move_list.begin(), E = move_list.end(); I != E; ++I)
        (*I)->moveBefore(inspt);

    for (I = delete_list.begin(), E = delete_list.end(); I != E; ++I)
        (*I)->eraseFromParent();

    return true;
}

BasicBlock* TIOpenclWorkGroupAggregation::findExitBlock(Function &F)
{
    BasicBlock *exit = 0;

    /*-------------------------------------------------------------------------
    * Find the one block with no successors
    *------------------------------------------------------------------------*/
    for (Function::iterator B = F.begin(), E = F.end(); B != E; ++B)
        if ((*B).getTerminator()->getNumSuccessors() == 0) 
        {
            if (!exit) exit = &(*B);
            else assert(false);
        }

    /*-------------------------------------------------------------------------
    * Split the return off into it's own block
    *------------------------------------------------------------------------*/
    Instruction *ret = exit->getTerminator();

    if (ret != &exit->front())
        exit = SplitBlock(exit, ret, this);

    return exit;
}

/**************************************************************************
* collect_mem_for_metadata(Function &F)
**************************************************************************/
void TIOpenclWorkGroupAggregation::collect_mem_for_metadata(Function &F)
{
    loop_mdnodes.clear();
    loop_mem_instrs.clear();
    for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I)
        if ((*I).mayReadOrWriteMemory()) loop_mem_instrs.push_back(&*I);
}

/**************************************************************************
* add_loop_mem_metadata(Function &F)
**************************************************************************/
void TIOpenclWorkGroupAggregation::add_loop_mem_metadata(Function &F)
{
    if (loop_mdnodes.empty()) return;

    LLVMContext &ctx   = F.getContext();
    MDNode *mem_mdnode = MDNode::get(ctx, loop_mdnodes);
    for (std::list<Instruction*>::iterator I = loop_mem_instrs.begin(),
                                           E = loop_mem_instrs.end();
        I != E; ++I)
        (*I)->setMetadata("llvm.mem.parallel_loop_access", mem_mdnode);
}

/**************************************************************************
* add_loop(Function &F, int dimIdx)
**************************************************************************/
void TIOpenclWorkGroupAggregation::add_loop(Function &F, int dimIdx,
                                            bool regLocals, int ubound)
{
   LLVMContext &ctx   = F.getContext();
   Type        *Int32 = Type::getInt32Ty(ctx);
   Value       *zero  = ConstantInt::get(Int32, 0); 
   Value       *one   = ConstantInt::get(Int32, 1); 
   Module      *M     = F.getParent();

   if (ubound == 1)
   {
       IVPhi[dimIdx] = zero;
       if (dimIdx < 1) IVPhi[1] = zero;
       if (dimIdx < 2) IVPhi[2] = zero;
       return;
   }

   BasicBlock*  exit     = findExitBlock(F);
   BasicBlock*  entry    = &(F.getEntryBlock());

   BasicBlock*  bodytop  = SplitBlock(entry, &entry->front(), this);
   BasicBlock*  bodyend  = exit;
                exit     = SplitBlock(bodyend, &exit->front(), this);

   exit->setName(".exit");
   entry->setName(".entry");
   bodytop->setName(".bodyTop");
   bodyend->setName(".bodyEnd");

   /*----------------------------------------------------------------------
   * Populate the branch around
   *---------------------------------------------------------------------*/
   Instruction *branch = entry->getTerminator();
   Instruction *ld_upper_bnd = NULL;
   Value       *upper_bnd    = NULL;
   if (ubound > 1)
       upper_bnd = ConstantInt::get(Int32, ubound);
   else
       upper_bnd = ld_upper_bnd = createLoadGlobal(4+dimIdx, M, branch);

   Instruction *cmp = CmpInst::Create (Instruction::ICmp, CmpInst::ICMP_SGT, 
                           upper_bnd, zero, "", branch);

   Instruction *cbr = BranchInst::Create(bodytop, exit, cmp);
   ReplaceInstWithInst(branch, cbr);

   if (di_function != NULL)
   {
       DebugLoc dloc = DebugLoc::get(di_scope_line_num, 0, di_function, NULL);
       if (ld_upper_bnd != NULL)  ld_upper_bnd->setDebugLoc(dloc);
       cmp->setDebugLoc(dloc);
       cbr->setDebugLoc(dloc);
   }

   /*----------------------------------------------------------------------
   * Add the phi node to the top of the body
   *---------------------------------------------------------------------*/
   PHINode *phi = PHINode::Create(Int32, 2, "", &bodytop->front());
   phi->addIncoming(zero, entry);

   /*----------------------------------------------------------------------
   * Add the loop control to the bottom of the bodyend
   *---------------------------------------------------------------------*/
   branch = bodyend->getTerminator();
   Instruction *inc = BinaryOperator::Create(Instruction::Add, 
                      phi, one, Twine(), branch);

   Instruction *cmp2 = CmpInst::Create (Instruction::ICmp, CmpInst::ICMP_SLT, 
                       inc, upper_bnd, "", branch);
   Instruction *cbr2 = BranchInst::Create(bodytop, exit, cmp2);
   ReplaceInstWithInst(branch, cbr2);


   if (di_function != NULL && di_end_scope_line != 0)
   {
       DebugLoc dloc2 = DebugLoc::get(di_end_scope_line, 0, di_function, NULL);
       inc->setDebugLoc(dloc2);
       cmp2->setDebugLoc(dloc2);
       cbr2->setDebugLoc(dloc2);
   }

   /*----------------------------------------------------------------------
   * Add llvm.loop.parallel, to add llvm.mem.loop_parallel_access metadata
   *---------------------------------------------------------------------*/
   if (regLocals)
   {
       MDNode *dummy = MDNode::getTemporary(ctx, ArrayRef<Metadata*>());
       MDNode *loopmeta = MDNode::get(ctx, dummy);
       loopmeta->replaceOperandWith(0, loopmeta);
       MDNode::deleteTemporary(dummy);

       cbr2->setMetadata("llvm.loop.parallel", loopmeta);
       loop_mdnodes.push_back(loopmeta);

       if (ld_upper_bnd)  loop_mem_instrs.push_back(ld_upper_bnd);  // okay for all dimensions?
   }

   phi->addIncoming(inc, bodyend);
   IVPhi[dimIdx] = phi;

   if (dimIdx < 1) IVPhi[1] = zero;
   if (dimIdx < 2) IVPhi[2] = zero;
}

/**************************************************************************
* Implicit long/ulong to float/double conversions: use convert_* BIFs
* e.g. long x; (float) x  => convert_float(x)
**************************************************************************/
bool TIOpenclWorkGroupAggregation::implicit_long_conv_use_bif(Function &F)
{
    bool changed = false;
    Type  *Int64 = Type::getInt64Ty(F.getContext());
    Type  *FP32  = Type::getFloatTy(F.getContext());
    Type  *FP64  = Type::getDoubleTy(F.getContext());

    std::list<CastInst*> cast_list;
    for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I)
    {
        if (CastInst *cast = dyn_cast<CastInst>(&*I))
        {
            if (!isa<SIToFPInst>(cast) && !isa<UIToFPInst>(cast))  continue;
            Type *ty = cast->getType();
            if (ty != FP32 && ty != FP64)  continue;
            if (cast->getOperand(0)->getType() != Int64)  continue;
            cast_list.push_back(cast);
        }

    }
    if (cast_list.empty())  return changed;

    Module       *M      = F.getParent();
    FunctionType *l2f_ft = FunctionType::get(FP32, Int64, false);
    FunctionType *l2d_ft = FunctionType::get(FP64, Int64, false);
    Function     *l2f_f  = dyn_cast<Function>(M->getOrInsertFunction(
                                              "_Z13convert_floatl", l2f_ft));
    Function     *l2d_f  = dyn_cast<Function>(M->getOrInsertFunction(
                                              "_Z14convert_doublel", l2d_ft));
    Function     *ul2f_f = dyn_cast<Function>(M->getOrInsertFunction(
                                              "_Z13convert_floatm", l2f_ft));
    Function     *ul2d_f = dyn_cast<Function>(M->getOrInsertFunction(
                                              "_Z14convert_doublem", l2d_ft));

    for (std::list<CastInst*>::iterator I = cast_list.begin(),
                                        E = cast_list.end(); I != E; ++I)
    {
        CastInst    *cast    = *I;
        Instruction *newcast = NULL;
        Type        *ty      = cast->getType();
        Value       *lv      = cast->getOperand(0);
        if (isa<SIToFPInst>(cast))
            newcast = (ty == FP32) ? CallInst::Create(l2f_f, lv)
                                   : CallInst::Create(l2d_f, lv);
        else
            newcast = (ty == FP32) ? CallInst::Create(ul2f_f, lv)
                                   : CallInst::Create(ul2d_f, lv);
        ReplaceInstWithInst(cast, newcast);
        changed = true;
    }

    return changed;
}

char TIOpenclWorkGroupAggregation::ID = 0;
static RegisterPass<TIOpenclWorkGroupAggregation> 
                   X("wga", "Work Group Aggregation", false, false);

}

