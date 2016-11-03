#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>

#include <boost/interprocess/managed_shared_memory.hpp>
#include "cmem-allocator.h"
#include "heap_manager.h"

namespace bipc = boost::interprocess;

typedef utility::HeapManager<uint64_t, uint64_t, 
                             utility::MultiProcess<uint64_t, uint64_t> > Heap;

/*-----------------------------------------------------------------------------
* This will allocate all the memory in CMEM that is designated for the 
* multicore tools.  assert calls exist in the constructor for this object
* and so it should be constructed before the daemon call closes the 
* stdxxx file descriptors.
*----------------------------------------------------------------------------*/
CmemAllocator CT;

/*-----------------------------------------------------------------------------
* Open or create the heap manager in linux shared memory
*----------------------------------------------------------------------------*/
Heap* ddr_heap1 = nullptr;
Heap* ddr_heap2 = nullptr;
Heap* msmc_heap = nullptr;

volatile bool graceful_exit = false;
void signal_handler(int sig) { graceful_exit = true; }

/*-----------------------------------------------------------------------------
* process_exists - Does a PID represent a running process?
*----------------------------------------------------------------------------*/
bool process_exists(uint32_t pid) { return (0 == kill(pid, 0)); }


/******************************************************************************
* main
******************************************************************************/
int main()
{
   /*--------------------------------------------------------------------------
   * Create or find handles to the multicore tools device heaps
   *-------------------------------------------------------------------------*/
    try 
    {
       bipc::managed_shared_memory segment (bipc::open_or_create, "HeapManager", 
                                            64 << 10);
       ddr_heap1 = segment.find_or_construct<Heap>("ddr_heap1")(segment);
       ddr_heap2 = segment.find_or_construct<Heap>("ddr_heap2")(segment);
       msmc_heap = segment.find_or_construct<Heap>("msmc_heap")(segment);
    } 
    catch (bipc::interprocess_exception &ex)
    { 
       std::cout << ex.what() << std::endl; 
       exit(EXIT_FAILURE); 
    }

    /*-------------------------------------------------------------------------
    * Register signal handlers
    *------------------------------------------------------------------------*/
    signal(SIGINT,  signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGTERM, signal_handler);

    /*-------------------------------------------------------------------------
    * Daemonize this process
    *------------------------------------------------------------------------*/
    daemon(0,0);

    openlog("ti-mctd", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
    syslog (LOG_INFO, "started");

    /*-------------------------------------------------------------------------
    * Persist indefinitely, until a signal is caught
    *------------------------------------------------------------------------*/
    while (1) 
    {
        pause();

        if (graceful_exit)
        {
            syslog (LOG_INFO, "Graceful exit");
            bipc::shared_memory_object::remove("HeapManager");
            exit(EXIT_SUCCESS);
        }
    }
}
