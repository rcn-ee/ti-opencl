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
#include <stdint.h>
#include <string.h>
#include "monitor.h"

uint64_t getTimestamp()
{
    uint32_t *GLOBAL_TIMESTAMP_ADRS = (uint32_t*)0x023101F0;
    return _itoll(GLOBAL_TIMESTAMP_ADRS[1], GLOBAL_TIMESTAMP_ADRS[0]); 
}

/******************************************************************************
* record_wg_data
******************************************************************************/
typedef struct 
{
    uint32_t start;
    uint32_t stop;
    uint32_t wg;
    uint32_t core;
} STATS;

#define MAX_STAT 1024

FAST_SHARED_1D(STATS, stats, MAX_STAT);

void stats_record(uint32_t wgid, uint32_t coreid, uint32_t start, uint32_t stop)
{
    STATS *core = &stats[wgid];
    if (wgid >= MAX_STAT) return;

    core->start = start;
    core->stop  = stop;
    core->wg    = wgid;
    core->core  = coreid;
}

/******************************************************************************
* stats_start
******************************************************************************/
void stats_start() 
{ 
    memset((void*) stats, 0, sizeof(stats)); 

    uint32_t *GLOBAL_TIMESTAMP_ADRS = (uint32_t*)0x023101F0;
    *GLOBAL_TIMESTAMP_ADRS = 0;
}

void stats_stop() { }
