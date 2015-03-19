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

#define COLLECT_STATS 0

/******************************************************************************
* record_wg_data
******************************************************************************/
typedef struct 
{
    uint32_t wgs;
    uint32_t first_start;
    uint32_t last_stop;
    uint32_t avg_overhead;
    uint32_t avg_elapsed;
    uint32_t pad[3];
} STATS;

#if COLLECT_STATS
FAST_SHARED_1D(STATS, stats, 8);
#endif

void stats_record(uint32_t wgid, uint32_t coreid, uint32_t start, uint32_t stop)
{
#if COLLECT_STATS
    STATS *core = &stats[coreid];

    core->wgs ++;
    if (!core->first_start) 
         core->first_start = start;
    else core->avg_overhead += start - core->last_stop;

    core->last_stop = stop;
    core->avg_elapsed  += stop - start;
#endif
}

/******************************************************************************
* stats_start
******************************************************************************/
void stats_start() 
{ 
#if COLLECT_STATS
    memset((void*) stats, 0, sizeof(stats)); 
#endif
}

/******************************************************************************
* stats_stop
******************************************************************************/
void stats_stop() 
{
#if COLLECT_STATS
    int coreid;
    for (coreid = 0; coreid < 8; coreid++)
    {
        STATS *core = &stats[coreid];
        core->last_stop    -= core->first_start;
        core->avg_overhead /= (core->wgs-1);
        core->avg_elapsed  /= core->wgs;
    }
#endif
}
