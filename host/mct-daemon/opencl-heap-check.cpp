#include <boost/interprocess/managed_shared_memory.hpp>
#include "heap_manager.h"
#include <iostream>

typedef utility::HeapManager<uint64_t, uint64_t, utility::MultiProcess<uint64_t, uint64_t> > Heap;

/*-----------------------------------------------------------------------------
* main 
*----------------------------------------------------------------------------*/
int main ()
{
   /*--------------------------------------------------------------------------
   * Create a named shared memory segment 
   *-------------------------------------------------------------------------*/
    boost::interprocess::managed_shared_memory segment (boost::interprocess::open_only, "HeapManager");
    Heap* ddr_heap1 = segment.find_or_construct<Heap>("ddr_heap1")(segment);
    Heap* ddr_heap2 = segment.find_or_construct<Heap>("ddr_heap2")(segment);
    Heap* msmc_heap  = segment.find_or_construct<Heap>("msmc_heap")(segment);

    if (ddr_heap1)
    {
        std::cout << "ddr_heap1 " << std::endl;
        ddr_heap1->dump();
        std::cout << std::endl;
    }
    if (ddr_heap2)
    {
        std::cout << "ddr_heap2 " << std::endl;
        ddr_heap2->dump();
        std::cout << std::endl;
    }
    if (msmc_heap)
    {
        std::cout << "msmc_heap " << std::endl;
        msmc_heap->dump();
        std::cout << std::endl;
    }
}
