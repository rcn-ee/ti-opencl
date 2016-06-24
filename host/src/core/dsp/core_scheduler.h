/******************************************************************************
* Copyright (c) 2012-2016, Texas Instruments Incorporated
* All rights reserved.
*
* Property of Texas Instruments Incorporated. Restricted rights to use,
* duplicate or disclose this code are granted through contract.
******************************************************************************/
#ifndef _CORE_SCHEDULER_H
#define _CORE_SCHEDULER_H

#include <assert.h>
#include <stdint.h>

#include <algorithm>
#include <array>
#include <iostream>

#include "u_lockable.h"

/******************************************************************************
* CoreScheduler : 
******************************************************************************/
class CoreScheduler : public Lockable
{
  public:
    CoreScheduler(uint32_t num_cores, uint32_t depth)
        : num_cores_(num_cores), depth_(depth), total_count_(0)
    { count_.fill(0); }

    /*-------------------------------------------------------------------------
    * Do not allow default construction, copy construction or assignment
    *------------------------------------------------------------------------*/
    CoreScheduler() =delete;
    CoreScheduler(const CoreScheduler&) =delete;
    CoreScheduler& operator=(const CoreScheduler&) =delete;

    /*-------------------------------------------------------------------------
    * allocate any available core
    *------------------------------------------------------------------------*/
    uint32_t allocate()
    {
        Lock lock(this);

        while (!any_core_available()) cv_.wait(lock.raw());
        uint32_t core = core_select();
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

        assert(core < num_cores_);
        core_release(core);
        cv_.notify_one();
    }

    /*-------------------------------------------------------------------------
    * Class private data and functions
    *------------------------------------------------------------------------*/
  private:
     uint32_t                  num_cores_;
     uint32_t                  depth_;
     uint32_t                  total_count_;
     std::array<uint32_t, 32>  count_;
     CondVar                   cv_;

     /*------------------------------------------------------------------------
     * These are only called from public member functions who have already 
     * locked a mutex, so no mutex locking is needed here.
     *-----------------------------------------------------------------------*/
     uint32_t any_core_available() const { return total_count_ < (num_cores_ * depth_); }
     uint32_t core_available(uint32_t core) { return count_[core] < depth_; }
     void     core_reserve  (uint32_t core) { count_[core]++; total_count_++; }
     void     core_release  (uint32_t core) { count_[core]--; total_count_--; }
     uint32_t core_select() const
     {
         auto it  = std::min_element(count_.begin(), count_.begin() + num_cores_);
         return std::distance(count_.begin(), it);
     }
};

#endif //_CORE_SCHEDULER_H
