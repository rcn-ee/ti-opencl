/**************************************************************************//**
*
*  Copyright (c) 2012, Texas Instruments Incorporated
*
*  All rights reserved. Property of Texas Instruments Incorporated.
*  Restricted rights to use, duplicate or disclose this code are
*  granted through contract.
*
*  ============================================================================
*
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
#include <iostream>
#include "u_lockable.h"
#include "ocl_debug.h"

#define ROUNDUP(val, pow2)   (((val) + (pow2) - 1) & ~((pow2) - 1))
#define MIN_BLOCK_SIZE       128

class dspheap : public Lockable
{
  typedef std::map<DSPDevicePtr, size_t> block_list;
  typedef block_list::iterator           block_iter;
  typedef block_list::value_type         block_descriptor;

  public:
    dspheap(DSPDevicePtr start_addr, size_t length)
    {
        /*---------------------------------------------------------------------
        * Ensure that the start_addr and length are multiples of 16M.
        * 16M is the granularity of a memory region that can be controlled
        * by a MAR register of C6x.
        *--------------------------------------------------------------------*/
        //assert((length             & 0xFFFFFF) == 0);
        //assert(((size_t)start_addr & 0xFFFFFF) == 0);

        Lock lock(this);
        if (free_list.empty())
            free_list[start_addr] = length;
    }

    ~dspheap() { }

    DSPDevicePtr malloc(size_t size)
    {
        size = min_block_size(size);

        Lock lock(this);
        for (block_iter it = free_list.begin(); it != free_list.end(); ++it)
        {
            DSPDevicePtr block_addr =  (*it).first;
            size_t block_size =  (*it).second;

            if (block_size >= size)
            {
                free_list.erase(it);
                alloc_list[block_addr] = size;

                /*-------------------------------------------------------------
                * if we only use a portion of the free block
                *------------------------------------------------------------*/
                if (block_size > size)
                    free_list[(DSPDevicePtr)block_addr+size] = block_size-size;

                ocl_debug2("Allocated DSP memory at 0x%08x with size = 0x%x", 
                           (uint32_t)block_addr, size);
                return block_addr;
            }
        }
        return 0;
    }

    void free (DSPDevicePtr addr)
    {
        /*---------------------------------------------------------------------
        * Nothing to do if not an allocated address
        *--------------------------------------------------------------------*/
        Lock lock(this);
        block_iter it = alloc_list.find(addr);
        if (it == alloc_list.end()) return;

        size_t size = (*it).second;
        free_list[addr] = size;
        alloc_list.erase(it);

        coalesce_free_list();
    }

  private:
    block_list free_list;
    block_list alloc_list;

    size_t min_block_size(size_t size) { return ROUNDUP(size, MIN_BLOCK_SIZE); }

    void coalesce_free_list()
    {
        // ASW TODO 
    }
};

#endif // _DSPHEAP_H
