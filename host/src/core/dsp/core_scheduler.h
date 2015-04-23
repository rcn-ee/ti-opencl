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
#include "u_lockable.h"
#ifndef _CORE_SCHEDULER_H
#define _CORE_SCHEDULER_H

class CoreScheduler : public Lockable
{
  public:
    /*-------------------------------------------------------------------------
    * Currently limited to a max of 8 cores
    *------------------------------------------------------------------------*/
    CoreScheduler(unsigned int num_cores = 8) : p_num_cores(num_cores), p_avail(0xff) {}

    void free(int core) 
    { 
        Lock lock(this);
        p_avail |= (1 << core);
        CV.notify_one();
    }

    int allocate()
    {
        Lock lock(this);

        /*---------------------------------------------------------------------
        * Wait in a loop in case the condvar is falsely signalled
        *--------------------------------------------------------------------*/
        while (!p_avail) CV.wait(lock.raw());

        for (int i=0, mask = 1; i < p_num_cores; ++i, mask <<= 1)
            if (p_avail & mask) { p_avail &= ~mask; return i; }
    }

  private:
     unsigned int  p_num_cores;
     unsigned char p_avail;
     CondVar       CV;
};

#endif //_CORE_SCHEDULER_H
