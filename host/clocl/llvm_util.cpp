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
 * \file core/util.c
 * \brief misc utils
 */

#include "llvm_util.h"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Constants.h>


using namespace llvm;

/******************************************************************************
* isKernelFunction(Function &F) const
******************************************************************************/
bool isKernelFunction(llvm::Function &F)
{
    llvm::NamedMDNode *ks = F.getParent()->getNamedMetadata("opencl.kernels");
    if (ks == NULL)  return false;
    for (unsigned int i = 0; i < ks->getNumOperands(); ++i)
    {
        MDNode *ker = ks->getOperand(i);

        if (ker->getOperand(0))
            if (llvm::Value *value =
                        cast<llvm::ValueAsMetadata>(ker->getOperand(0))->getValue())
                if (llvm::cast<llvm::Function>(value) == &F)
                    return true;
    }
    return false;
}

/******************************************************************************
* getReqdWGSize(Function &F, int wgsizes[3])
******************************************************************************/
bool getReqdWGSize(llvm::Function &F, int wgsizes[3])
{
    wgsizes[0] = wgsizes[1] = wgsizes[2] = 0;
    llvm::NamedMDNode *ks = F.getParent()->getNamedMetadata("opencl.kernels");
    for (unsigned int i = 0; i < ks->getNumOperands(); ++i)
    {
        MDNode *ker = ks->getOperand(i);

        assert (ker->getNumOperands() > 0);

        if (llvm::Value *value = cast<llvm::ValueAsMetadata>(ker->getOperand(0))->getValue())
        {
            if (llvm::cast<llvm::Function>(value) == &F)
            {
                for (unsigned int j = 1; j < ker->getNumOperands(); j++)
                {
                    MDNode *meta = llvm::cast<MDNode>(ker->getOperand(j));
                    if (meta->getNumOperands() == 4 &&
                        llvm::cast<llvm::MDString>(meta->getOperand(0))->getString().str()
                        == std::string("reqd_work_group_size"))
                    {
                        wgsizes[0] = llvm::mdconst::dyn_extract<ConstantInt>(
                                meta->getOperand(1))->getLimitedValue();
                        wgsizes[1] = llvm::mdconst::dyn_extract<ConstantInt>(
                                meta->getOperand(2))->getLimitedValue();
                        wgsizes[2] = llvm::mdconst::dyn_extract<ConstantInt>(
                                meta->getOperand(3))->getLimitedValue();
                        return true;
                    }
                }
                return false;
            }
        }
    }

    return false;
}

/******************************************************************************
* isReqdWGSize111(Function &F)
******************************************************************************/
bool isReqdWGSize111(llvm::Function &F)
{
    int wgsizes[3];
    if (! getReqdWGSize(F, wgsizes))  return false;
    return (wgsizes[0] == 1 && wgsizes[1] == 1 && wgsizes[2] == 1);
}

/******************************************************************************
* getDebugInfo(Function &F, unsigned int &scope_line_num)
******************************************************************************/
llvm::MDNode* getDebugInfo(llvm::Function &F, unsigned int &scope_line_num)
{
#ifndef _SYS_BIOS
    /*-------------------------------------------------------------------------
    * Obtain Debug Information (func scope line number) (when debug is on)
    *------------------------------------------------------------------------*/
    DebugInfoFinder di_finder;
    di_finder.processModule(* F.getParent());
    for (DISubprogram sub : di_finder.subprograms())
    {
        if (sub.describes(&F))
        {
            scope_line_num = sub.getScopeLineNumber();
            return ((MDNode *) sub);
        }
    }
#endif
    scope_line_num = 0;
    return NULL;
}

/******************************************************************************
* containsBarrierCall(Module &M)
******************************************************************************/
bool containsBarrierCall(llvm::Module &M)
{
    llvm::CallInst* call;
    for (llvm::Module::iterator F = M.begin(), EF = M.end();
         F != EF; ++F)
        for (llvm::inst_iterator I = inst_begin(*F), E = inst_end(*F);
             I != E; ++I)
        {
            if (!(call = llvm::dyn_cast<llvm::CallInst>(&*I))) continue;
            if (!call->getCalledFunction())                    continue;
            std::string name(call->getCalledFunction()->getName());
            if (name == "barrier" ||
                name.find("wait_group_events") != std::string::npos ||
                name.find("async_work_group_copy") != std::string::npos ||
                name.find("async_work_group_strided_copy") != std::string::npos
               )  return true;
        }
    return false;
}

/******************************************************************************
* canLocalsFitInReg(Function &F)
* c66 specific: max VREG size is 128 bits, anything else will be turned into
* "auto" variable, which means, memory
******************************************************************************/
#define C66_MAX_VREG_SIZE	128
bool canLocalsFitInReg(llvm::BasicBlock *bb)
{
    DataLayout dataLayout(bb->getParent()->getParent());
    llvm::Type *ty;

    for (BasicBlock::iterator I = bb->begin(), E = bb->end(); I != E; ++I)
    {
        if (!(ty = I->getType())) continue;
        if (! ty->isSized()) continue;
        if (dataLayout.getTypeSizeInBits(ty) > C66_MAX_VREG_SIZE) return false;
    }

    return true;
}

bool canLocalsFitInReg(llvm::Function &F)
{
    for (Function::iterator B = F.begin(), E = F.end(); B != E; ++B)
        if (! canLocalsFitInReg(B)) return false;
    return true;
}

/******************************************************************************
* findFirstDebugLine(BasicBlock *bb)
******************************************************************************/
unsigned int
findFirstDebugLine(BasicBlock *bb)
{
  LLVMContext &ctx = bb->getContext();
  for (BasicBlock::iterator I = bb->begin(), E = bb->end();
       I != E; ++I)
  {
    const DebugLoc &dl = (*I).getDebugLoc();
    if (dl.getInlinedAt(ctx) == NULL && dl.getLine() != 0)
      return dl.getLine();
  }
  return 0;
}

/******************************************************************************
* findLastDebugLine(BasicBlock *bb)
******************************************************************************/
unsigned int
findLastDebugLine(BasicBlock *bb)
{
  LLVMContext &ctx = bb->getContext();
  for (BasicBlock::reverse_iterator I = bb->rbegin(), E = bb->rend();
       I != E; ++I)
  {
    const DebugLoc &dl = (*I).getDebugLoc();
    if (dl.getInlinedAt(ctx) == NULL && dl.getLine() != 0)
      return dl.getLine();
  }
  return 0;
}

/******************************************************************************
* getKernelConfigGEPInstIndex(GetElementPtrInst *gep_instr)
* -1: not found, >= 0, index in kernel_config_l2 data structure
******************************************************************************/
int getKernelConfigGEPInstIndex(Instruction *instr)
{
  if (GetElementPtrInst *gep_instr = dyn_cast<GetElementPtrInst>(instr))
  {
    if (gep_instr->getNumIndices() == 2)
    {
      Module              *M = instr->getParent()->getParent()->getParent();
      llvm::Type      *Int32 = llvm::IntegerType::getInt32Ty(M->getContext());
      llvm::ArrayType  *type = ArrayType::get(Int32, 32);

      M->getOrInsertGlobal("kernel_config_l2", type);
      GlobalVariable* global = M->getNamedGlobal("kernel_config_l2");

      llvm::Value *load_from  = gep_instr->getPointerOperand();
      llvm::Value *load_index = gep_instr->getOperand(2);
      if (load_from == global && isa<ConstantInt>(load_index))
        return dyn_cast<ConstantInt>(load_index)->getSExtValue();
    }
  }

  return -1;
}

/******************************************************************************
* getKernelConfigLoadInstIndex(LoadInst *load)
******************************************************************************/
int getKernelConfigLoadInstIndex(Instruction *instr)
{
  if (LoadInst *load = dyn_cast<LoadInst>(instr))
  {
    llvm::Value *ptr = load->getPointerOperand();
    if (ConstantExpr *gep_expr = dyn_cast<ConstantExpr>(ptr))
    {
      if (gep_expr->getNumOperands() == 3)
      {
        Module              *M = instr->getParent()->getParent()->getParent();
        llvm::Type      *Int32 = llvm::IntegerType::getInt32Ty(M->getContext());
        llvm::ArrayType  *type = ArrayType::get(Int32, 32);

        M->getOrInsertGlobal("kernel_config_l2", type);
        GlobalVariable* global = M->getNamedGlobal("kernel_config_l2");

        llvm::Value *load_from  = gep_expr->getOperand(0);
        llvm::Value *load_index = gep_expr->getOperand(2);
        if (load_from == global && isa<ConstantInt>(load_index))
          return dyn_cast<ConstantInt>(load_index)->getSExtValue();
      }
    }
    else if (GetElementPtrInst *gep_instr = dyn_cast<GetElementPtrInst>(ptr))
      return getKernelConfigGEPInstIndex(gep_instr);
  }
  return -1;
}

