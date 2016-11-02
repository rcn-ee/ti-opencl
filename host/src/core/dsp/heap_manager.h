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
#ifndef HEAP_MANAGER_H_
#define HEAP_MANAGER_H_

#include <cstdio>
#include <cstdlib>
#include <inttypes.h>
#include <unistd.h>

#include <map>
#include <unordered_set>
#include <iostream>
#include <iomanip>

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

/*-----------------------------------------------------------------------------
* Policy class for HeapManager, when multi-threaded usage is required
*----------------------------------------------------------------------------*/
template <typename Address, typename Length> struct MultiThread
{
    typedef boost::mutex                        Mutex;
    typedef boost::mutex::scoped_lock           ScopedLock;
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


/*-----------------------------------------------------------------------------
* HeapManager - manage an out of line heap allocator, meaning that the heap 
*    control structures are in a separate memory space from the underlying 
*    managed memory.
*
* Usage:
*
*    typedef uint64_t Address;
*    typedef uint64_t Length;
*
* For a shared memory heap manager
*
*    boost::interprocess::managed_shared_memory segment 
*              (boost::interprocess::open_or_create, "HeapManager", 256 << 10);
* 
*    typedef HeapManager<Address, Length, MultiProcess<Address,Length>> DdrHeap;
*    DdrHeap* ddr_heap= segment.find_or_construct<DdrHeap>("ddr_heap")(segment);
* 
* For a single process multi threaded heap manager
*
*    typedef HeapManager<Address, Length, MultiThread<Address,Length>> DdrHeap;
*    DdrHeap* ddr_heap= new DdrHeap;
* 
*----------------------------------------------------------------------------*/
template <typename Address, typename Length, typename Scope> class HeapManager
{
  public:

    typedef typename Scope::Mutex             Mutex;
    typedef typename Scope::ScopedLock        ScopedLock;
    typedef typename Scope::ManagedMemory     ManagedMemory;
    typedef typename Scope::Exception         Exception;
    typedef typename Scope::AllocMapAllocator AllocMapAllocator;
    typedef typename Scope::FreeMapAllocator  FreeMapAllocator;
    typedef typename Scope::AllocBlockList    AllocBlockList;
    typedef typename Scope::FreeBlockList     FreeBlockList;
  
    /*-------------------------------------------------------------------------
    * CTOR - Used for Scope = MultiProcess
    *------------------------------------------------------------------------*/
    HeapManager (const ManagedMemory& segment) :
       alloc_map_allocator_            (segment.get_segment_manager()),
       free_map_allocator_             (segment.get_segment_manager()),
       free_list_                      (std::less<Address>(), free_map_allocator_),
       alloc_list_                     (std::less<Address>(), alloc_map_allocator_),
       start_addr_                     (0),
       length_                         (0),
       min_block_size_                 (0),
       available_                      (0),
       addrs_need_pow2_size_alignment_ (false),
       max_pow2_size_alignment_        (0) {}

    /*-------------------------------------------------------------------------
    * CTOR - Used for Scope = MultiThread
    *------------------------------------------------------------------------*/
    HeapManager () :
       alloc_map_allocator_            (),
       free_map_allocator_             (),
       free_list_                      (),
       alloc_list_                     (),
       start_addr_                     (0),
       length_                         (0),
       min_block_size_                 (0),
       available_                      (0),
       addrs_need_pow2_size_alignment_ (false),
       max_pow2_size_alignment_        (0) {}

    /*-------------------------------------------------------------------------
    * DTOR
    *------------------------------------------------------------------------*/
    ~HeapManager () {}

    /*-------------------------------------------------------------------------
    * malloc
    *------------------------------------------------------------------------*/
    Address malloc (Length size, bool allow_fail = false)
    {
        Length align = min_block_size_;
        size         = min_block_size(size);
   
        /*---------------------------------------------------------------------
        * If an alignment greater than the standard alignment is required
        *--------------------------------------------------------------------*/
        if (addrs_need_pow2_size_alignment_)
        {
            if (size >= max_pow2_size_alignment_) 
                 align = max_pow2_size_alignment_;
            else align  = next_power_of_two(size);
        }

        try 
        {
            ScopedLock lock(mutex_);

            /*-----------------------------------------------------------------
            * Find a free block large enough to accomodate this allocation 
            *----------------------------------------------------------------*/
            for (auto& kv : free_list_)
            {
                Address avail_addr = kv.first;
                Length  avail_size = kv.second;

                Address align_addr = roundup(avail_addr, (Address)align);
                Length  align_gap  = align_addr - avail_addr;

                /*-------------------------------------------------------------
                * Adjust avail_size for any alignment gap
                *------------------------------------------------------------*/
                if (align_gap < avail_size) avail_size -= align_gap;
                else                        avail_size = 0;

                avail_addr  = align_addr;

                /*-------------------------------------------------------------
                * if this free block will not satisfy the request, keep looking
                *------------------------------------------------------------*/
                if (avail_size < size) continue;

                /*-------------------------------------------------------------
                * if there is unused space at the beginning of this free block
                * due to alignment restrictions, then update this free block,
                * otherwise remove this free block from the list.
                *------------------------------------------------------------*/
                if (align_gap) kv.second = align_gap;
                else           free_list_.erase(kv.first);

                /*-------------------------------------------------------------
                * add the allocated block to the alloc_list
                *------------------------------------------------------------*/
                //PID alloc_list_[avail_addr] = size;
                alloc_list_[avail_addr] = Scope::alloc_entry(size);

                /*-------------------------------------------------------------
                * if there is remaining space at the end of the free block,
                * Add that back to the free list.
                *------------------------------------------------------------*/
                if (avail_size > size)
                    free_list_[avail_addr + size] = avail_size - size;

                available_ -= size;

                return avail_addr;
            }

            /*-----------------------------------------------------------------
            * If the client does not want silent failure with a return of 0
            *----------------------------------------------------------------*/
            if (!allow_fail)
            {
                std::cout << std::hex 
                          << "Malloc failed for size 0x" << size 
                          << " from range (0x"           << start_addr_
                          << ", 0x"                      << start_addr_ + length_ - 1
                          << ")"                         << std::endl << std::dec;

                exit(EXIT_FAILURE);
            }

            /*-----------------------------------------------------------------
            * return NULL if no memory can be found
            *----------------------------------------------------------------*/
            return 0;
        }
        catch(Exception &ex)
            { std::cout << ex.what() << std::endl; exit(EXIT_FAILURE); }
    }

    /*-------------------------------------------------------------------------
    * free
    *------------------------------------------------------------------------*/
    int free(Address addr)
    {
        try 
        {
            ScopedLock lock(mutex_);

            /*-----------------------------------------------------------------
            * Nothing to do, if addr is not an allocated address
            *----------------------------------------------------------------*/
            auto it = alloc_list_.find(addr);
            if (it == alloc_list_.end()) return -1;

            // PID Length size = it->second;
            Length size = Scope::length(it->second);

            /*-----------------------------------------------------------------
            * Remove the block from the allocated list and return the # of 
            * bytes to the global size
            *----------------------------------------------------------------*/
            alloc_list_.erase(it);
            available_ += size;

            /*-----------------------------------------------------------------
            * set the iterator to the free block with the greatest address less
            * than the address we are returning to the free list. If there is
            * no such address then start at the first free block.
            *----------------------------------------------------------------*/
            auto after_block = free_list_.lower_bound(addr);
            auto prior_block(after_block);
            if (prior_block != free_list_.begin()) prior_block--;

            /*-----------------------------------------------------------------
            * We are returning memory to the free list.  There are 4 cases in 
            * which this can happen.  Using P to represent the prior block,
            * A to represent the after block and T to represent this block that 
            * is being returned to the free list, the free can change 
            * P A  to  P T A, PT A, P TA, or PTA, where a space indicates 
            * a separate block in the free list and no space indicates a merged
            * block
            *----------------------------------------------------------------*/
            bool can_merge_prior = (prior_block->first + prior_block->second == addr);
            bool can_merge_after = (addr + size == after_block->first);

            if (can_merge_after) size += after_block->second;
            if (can_merge_prior) prior_block->second += size;
            else                 free_list_[addr] = size;
            if (can_merge_after) free_list_.erase(after_block);

            return 0;
        }
        catch(Exception &ex)
            { std::cout << ex.what() << std::endl; exit(EXIT_FAILURE); }
    }

   /*-------------------------------------------------------------------------
    * max_block_size - What is the largest block of memory that can be allocated
    *------------------------------------------------------------------------*/
    Address max_block_size(Length &size, uint32_t &block_size)
    { 
        block_size = min_block_size_;

        if (length_ < min_block_size_) 
        {
            size = 0;
            return 0;
        }

        Address max_avail_addr = 0;
        Length  max_avail_size = 0;

        try
        {
            ScopedLock lock(mutex_);

            for (const auto& kv : free_list_)
            {
                Address avail_addr = kv.first;
                Length  avail_size = kv.second;

                if (avail_size >= max_avail_size)
                {
                    max_avail_addr = avail_addr;
                    max_avail_size = avail_size;
                }
            }

            size = max_avail_size;
            return max_avail_addr;
        }
        catch(Exception &ex)
            { std::cout << ex.what() << std::endl; exit(EXIT_FAILURE); }
    }

    /*-------------------------------------------------------------------------
    * size - what was the original size of the heap
    *------------------------------------------------------------------------*/
    Length size() const 
    { 
        return length_;    
    }

    /*-------------------------------------------------------------------------
    * available - How much memory is available in the heap, albeit with holes.
    *------------------------------------------------------------------------*/
    Length available() 
    { 
        try
        {
            ScopedLock lock(mutex_);
            return available_; 
        }
        catch(Exception &ex)
            { std::cout << ex.what() << std::endl; exit(EXIT_FAILURE); }
    }

    /*-------------------------------------------------------------------------
    * HeapManager::configure - Allow configuration of the heap after 
    *    construction.  
    *------------------------------------------------------------------------*/
    void configure(Address start_addr, Length length, Length min_block_size)
    {
        try
        {
            ScopedLock lock(mutex_);

            /*-----------------------------------------------------------------
            * If not already initialized and length > 0
            *----------------------------------------------------------------*/
            if (length_ == 0 && length > 0)
            {
                length_                = length;
                available_             = length;
                start_addr_            = start_addr;
                min_block_size_        = min_block_size;
                free_list_[start_addr] = length;
            }
        }

        catch(Exception &ex)
            { std::cout << ex.what() << std::endl; exit(EXIT_FAILURE); }
    }

    /*---------------------------------------------------------------------
    * HeapManager::set_additional_alignment_requirements
    *
    *  Some heaps may require power of two alignment for every allocation.
    *  If so, then this configuration API can be used to specify that 
    *  behavior.  The argument gives a maximum required alignment.
    *--------------------------------------------------------------------*/
    void set_additional_alignment_requirements(Length max_pow2_size_alignment)
    {
        try
        {
            ScopedLock lock(mutex_);

            if (addrs_need_pow2_size_alignment_ == false && alloc_list_.empty())
            {
                addrs_need_pow2_size_alignment_ = true;
                max_pow2_size_alignment_        = max_pow2_size_alignment;
            }
        }
        catch(Exception &ex)
            { std::cout << ex.what() << std::endl; exit(EXIT_FAILURE); }
    }

    /*-------------------------------------------------------------------------
    * HeapManager::garbage_collect 
    *
    *   Remove and allocated blocks associated with the provided PID.
    *------------------------------------------------------------------------*/
    void garbage_collect(uint32_t pid)
    {
        try
        {
            std::unordered_set<Address> need_free;

            ScopedLock lock(mutex_);
            for (auto ab : alloc_list_)
                if (pid == Scope::pid(ab.second)) need_free.insert(ab.first);

            /*-----------------------------------------------------------------
            * Unlock the lock so the free calls can get the lock
            *----------------------------------------------------------------*/
            lock.unlock();

            for (auto nf : need_free) free(nf);
        }
        catch(Exception &ex)
            { std::cout << ex.what() << std::endl; exit(EXIT_FAILURE); }
    }

    /*-------------------------------------------------------------------------
    * HeapManager::dump
    *------------------------------------------------------------------------*/
    void dump(void) 
    {
        try
        {
            ScopedLock lock(mutex_);

            std::cout << std::hex;
            std::cout << "-- HeapManager ------------------------------" << std::endl;
            std::cout << "   Addr: 0x" << start_addr_ << std::endl;
            std::cout << "   Size: 0x" << length_     << std::endl;
            std::cout << "   Algn: 0x" << min_block_size_ << std::endl;
            std::cout << "   Pow2: " << addrs_need_pow2_size_alignment_ << std::endl;
            std::cout << "   Free: " << std::endl;

            for (auto& kv : free_list_)
                std::cout << "        addr: 0x" << kv.first << " size: 0x" << kv.second << std::endl;

            std::cout << "   Aloc: " << std::endl;

            for (auto& kv : alloc_list_)
                std::cout << "        addr: 0x" << kv.first << " size: 0x" << Scope::length(kv.second)
                          << std::dec << " pid: " << Scope::pid(kv.second) << std::hex << std::endl;

            std::cout << "-----------------------------------------" << std::endl;
            std::cout << std::dec;
        }
        catch(Exception &ex)
            { std::cout << ex.what() << std::endl; exit(EXIT_FAILURE); }
    }

  private:

    AllocMapAllocator alloc_map_allocator_;
    FreeMapAllocator  free_map_allocator_;
    FreeBlockList     free_list_;
    AllocBlockList    alloc_list_;
    Address           start_addr_;
    Length            length_;
    Length            min_block_size_;
    Length            available_;
    bool              addrs_need_pow2_size_alignment_;
    Length            max_pow2_size_alignment_;
    Mutex             mutex_;

    /*-------------------------------------------------------------------------
    * min_block_size - all allocations are at least the minimum block size
    *------------------------------------------------------------------------*/
    Length min_block_size (Length size) const
    { 
        return roundup(size, min_block_size_); 
    }

    /*-------------------------------------------------------------------------
    * roundup
    *------------------------------------------------------------------------*/
    template <typename T> T roundup(T val, T pow2) const
    {
        return ((val + pow2 - (T)1) & ~(pow2 - (T)1));
    }

    /*-------------------------------------------------------------------------
    * next_power_of_two - Round argument up to next power of two value.
    * www.wikipedia.org/wiki/Power_of_two#Algorithm_to_round_up_to_power_of_two
    *------------------------------------------------------------------------*/
    template <typename T> T next_power_of_two(T k) const
    {
        if (k == 0) return 1;

        k--;
        T bits_in_T = sizeof(T) * 8;

        for (T i = 1; i < bits_in_T; i <<= 1)
            k = k | k >> i;

        return k + 1;
    }
};

} // end namespace utility

#endif // HEAP_MANAGER_H_
