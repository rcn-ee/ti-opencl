/******************************************************************************
 * Copyright (c) 2013-2016, Texas Instruments Incorporated - http://www.ti.com/
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
#include "PrivatizationAliasAnalysis.h"
#include <iostream>

namespace tiocl {

char TIOpenCLPrivatizationAliasAnalysis::ID = 0;
RegisterPass<TIOpenCLPrivatizationAliasAnalysis>
    X("tiocl-priv-aa", "TI OCL privatization alias analysis.", false, true);
// Register it also to pass group
RegisterAnalysisGroup<AliasAnalysis> Y(X);

AliasAnalysis::AliasResult
TIOpenCLPrivatizationAliasAnalysis::alias(const Location &LocA,
                                          const Location &LocB)
{
  if (LocA.Size == 0 || LocB.Size == 0)  return NoAlias;
  if (cast<PointerType>(LocA.Ptr->getType())->getAddressSpace() !=
      cast<PointerType>(LocB.Ptr->getType())->getAddressSpace())  return NoAlias;

  if (isa<Instruction>(LocA.Ptr) && isa<Instruction>(LocB.Ptr))
  {
    const Value *memA = findBasedOnMemory(LocA.Ptr);
    const Value *memB = findBasedOnMemory(LocB.Ptr);

    // Based on two different ocl privatized memory: no alias
    // One based ocl privatized memory, the other not: no alias
    if (memA != memB && isa<AllocaInst>(memA) && isa<AllocaInst>(memB))
      if (cast<AllocaInst>(memA)->getMetadata("ocl.restrict") != nullptr ||
          cast<AllocaInst>(memB)->getMetadata("ocl.restrict") != nullptr)
        return NoAlias;

    // One based on ocl privatized memory, the other loads a pointer: no alias
    // 1. Ocl privatized memory does not alias to the content that it stores
    // 2. Pointer to ocl privatized memory never gets stored into another
    //    privatized memory (i.e. privatization process is NOT recursive)
    if (memA != memB && ((isa<AllocaInst>(memA) && isa<LoadInst>(memB)) ||
                         (isa<AllocaInst>(memB) && isa<LoadInst>(memA))) )
      if (cast<Instruction>(memA)->getMetadata("ocl.restrict") != nullptr ||
          cast<Instruction>(memB)->getMetadata("ocl.restrict") != nullptr)
        return NoAlias;
  }

  return AliasAnalysis::alias(LocA, LocB);
}

const Value*
TIOpenCLPrivatizationAliasAnalysis::findBasedOnMemory(const Value *val)
{
  if (const GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(val))
    return findBasedOnMemory(gep->getPointerOperand());
  if (const BitCastInst *bitcast = dyn_cast<BitCastInst>(val))
    return findBasedOnMemory(bitcast->getOperand(0));
  return val;
}

}

