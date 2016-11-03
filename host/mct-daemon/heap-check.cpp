#include <unistd.h>
#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>
#include "heap_manager.h"

typedef utility::HeapManager<uint64_t, uint64_t, utility::MultiProcess<uint64_t, uint64_t> > Heap;

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

        Heap* ddr_heap1 = segment.find_or_construct<Heap>("ddr_heap1")(segment);
        Heap* ddr_heap2 = segment.find_or_construct<Heap>("ddr_heap2")(segment);
        Heap* msmc_heap  = segment.find_or_construct<Heap>("msmc_heap")(segment);

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
}
