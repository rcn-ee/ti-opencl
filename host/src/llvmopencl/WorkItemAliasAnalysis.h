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

#include "config.h"
#include <iostream>

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Pass.h"
#if (defined LLVM_3_1 || defined LLVM_3_2)
#include "llvm/Metadata.h"
#include "llvm/Constants.h"
#else
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Constants.h"
#endif

using namespace llvm;

namespace pocl {
/// WorkItemAliasAnalysis - This is a simple alias analysis
/// implementation that uses pocl metadata to make sure memory accesses from
/// different work items are not aliasing.
class WorkItemAliasAnalysis : public llvm::ImmutablePass, public llvm::AliasAnalysis {
public:
    static char ID; 
    WorkItemAliasAnalysis() : ImmutablePass(ID) {}

    /// getAdjustedAnalysisPointer - This method is used when a pass implements
    /// an analysis interface through multiple inheritance.  If needed, it
    /// should override this to adjust the this pointer as needed for the
    /// specified pass info.
    virtual void *getAdjustedAnalysisPointer(AnalysisID PI) {
        if (PI == &AliasAnalysis::ID)
            return (AliasAnalysis*)this;
        return this;
    }
    virtual void initializePass() {
        InitializeAliasAnalysis(this);
    }
    
    private:
        virtual void getAnalysisUsage(AnalysisUsage &AU) const;
        virtual AliasResult alias(const Location &LocA, const Location &LocB);

    };
}

