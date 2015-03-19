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
#ifndef __TIOPENCLWORKGROUPAGGREGATIONPASS_H
#define __TIOPENCLWORKGROUPAGGREGATIONPASS_H

#include <string>
#include <set>
#include <vector>
#include <list>
#include "boost/tuple/tuple.hpp"
#include <llvm/Pass.h>
#include <llvm/IR/Instruction.h>

#define MAX_DIMENSIONS 3u

namespace llvm
{

class TIOpenclWorkGroupAggregation : public FunctionPass 
{
  public:
    static char ID;

    TIOpenclWorkGroupAggregation(bool pocl_mode = false);
    virtual bool runOnFunction(Function &F);
    virtual void getAnalysisUsage(AnalysisUsage &Info) const;

  private:
    Value                 *IVPhi[MAX_DIMENSIONS];
    int                    wgsizes[MAX_DIMENSIONS];
    bool                   is_pocl_mode;
    llvm::MDNode          *di_function;
    unsigned int           di_scope_line_num;
    unsigned int           di_end_scope_line;
    std::vector<Value*>    loop_mdnodes;
    std::list<Instruction*> loop_mem_instrs;

  private:
    Instruction* createLoadGlobal(int32_t idx, Module* m, Instruction *before=0, 
                                  const char *name=0);

    BasicBlock*         findExitBlock     (Function &F);
    unsigned int        findNeededLoopNest(Function &F);
    unsigned int        findDim           (class CallInst* call);
    bool                rewrite_ocl_funcs (Function &F);
    void                add_loop          (Function &F, int dimIdx,
                                           bool regLocals, int ubound);
    bool                rewrite_first_wi  (Function &F, int dims);
    Value*              get_IV(Function &F, CallInst *call);
    bool                rewrite_allocas(Function &F);
    bool                hoist_wg_invariant_code(Function &F);
    void                collect_mem_for_metadata(Function &F);
    void                add_loop_mem_metadata(Function &F);
    bool                implicit_long_conv_use_bif(Function &F);
};

Pass *createTIOpenclWorkGroupAggregationPass(bool is_pocl_mode = false);

}

#endif // __TIOPENCLWORKGROUPAGGREGATIONPASS_H
