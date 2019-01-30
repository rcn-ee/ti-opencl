// Class for barrier instructions, modelled as a CallInstr.
// 
// Copyright (c) 2011 Universidad Rey Juan Carlos
// Copyright (c) 2013-2019, Texas Instruments Incorporated - http://www.ti.com/
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

#include "config.h"

#include "pocl.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/GlobalValue.h"

#include "llvm/IR/Constants.h"
#include "llvm/Support/Casting.h"

#ifndef TI_POCL
#define BARRIER_FUNCTION_NAME "pocl.barrier"
#else
#define BARRIER_FUNCTION_NAME "barrier"
#define ASYNC_WG_COPY_FUNCTION_NAME "async_work_group_copy"
#define ASYNC_WG_STRIDED_COPY_FUNCTION_NAME "async_work_group_strided_copy"
#define WAIT_WG_EVENT_FUNCTION_NAME "wait_group_events"
#endif

namespace pocl {
  
  class Barrier : public llvm::CallInst {

  public:
    static void GetBarriers(llvm::SmallVectorImpl<Barrier *> &B,
                            llvm::Module &M) {
      llvm::Function *F = M.getFunction(BARRIER_FUNCTION_NAME);
      if (F != NULL) {
        for (llvm::Function::use_iterator i = F->use_begin(), e = F->use_end();
             i != e; ++i)
          B.push_back(llvm::cast<Barrier>(*i));
      }
    }
    /**
     * Creates a new barrier before the given instruction.
     *
     * If there was already a barrier there, returns the old one.
     */
    static Barrier *Create(llvm::Instruction *InsertBefore) {
      llvm::Module *M = InsertBefore->getParent()->getParent()->getParent();

      if (InsertBefore != &InsertBefore->getParent()->front() && 
          llvm::isa<Barrier>(InsertBefore->getPrevNode()))
        return llvm::cast<Barrier>(InsertBefore->getPrevNode());

#ifndef TI_POCL
#if LLVM_OLDER_THAN_5_0
      llvm::Function *F = llvm::cast<llvm::Function>
        (M->getOrInsertFunction(BARRIER_FUNCTION_NAME,
                                llvm::Type::getVoidTy(M->getContext()), NULL));
#else
      llvm::Function *F = llvm::cast<llvm::Function>
        (M->getOrInsertFunction(BARRIER_FUNCTION_NAME,
                                llvm::Type::getVoidTy(M->getContext())));
#endif
      F->addFnAttr(llvm::Attribute::NoDuplicate);
      F->setLinkage(llvm::GlobalValue::LinkOnceAnyLinkage);
      return llvm::cast<pocl::Barrier>
        (llvm::CallInst::Create(F, "", InsertBefore));
#else
      llvm::Type *Int32Type = llvm::Type::getInt32Ty(M->getContext());
#if LLVM_OLDER_THAN_5_0
      llvm::Function *F = llvm::cast<llvm::Function>
        (M->getOrInsertFunction(BARRIER_FUNCTION_NAME,
                                llvm::Type::getVoidTy(M->getContext()),
                                Int32Type,
                                NULL));
#else
      llvm::Function *F = llvm::cast<llvm::Function>
        (M->getOrInsertFunction(BARRIER_FUNCTION_NAME,
                                llvm::Type::getVoidTy(M->getContext()),
                                Int32Type));
#endif
      F->addFnAttr(llvm::Attribute::NoDuplicate);
      F->setLinkage(llvm::GlobalValue::LinkOnceAnyLinkage);
      llvm::SmallVector<llvm::Value *, 4> argsarray;
      argsarray.push_back(llvm::ConstantInt::get(Int32Type, 0));
      llvm::ArrayRef<llvm::Value *> args(argsarray);
      return llvm::cast<pocl::Barrier>
        (llvm::CallInst::Create(F, args, "", InsertBefore));
#endif
    }
    static bool classof(const Barrier *) { return true; };
    static bool classof(const llvm::CallInst *C) {
#ifndef TI_POCL
      return C->getCalledFunction() != NULL &&
        C->getCalledFunction()->getName() == BARRIER_FUNCTION_NAME;
#else
      if (llvm::Function *F = C->getCalledFunction())
      {
          llvm::StringRef name = F->getName();
          return name == BARRIER_FUNCTION_NAME ||
                 name.find(ASYNC_WG_COPY_FUNCTION_NAME) != llvm::StringRef::npos ||
                 name.find(ASYNC_WG_STRIDED_COPY_FUNCTION_NAME) != llvm::StringRef::npos ||
                 name.find(WAIT_WG_EVENT_FUNCTION_NAME) != llvm::StringRef::npos;
      }
      return false;
#endif
    }
    static bool classof(const Instruction *I) {
      return (llvm::isa<llvm::CallInst>(I) &&
              classof(llvm::cast<llvm::CallInst>(I)));
    }
    static bool classof(const User *U) {
      return (llvm::isa<Instruction>(U) &&
              classof(llvm::cast<llvm::Instruction>(U)));
    }
    static bool classof(const Value *V) {
      return (llvm::isa<User>(V) &&
              classof(llvm::cast<llvm::User>(V)));
    }



    static bool hasOnlyBarrier(const llvm::BasicBlock *bb) {
      return endsWithBarrier(bb) && bb->size() == 2;
    }

    static bool hasBarrier(const llvm::BasicBlock *bb) 
    {
      for (llvm::BasicBlock::const_iterator i = bb->begin(), e = bb->end();
           i != e; ++i) 
        {
          if (llvm::isa<Barrier>(i)) return true;
        }
      return false;
    }

    // Returns true in case the given basic block starts with a barrier,
    // that is, contains a branch instruction after possible PHI nodes.
    static bool startsWithBarrier(const llvm::BasicBlock *bb) {
      const llvm::Instruction *i = bb->getFirstNonPHI();
      if (i == NULL) 
        return false;
      return llvm::isa<Barrier>(i);
    }

    // Returns true in case the given basic block ends with a barrier,
    // that is, contains only a branch instruction after a barrier call.
    static bool endsWithBarrier(const llvm::BasicBlock *bb) {
      const llvm::TerminatorInst *t = bb->getTerminator();
      if (t == NULL) 
        return false;
      return bb->size() > 1 && t->getPrevNode() != NULL && 
          llvm::isa<Barrier>(t->getPrevNode());
    }

#ifdef TI_POCL
    /* TI: IS barrier(), IS NOT wait_group_events(), async_work_group_*() */
    static bool hasOriginalBarrier(const llvm::BasicBlock *bb)
    {
      for (llvm::BasicBlock::const_iterator i = bb->begin(), e = bb->end();
           i != e; ++i)
        {
          if (llvm::isa<Barrier>(i) &&
              llvm::cast<llvm::CallInst>(i)->getCalledFunction()->getName()
                                                   == BARRIER_FUNCTION_NAME)
            return true;
        }
      return false;
    }
#endif
  };

}

