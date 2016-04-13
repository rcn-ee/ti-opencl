/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
*  @file    dspheap.h
*
*  @brief   Define a dsp device heap manager run on the host.
*
*  @version 1.00.00
*
******************************************************************************/
#ifndef _DSPHEAP_H
#define _DSPHEAP_H
#include <map>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include "u_lockable.h"
#include "dspmem.h"

class dspheap : public Lockable
{
  typedef std::map<DSPDevicePtr64, uint64_t> block_list;
  typedef block_list::iterator               block_iter;
  typedef block_list::const_iterator         const_block_iter;
  typedef block_list::value_type             block_descriptor;

  public:
    dspheap(DSPDevicePtr64 start_addr, uint64_t length) 
    {
        configure(start_addr, length);
    }

    dspheap() { }

    void configure(DSPDevicePtr64 start_addr, uint64_t length,
                   bool is_cmem_ondemand_heap = false)
    {
        /*---------------------------------------------------------------------
        * Ensure that the start_addr and length are multiples of 16M.
        * 16M is the granularity of a memory region that can be controlled
        * by a MAR register of C6x.
        *--------------------------------------------------------------------*/
        //assert((length             & 0xFFFFFF) == 0);
        //assert(((uint32_t)start_addr & 0xFFFFFF) == 0);
        
        p_start_addr = start_addr;
        p_length     = length;
        p_available  = length;
        p_block_size = is_cmem_ondemand_heap ? MIN_CMEM_ONDEMAND_BLOCK_SIZE
                                             : MIN_BLOCK_SIZE;
        p_map_align  = is_cmem_ondemand_heap;

        Lock lock(this);
        if (free_list.empty())
            free_list[start_addr] = length;
    }

    ~dspheap() { }

    DSPDevicePtr64 malloc(uint32_t size, bool allow_fail=false)
    {
        size = min_block_size(size);

        uint32_t align = p_block_size;
        if (p_map_align)
            align = (size >= MAX_CMEM_MAP_ALIGN) ? MAX_CMEM_MAP_ALIGN
                                                 : next_power_of_two(size);

        Lock lock(this);
        for (block_iter it = free_list.begin(); it != free_list.end(); ++it)
        {
            DSPDevicePtr64 block_addr =  (*it).first;
            uint64_t block_size =  (*it).second;

            if (block_size >= size)
            {
                bool update_insteadof_remove = false;
                if (p_map_align)
                {
                    DSPDevicePtr64 align_addr = ROUNDUP(block_addr,
                                                        (DSPDevicePtr64)align);
                    if (block_size < align_addr - block_addr + size)  continue;

                    /*---------------------------------------------------------
                    * if align_addr does not start at block_addr, add the
                    * (align_addr - block_addr) portion back to free list
                    * by updating the current entry to the new size
                    *--------------------------------------------------------*/
                    if (align_addr > block_addr)
                    {
                        free_list[block_addr]   = align_addr - block_addr;
                        update_insteadof_remove = true;
                        block_size -= (align_addr - block_addr);
                        block_addr  = align_addr;
                    }
                }

                if (! update_insteadof_remove)  free_list.erase(it);

                alloc_list[block_addr] = size;

                /*-------------------------------------------------------------
                * if we only use a portion of the free block
                *------------------------------------------------------------*/
                if (block_size > size)
                    free_list[(DSPDevicePtr64)block_addr+size] = block_size-size;

                p_available -= size;
                return block_addr;
            }
        }

        if (!allow_fail)
        {
            printf("Malloc failed for size 0x%x from range (0x%08llx, 0x%08llx)\n",
                   size, p_start_addr, p_start_addr+p_length-1);
            abort();
        }
        
        return 0;
    }

    int free(DSPDevicePtr64 addr)
    {
        /*---------------------------------------------------------------------
        * Nothing to do if not an allocated address
        *--------------------------------------------------------------------*/
        Lock lock(this);
        block_iter it = alloc_list.find(addr);
        if (it == alloc_list.end()) return -1;

        uint32_t size = (*it).second;
        alloc_list.erase(it);
        p_available += size;

        /*---------------------------------------------------------------------
        * Merge the block with neighboring free blocks
        *--------------------------------------------------------------------*/
        it = free_list.begin();
        while (it != free_list.end())
        {
            DSPDevicePtr64 block_addr =  (*it).first;
            uint64_t       block_size =  (*it).second;

            if (   block_addr + block_size == addr
                || addr + size == block_addr)
            {
                block_iter merge_it = it;
                if (block_addr < addr) addr = block_addr;
                size = block_size + size;
                ++it;
                free_list.erase(merge_it);
                continue;
            }
            ++it;
        }
        free_list[addr] = size;
        return 0;
    }

    DSPDevicePtr64 size()      const { return p_length;    }
    DSPDevicePtr64 available() const { Lock lock(this);  return p_available; }

    DSPDevicePtr64 max_block_size(uint64_t &size, uint32_t &block_size) const
    { 
        if (p_length < p_block_size) 
        {
            block_size = p_block_size;
            size = 0;
            return 0;
        }

        DSPDevicePtr64 max_block_addr = 0;
        uint64_t       max_block_size = p_block_size;

        Lock lock(this);
        for (const_block_iter it = free_list.begin(); it != free_list.end(); ++it)
        {
            DSPDevicePtr64 block_addr =  (*it).first;
            uint64_t       block_size =  (*it).second;

            if (block_size >= max_block_size)
            {
                max_block_addr = block_addr;
                max_block_size = block_size;
            }
        }

        block_size = p_block_size;
        size = max_block_size;
        return max_block_addr;
    }

  private:
    block_list     free_list;
    block_list     alloc_list;
    DSPDevicePtr64 p_start_addr;
    uint64_t       p_length;
    uint32_t       p_block_size;
    uint64_t       p_available;
    bool           p_map_align;

    uint32_t min_block_size(uint32_t size) { return ROUNDUP(size, p_block_size); }

    // From Wikipedia : http://www.wikipedia.org/wiki/Power_of_two#Algorithm_to_round_up_to_power_of_two
template <class T>
T next_power_of_two(T k)
{
    if (k == 0) return 1;

    k--;
    for (int i=1; i<sizeof(T)*8; i<<=1)
            k = k | k >> i;
    return k+1;
}

};

#endif // _DSPHEAP_H
