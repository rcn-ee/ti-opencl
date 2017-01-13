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
/**************************************************************************//**
*  @file    heap_manager.h
*
*  @brief   Defines an out of line heap manager.
*           It can exist in shared memory so that multiple clients can share
*           it's structure, or it can simply be multithread safe within a 
*           single process.
*
*  @version 2.00.00
*
******************************************************************************/
#ifndef HEAP_MANAGER_POLICY_PROCESS_H_
#define HEAP_MANAGER_POLICY_PROCESS_H_

#include <boost/thread/mutex.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/map.hpp>

namespace utility { 

/*-----------------------------------------------------------------------------
* Create the alias BIP for boost:interprocess, but make it only visible in this 
* header by placing it in an anonymous namespace.
*----------------------------------------------------------------------------*/
namespace { namespace BIP = boost::interprocess; }

/*-----------------------------------------------------------------------------
* Policy class for HeapManager, when cross inter-process usage is required
*----------------------------------------------------------------------------*/
template <typename Address, typename Length> struct MultiProcess
{
    typedef BIP::interprocess_mutex                         Mutex;
    typedef BIP::scoped_lock<Mutex>                         ScopedLock;
    typedef BIP::managed_shared_memory                      ManagedMemory;
    typedef ManagedMemory::segment_manager                  SegmentManager;
    typedef BIP::interprocess_exception                     Exception;
    typedef std::pair<Length, uint32_t>                     AllocMapElementVal;
    typedef std::pair<const Address, AllocMapElementVal>    AllocMapElement;
    typedef BIP::allocator<AllocMapElement, SegmentManager> AllocMapAllocator;
    typedef Length                                          FreeMapElementVal;
    typedef std::pair<const Address, FreeMapElementVal>     FreeMapElement;
    typedef BIP::allocator<FreeMapElement, SegmentManager>  FreeMapAllocator;

    typedef BIP::map<Address, AllocMapElementVal, 
              std::less<Address>, const AllocMapAllocator>  AllocBlockList;

    typedef BIP::map<Address, FreeMapElementVal, 
              std::less<Address>, const FreeMapAllocator>   FreeBlockList;

    static inline Length length(AllocMapElementVal& val) { return val.first;  }
    static inline Length pid   (AllocMapElementVal& val) { return val.second; }
    static inline AllocMapElementVal alloc_entry (Length& val) 
                                  { return AllocMapElementVal(val, getpid()); }
};

} // end namespace utility

#endif // HEAP_MANAGER_POLICY_PROCESS_H_
