/*
    Copyright (c) 2012 Tampere University of Technology.
    Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
 */
/**
 * @file WorkItemAliasAnalysis.cc
 *
 * Definition of WorkItemAliasAnalysis class.
 *
 * @author Vladimír Guzma 2012
 */

#include "WorkItemAliasAnalysis.h"
using namespace pocl;

// Register this pass...
char WorkItemAliasAnalysis::ID = 0;
RegisterPass<WorkItemAliasAnalysis>
    X("wi-aa", "Work item alias analysis.", false, false);
// Register it also to pass group
RegisterAnalysisGroup<AliasAnalysis> Y(X);  

ImmutablePass *createWorkItemAliasAnalysisPass() {
    return new WorkItemAliasAnalysis();
}

extern "C" {                                
    ImmutablePass*
    create_workitem_aa_plugin() {
        return new WorkItemAliasAnalysis();
    }
}

void
WorkItemAliasAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AliasAnalysis::getAnalysisUsage(AU);
}

/**
 * Test if memory locations are from different work items from same region.
 * Then they can not alias.
 */
AliasAnalysis::AliasResult
WorkItemAliasAnalysis::alias(const Location &LocA,
                                    const Location &LocB) {
    // If either of the memory references is empty, it doesn't matter what the
    // pointer values are. This allows the code below to ignore this special
    // case.
    if (LocA.Size == 0 || LocB.Size == 0)
        return NoAlias;
    
    // Pointers from different address spaces do not alias
    if (cast<PointerType>(LocA.Ptr->getType())->getAddressSpace() != 
        cast<PointerType>(LocB.Ptr->getType())->getAddressSpace()) {
        return NoAlias;
    }
    // In case code is created by pocl, we can also use metadata.
    if (isa<Instruction>(LocA.Ptr) && isa<Instruction>(LocB.Ptr)) {
        const Instruction* valA = dyn_cast<Instruction>(LocA.Ptr);
        const Instruction* valB = dyn_cast<Instruction>(LocB.Ptr);
        if (valA->getMetadata("wi") && valB->getMetadata("wi")) {
            const MDNode* mdA = valA->getMetadata("wi");
            const MDNode* mdB = valB->getMetadata("wi");
            // Compare region ID. If they are same, different work items
            // imply no aliasing. If regions are different or work items
            // are same anything can happen.
            // Fall back to other AAs.
            const MDNode* mdRegionA = dyn_cast<MDNode>(mdA->getOperand(1));
            const MDNode* mdRegionB = dyn_cast<MDNode>(mdB->getOperand(1)); 
#if LLVM_VERSION_MAJOR <= 3 && LLVM_VERSION_MINOR < 6
            ConstantInt* C1 = dyn_cast<ConstantInt>(mdRegionA->getOperand(1));
            ConstantInt* C2 = dyn_cast<ConstantInt>(mdRegionB->getOperand(1));
#else
            ConstantInt* C1 = dyn_cast<ConstantInt>(
              dyn_cast<ConstantAsMetadata>(mdRegionA->getOperand(1))->getValue());
            ConstantInt* C2 = dyn_cast<ConstantInt>(
              dyn_cast<ConstantAsMetadata>(mdRegionB->getOperand(1))->getValue());
#endif
            if (C1->getValue() == C2->getValue()) {
                // Now we have both locations from same region. Check for different
                // work items.
                MDNode* iXYZ= dyn_cast<MDNode>(mdA->getOperand(2));
                MDNode* jXYZ= dyn_cast<MDNode>(mdB->getOperand(2));
                assert(iXYZ->getNumOperands() == 4);
                assert(jXYZ->getNumOperands() == 4);
#if LLVM_VERSION_MAJOR <= 3 && LLVM_VERSION_MINOR < 6              
                ConstantInt *CIX = dyn_cast<ConstantInt>(iXYZ->getOperand(1));
                ConstantInt *CJX = dyn_cast<ConstantInt>(jXYZ->getOperand(1));

                ConstantInt *CIY = dyn_cast<ConstantInt>(iXYZ->getOperand(2));
                ConstantInt *CJY = dyn_cast<ConstantInt>(jXYZ->getOperand(2));
                
                ConstantInt *CIZ = dyn_cast<ConstantInt>(iXYZ->getOperand(3));
                ConstantInt *CJZ = dyn_cast<ConstantInt>(jXYZ->getOperand(3));
#else
                ConstantInt *CIX = 
                  dyn_cast<ConstantInt>(
                      dyn_cast<ConstantAsMetadata>(
                        iXYZ->getOperand(1))->getValue());
                ConstantInt *CJX = 
                  dyn_cast<ConstantInt>(
                    dyn_cast<ConstantAsMetadata>(
                      jXYZ->getOperand(1))->getValue());

                ConstantInt *CIY = 
                  dyn_cast<ConstantInt>(
                    dyn_cast<ConstantAsMetadata>(
                      iXYZ->getOperand(2))->getValue());
                ConstantInt *CJY = 
                  dyn_cast<ConstantInt>(
                    dyn_cast<ConstantAsMetadata>(
                      jXYZ->getOperand(2))->getValue());
                
                ConstantInt *CIZ = 
                  dyn_cast<ConstantInt>(
                    dyn_cast<ConstantAsMetadata>(
                      iXYZ->getOperand(3))->getValue());
                ConstantInt *CJZ = 
                  dyn_cast<ConstantInt>(
                    dyn_cast<ConstantAsMetadata>(
                      jXYZ->getOperand(3))->getValue());
#endif
                
                if ( !(CIX->getValue() == CJX->getValue()
                    && CIY->getValue() == CJY->getValue()
                    && CIZ->getValue() == CJZ->getValue())) {
                    return NoAlias;
                }                
            }        
        }
    }
  
    // Forward the query to the next analysis.
    return AliasAnalysis::alias(LocA, LocB);
}
