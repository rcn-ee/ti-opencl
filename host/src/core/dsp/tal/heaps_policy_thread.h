/******************************************************************************
 * Copyright (c) 2016, Texas Instruments Incorporated - http://www.ti.com/
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Texas Instruments Incorporated nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#ifndef HEAPS_POLICY_THREAD_H_
#define HEAPS_POLICY_THREAD_H_

#include <unistd.h>
#include "heap_manager.h"
#include "heap_manager_policy_thread.h"

/******************************************************************************
* class HeapsMultiThreadedPolicy
*   Create heap managers in private process memory and share across threads.
******************************************************************************/
class HeapsMultiThreadedPolicy
{
public:
    typedef utility::HeapManager<DSPDevicePtr, uint32_t, 
            utility::MultiThread<DSPDevicePtr, uint32_t> > Heap32Bit;

    typedef utility::HeapManager<DSPDevicePtr64, uint64_t, 
            utility::MultiThread<DSPDevicePtr64, uint64_t> > Heap64Bit;

    HeapsMultiThreadedPolicy() : ddr_heap1_(new Heap64Bit), 
                                 ddr_heap2_(new Heap64Bit), 
                                 msmc_heap_(new Heap64Bit)
    {}
        
    ~HeapsMultiThreadedPolicy()
    {
	delete msmc_heap_;
	delete ddr_heap1_;
	delete ddr_heap2_;
    }

protected:
    Heap64Bit* ddr_heap1_;  // persistently mapped off-chip memory
    Heap64Bit* ddr_heap2_;  // ondemand mapped off-chip memory
    Heap64Bit* msmc_heap_;  // on-chip memory
};
#endif // HEAPS_POLICY_THREAD_H_
