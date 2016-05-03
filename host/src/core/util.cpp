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

#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Constants.h>


using namespace llvm;

#if !defined (_MSC_VER)
/******************************************************************************
* Parse first line in a file, read integer immediately following a string
******************************************************************************/
uint32_t parse_file_line_value(const char *fname, const char *sname,
                              uint32_t default_val)
{
    uint32_t val   = default_val;
    FILE *fp   = NULL;
    char *line = NULL;
    char *str  = NULL;
    size_t len = 0;

    if ((fp = fopen(fname, "r")) == NULL) return val;
    if (getline(&line, &len, fp) != -1)
    {
        if ((str = strstr(line, sname)) != NULL)
        {
            str += strlen(sname);
            while(!isdigit(*str) && *str != '\0') str++;
            if (*str != '\0') val = atoi(str);
        }
    }

    if (fp   != NULL) fclose(fp);
    if (line != NULL) free(line);
    return val;
}
#endif

/******************************************************************************
* Converting ocl error number to ocl error string, for more meaningful report
******************************************************************************/
const char* ocl_error_str(int ocl_error_num)
{
    switch(ocl_error_num)
    {
        case 0  : return "CL_SUCCESS";
        case -1 : return "CL_DEVICE_NOT_FOUND";
        case -2 : return "CL_DEVICE_NOT_AVAILABLE";
        case -3 : return "CL_COMPILER_NOT_AVAILABLE";
        case -4 : return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case -5 : return "CL_OUT_OF_RESOURCES";
        case -6 : return "CL_OUT_OF_HOST_MEMORY";
        case -7 : return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case -8 : return "CL_MEM_COPY_OVERLAP";
        case -9 : return "CL_IMAGE_FORMAT_MISMATCH";
        case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case -11: return "CL_BUILD_PROGRAM_FAILURE";
        case -12: return "CL_MAP_FAILURE";
        case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";

        case -30: return "CL_INVALID_VALUE";
        case -31: return "CL_INVALID_DEVICE_TYPE";
        case -32: return "CL_INVALID_PLATFORM";
        case -33: return "CL_INVALID_DEVICE";
        case -34: return "CL_INVALID_CONTEXT";
        case -35: return "CL_INVALID_QUEUE_PROPERTIES";
        case -36: return "CL_INVALID_COMMAND_QUEUE";
        case -37: return "CL_INVALID_HOST_PTR";
        case -38: return "CL_INVALID_MEM_OBJECT";
        case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case -40: return "CL_INVALID_IMAGE_SIZE";
        case -41: return "CL_INVALID_SAMPLER";
        case -42: return "CL_INVALID_BINARY";
        case -43: return "CL_INVALID_BUILD_OPTIONS";
        case -44: return "CL_INVALID_PROGRAM";
        case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
        case -46: return "CL_INVALID_KERNEL_NAME";
        case -47: return "CL_INVALID_KERNEL_DEFINITION";
        case -48: return "CL_INVALID_KERNEL";
        case -49: return "CL_INVALID_ARG_INDEX";
        case -50: return "CL_INVALID_ARG_VALUE";
        case -51: return "CL_INVALID_ARG_SIZE";
        case -52: return "CL_INVALID_KERNEL_ARGS";
        case -53: return "CL_INVALID_WORK_DIMENSION";
        case -54: return "CL_INVALID_WORK_GROUP_SIZE";
        case -55: return "CL_INVALID_WORK_ITEM_SIZE";
        case -56: return "CL_INVALID_GLOBAL_OFFSET";
        case -57: return "CL_INVALID_EVENT_WAIT_LIST";
        case -58: return "CL_INVALID_EVENT";
        case -59: return "CL_INVALID_OPERATION";
        case -60: return "CL_INVALID_GL_OBJECT";
        case -61: return "CL_INVALID_BUFFER_SIZE";
        case -62: return "CL_INVALID_MIP_LEVEL";
        case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
        case -64: return "CL_INVALID_PROPERTY";
        default:  return "UNKNOWN_ERROR_NUM";
    }
}

/******************************************************************************
* isKernelFunction(Function &F) const
******************************************************************************/
bool isKernelFunction(llvm::Function &F)
{
    llvm::NamedMDNode *ks = F.getParent()->getNamedMetadata("opencl.kernels");
    if (ks == NULL)  return false;
    for (unsigned int i = 0; i < ks->getNumOperands(); ++i)
        if (llvm::Value *value = dyn_cast<llvm::ValueAsMetadata>(ks->getOperand(i)->getOperand(0))->getValue())
            if (llvm::cast<llvm::Function>(value) == &F)
                return true;
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
        if (llvm::Value *value = dyn_cast<llvm::ValueAsMetadata>(ker->getOperand(0))->getValue())
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

