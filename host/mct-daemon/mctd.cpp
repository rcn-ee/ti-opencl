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
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>
#include <getopt.h>
#include <stdio.h>

#include <boost/interprocess/managed_shared_memory.hpp>
#include "cmem_allocator.h"
#include "heap_manager.h"
#include "heap_manager_policy_process.h"

namespace bipc = boost::interprocess;

typedef utility::HeapManager<uint64_t, uint64_t, 
                             utility::MultiProcess<uint64_t, uint64_t> > Heap64;
typedef utility::HeapManager<uint32_t, uint32_t, 
                             utility::MultiProcess<uint32_t, uint32_t> > Heap32;

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
Heap64* ddr_heap1 = nullptr;
Heap64* ddr_heap2 = nullptr;
Heap64* msmc_heap = nullptr;

volatile bool graceful_exit = false;
void signal_handler(int sig) { graceful_exit = true; }

/******************************************************************************
* process_exists - Does a PID represent a running process?
******************************************************************************/
bool process_exists(uint32_t pid) { return (0 == kill(pid, 0)); }

extern void reset_dsps() __attribute__((weak));
extern void load_dsps()  __attribute__((weak));
extern void run_dsps()   __attribute__((weak));

static void process_options(int argc, char* argv[], size_t *heap_size);
static void print_usage();

/******************************************************************************
* main
******************************************************************************/
int main(int argc, char* argv[])
{
    openlog("ti-mctd", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);

    size_t heap_size = HEAP_MANAGER_DEFAULT_SIZE;
    process_options(argc, argv, &heap_size);

    /*-------------------------------------------------------------------------
    * Create or find handles to the multicore tools device heaps
    *------------------------------------------------------------------------*/
    try 
    {
       bipc::managed_shared_memory segment (bipc::open_or_create, "HeapManager",
                                            heap_size);
       ddr_heap1 = segment.find_or_construct<Heap64>("ddr_heap1")(segment);
       ddr_heap2 = segment.find_or_construct<Heap64>("ddr_heap2")(segment);
       msmc_heap = segment.find_or_construct<Heap64>("msmc_heap")(segment);
    } 
    catch (bipc::interprocess_exception &ex)
    { 
       std::cout << ex.what() << std::endl; 
       exit(EXIT_FAILURE); 
    }

    syslog (LOG_INFO, "Shared Memory heaps created, size %d bytes",
            heap_size);

    if (reset_dsps) reset_dsps();
    if (load_dsps)  load_dsps();
    if (run_dsps)   run_dsps();

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
            if (reset_dsps) reset_dsps();
            closelog();
            exit(EXIT_SUCCESS);
        }
    }
}

static void process_options(int argc, char* argv[], size_t *heap_size)
{

    static struct option long_options[] = {

        {"help",      no_argument,       0,    'h'},
        {"heap-size", required_argument, 0,    's'},
        {0,           0,                 0,     0 }
    };

    int option_index = 0;
    int c;

    while ((c=getopt_long(argc, argv, "hs:", long_options,
                          &option_index)) != -1)
    {
        switch (c)
        {
            case 's': *heap_size = atoi(optarg);
                      break;

            default:
            case 'h': print_usage();
                      exit(EXIT_SUCCESS);
        }
    }

    return;
}

static void print_usage()
{
    printf( "ti-mctd [options]\n");
    printf( "Options:\n");
    printf( "    -h, --help      : Print this help screen\n");
    printf( "    -s, --heap-size : Specify OpenCL inter process shared memory heap size in bytes\n");
    printf( "                      Heap is created in /dev/shm/HeapManager and defaults to 128KB.\n\n");
}
