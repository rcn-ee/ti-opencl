/******************************************************************************
 * Copyright (c) 2013-2018, Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef _CORE_SCHEDULER_H
#define _CORE_SCHEDULER_H

#include <assert.h>
#include <stdint.h>

#include <algorithm>
#include <map>
#include <iostream>

#include "u_lockable.h"
#include "../tiocl_types.h"


#define CORE_SCHEDULER_DEFAULT_DEPTH    4

/******************************************************************************
* CoreScheduler :
******************************************************************************/
class CoreScheduler : public Lockable
{
public:
    CoreScheduler(DSPCoreSet& compute_units, uint32_t depth)
        : depth_(depth)
    {
        for (auto & core : compute_units)
        {
            count_[uint32_t(core)] = 0;
        }
    }

    /*-------------------------------------------------------------------------
    * Do not allow default construction, copy construction or assignment
    *------------------------------------------------------------------------*/
    CoreScheduler()                                                 = delete;
    CoreScheduler(const CoreScheduler&)                             = delete;
    CoreScheduler& operator=(const CoreScheduler&)                  = delete;

    /*-------------------------------------------------------------------------
    * allocate any available core in the given set of compute units
    *------------------------------------------------------------------------*/
    uint32_t allocate(const DSPCoreSet& compute_units)
    {
        Lock lock(this);
        assert(compute_units.size() > 0);
        while (!any_core_available(compute_units)) cv_.wait(lock.raw());
        uint32_t core = core_select(compute_units);
        core_reserve(core);
        return core;
    }

    /*-------------------------------------------------------------------------
    * allocate a particular core
    *------------------------------------------------------------------------*/
    uint32_t allocate(uint32_t core)
    {
        Lock lock(this);
        while (!core_available(core)) cv_.wait(lock.raw());
        core_reserve(core);
        return core;
    }

    /*-------------------------------------------------------------------------
    * free a core
    *------------------------------------------------------------------------*/
    void free(uint32_t core)
    {
        Lock lock(this);
        /* Check if given core is in one of the compute units for this device */
        assert(count_.find(core) != count_.end());
        core_release(core);
        cv_.notify_one();
    }

    /*-------------------------------------------------------------------------
    * Class private data and functions
    *------------------------------------------------------------------------*/
private:
    uint32_t                  depth_;
    std::map<uint32_t, int>   count_;
    CondVar                   cv_;

    /*------------------------------------------------------------------------
    * These are only called from public member functions who have already
    * locked a mutex, so no mutex locking is needed here.
    *-----------------------------------------------------------------------*/
    uint32_t any_core_available(const DSPCoreSet& compute_units)
    {
        int32_t total_capacity = compute_units.size() * depth_;
        int32_t current_capacity = 0;
        for (auto & core : compute_units)
        {
            current_capacity += count_[uint32_t(core)];
        }
        return current_capacity < total_capacity;
    }
    uint32_t core_available(uint32_t core)  {return count_[core] < depth_;}
    void     core_reserve(uint32_t core)    {count_[core]++;}
    void     core_release(uint32_t core)    {count_[core]--;}
    uint32_t core_select(const DSPCoreSet& compute_units)
    {
        uint32_t min_depth = depth_;
        uint32_t min_core = *compute_units.begin();
        for (auto & core : compute_units)
        {
            if (count_[(uint32_t) core] < min_depth)
            {
                min_depth = count_[(uint32_t) core];
                min_core  = (uint32_t) core;
            }
        }
        return min_core;
    }
};

#endif //_CORE_SCHEDULER_H
