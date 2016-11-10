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
#ifndef HEAP_MANAGER_POLICY_THREAD_H_
#define HEAP_MANAGER_POLICY_THREAD_H_

#include <map>
#include "u_locks_pthread.h"

namespace utility { 

/*-----------------------------------------------------------------------------
* Policy class for HeapManager, when multi-threaded usage is required
*----------------------------------------------------------------------------*/
template <typename Address, typename Length> struct MultiThread
{
    typedef ::Mutex                             Mutex;
    typedef ::ScopedLock                        ScopedLock;
    typedef class managed_memory                ManagedMemory;
    typedef std::pair<const Address, Length>    MapElement;
    typedef std::exception                      Exception;
    typedef std::allocator<MapElement>          MapAllocator;
    typedef MapAllocator                        AllocMapAllocator;
    typedef MapAllocator                        FreeMapAllocator;
    typedef std::map<Address, Length>           BlockList;
    typedef BlockList                           AllocBlockList;
    typedef BlockList                           FreeBlockList;

    struct managed_memory
    {
         MapAllocator get_segment_manager() { return MapAllocator(); }
    };

    static inline Length length(Length& val)       { return val; }
    static inline Length pid   (Length& )          { return 0;   }
    static inline Length alloc_entry (Length& val) { return val; }
};

} // end namespace utility

#endif // HEAP_MANAGER_POLICY_THREAD_H_
