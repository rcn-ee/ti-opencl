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
#ifndef HEAPS_POLICY_PROCESS_H_
#define HEAPS_POLICY_PROCESS_H_

#include <unistd.h>
#include "heap_manager.h"
#include "heap_manager_policy_process.h"

/******************************************************************************
* This file contains a policy classes for controlling typedefs, construction
* and destruction of HeapManager heaps for OpenCL use.  This policy class 
* creates heap managers in linux shared memory and can be shared across 
* processes.  The actual underlying memory managed by the HeapManager is 
* currently being allocated by a daemon for OpenCL under Linux.
******************************************************************************/

/******************************************************************************
* class HeapsMultiProcessPolicy
*   Create heap managers in linux shared memory and share across processes.
******************************************************************************/
class HeapsMultiProcessPolicy
{
public:
    typedef utility::HeapManager<DSPDevicePtr, uint32_t, 
            utility::MultiProcess<DSPDevicePtr, uint32_t> > Heap32Bit;

    typedef utility::HeapManager<DSPDevicePtr64, uint64_t, 
            utility::MultiProcess<DSPDevicePtr64, uint64_t> > Heap64Bit;

    /*-------------------------------------------------------------------------
    * A shared memory segment named "HeapManager" will be opened and will
    * contain the heap managers for all needed OpenCL buffer heaps. It will 
    * reside on the file system at /dev/shm/HeapManager. It is assumed this 
    * shared memory is created by a pre-existing daemon on Linux systems.
    * Note: Using open_or_create to avoid a throw from the constructor when
    * the opencl application is run without starting ti-mctd.
    *------------------------------------------------------------------------*/
    HeapsMultiProcessPolicy() :
       segment_(boost::interprocess::open_or_create, "HeapManager",
                HEAP_MANAGER_DEFAULT_SIZE),
       ddr_heap1_(segment_.find_or_construct<Heap64Bit>("ddr_heap1")(segment_)),
       ddr_heap2_(segment_.find_or_construct<Heap64Bit>("ddr_heap2")(segment_)),
       msmc_heap_(segment_.find_or_construct<Heap64Bit>("msmc_heap")(segment_))
    {}

    /*-------------------------------------------------------------------------
    * When this class is destructed, we guarantee that no allocations by this 
    * process will remain in the shared heap managers.
    *------------------------------------------------------------------------*/
    ~HeapsMultiProcessPolicy()
    {
	uint32_t pid = getpid();
	msmc_heap_->free_all_pid(pid);
	ddr_heap1_->free_all_pid(pid);
	ddr_heap2_->free_all_pid(pid);
    }

private:
    boost::interprocess::managed_shared_memory segment_;

protected:
    Heap64Bit*  ddr_heap1_;  // persistently mapped off-chip memory
    Heap64Bit*  ddr_heap2_;  // ondemand mapped off-chip memory
    Heap64Bit*  msmc_heap_;  // on-chip memory
};

#endif // HEAPS_POLICY_PROCESS_H_
