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
#include <unistd.h>
#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>
#include "heap_manager.h"
#include "heap_manager_policy_process.h"

typedef utility::HeapManager<uint64_t, uint64_t, 
        utility::MultiProcess<uint64_t, uint64_t> > Heap64;

typedef utility::HeapManager<uint32_t, uint32_t, 
        utility::MultiProcess<uint32_t, uint32_t> > Heap32;

bool option_help = false;
bool option_clean = false;

/*-----------------------------------------------------------------------------
* main 
*----------------------------------------------------------------------------*/
int main (int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "ch")) != -1)
    {
       switch (c)
       { 
          case 'c': option_clean = true; break;
          case 'h': option_help  = true; break;
          default:  
             std::cout << "Unknown option specified: " << c << std::endl; break;
       }
    }

    if (option_help)
    {
        std::cout << argv[0] << " [option]" << std::endl;
        std::cout << "  -h: Print help screen" << std::endl;
        std::cout << "  -c: Clean heaps of non-existant processes" << std::endl;
    }

    try
    {
       /*----------------------------------------------------------------------
       * Create a named shared memory segment 
       *---------------------------------------------------------------------*/
        boost::interprocess::managed_shared_memory segment 
                              (boost::interprocess::open_only, "HeapManager");

        Heap64* ddr_heap1 = segment.find_or_construct<Heap64>("ddr_heap1")
                                                             (segment);
        Heap64* ddr_heap2 = segment.find_or_construct<Heap64>("ddr_heap2")
                                                             (segment);
        Heap64* msmc_heap = segment.find_or_construct<Heap64>("msmc_heap")
                                                             (segment);

        if (option_clean)
        {
           if (ddr_heap1 && ddr_heap1->size() > 0) ddr_heap1->garbage_collect();
           if (ddr_heap2 && ddr_heap2->size() > 0) ddr_heap2->garbage_collect();
           if (msmc_heap && msmc_heap->size() > 0) msmc_heap->garbage_collect();
        }

        if (ddr_heap1 && ddr_heap1->size() > 0) ddr_heap1->dump("ddr_heap1");
        if (ddr_heap2 && ddr_heap2->size() > 0) ddr_heap2->dump("ddr_heap2");
        if (msmc_heap && msmc_heap->size() > 0) msmc_heap->dump("msmc_heap");
    }
    catch(...)
    {
        std::cout << "TI Multicore Tools Heaps do not exist" << std::endl;
    }

    return 0;
}
