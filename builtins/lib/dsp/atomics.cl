/******************************************************************************
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
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
#include "dsp.h"

uint __sem_lock(int);
void __sem_unlock(int, uint);
void __inv(char*, int);

#define LOCK_GLOBAL   		uint lvInt = __sem_lock(1)
#define UNLOCK_GLOBAL 		__sem_unlock(1, lvInt)
#define INV_GLOBAL(p, sz)	__inv((char*)(p), (sz))
#define WB_GLOBAL(p, sz)	

#define LOCK_LOCAL   
#define UNLOCK_LOCAL 
#define INV_LOCAL(p, sz)
#define WB_LOCAL(p, sz)

_CLC_OVERLOAD _CLC_DEF int  atomic_add(volatile global int*  p, int  val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    int old = *p;
    *p = old + val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_add(volatile global uint* p, uint val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    uint old = *p;
    *p = old + val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF int  atomic_add(volatile local  int*  p, int  val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    int old = *p;
    *p = old + val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_add(volatile local  uint* p, uint val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    uint old = *p;
    *p = old + val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}


_CLC_OVERLOAD _CLC_DEF int  atomic_sub(volatile global int*  p, int  val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    int old = *p;
    *p = old - val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_sub(volatile global uint* p, uint val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    uint old = *p;
    *p = old - val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF int  atomic_sub(volatile local  int*  p, int  val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    int old = *p;
    *p = old - val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_sub(volatile local  uint* p, uint val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    uint old = *p;
    *p = old - val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}


_CLC_OVERLOAD _CLC_DEF int  atomic_xchg(volatile global int*  p, int  val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    int old = *p;
    *p = val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_xchg(volatile global uint* p, uint val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    uint old = *p;
    *p = val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF float atomic_xchg(volatile global float* p, float val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    float old = *p;
    *p = val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF int  atomic_xchg(volatile local  int*  p, int  val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    int old = *p;
    *p = val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_xchg(volatile local  uint* p, uint val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    uint old = *p;
    *p = val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF float atomic_xchg(volatile local  float* p, float val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    float old = *p;
    *p = val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}


_CLC_OVERLOAD _CLC_DEF int  atomic_inc(volatile global int*  p) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    int old = *p;
    *p = old + 1;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_inc(volatile global uint* p) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    uint old = *p;
    *p = old + 1;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF int  atomic_inc(volatile local  int*  p) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    int old = *p;
    *p = old + 1;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_inc(volatile local  uint* p) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    uint old = *p;
    *p = old + 1;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}


_CLC_OVERLOAD _CLC_DEF int  atomic_dec(volatile global int*  p) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    int old = *p;
    *p = old - 1;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_dec(volatile global uint* p) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    uint old = *p;
    *p = old - 1;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF int  atomic_dec(volatile local  int*  p) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    int old = *p;
    *p = old - 1;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_dec(volatile local  uint* p) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    uint old = *p;
    *p = old - 1;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}


_CLC_OVERLOAD _CLC_DEF int  atomic_cmpxchg(volatile global int*  p, int  cmp, int  val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    int old = *p;
    if (old == cmp) *p = val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_cmpxchg(volatile global uint* p, uint cmp, uint val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    uint old = *p;
    if (old == cmp) *p = val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF int  atomic_cmpxchg(volatile local  int*  p, int  cmp, int  val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    int old = *p;
    if (old == cmp) *p = val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_cmpxchg(volatile local  uint* p, uint cmp, uint val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    uint old = *p;
    if (old == cmp) *p = val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}


_CLC_OVERLOAD _CLC_DEF int  atomic_min(volatile global int*  p, int  val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    int old = *p;
    if (val < old) *p = val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_min(volatile global uint* p, uint val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    uint old = *p;
    if (val < old) *p = val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF int  atomic_min(volatile local  int*  p, int  val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    int old = *p;
    if (val < old) *p = val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_min(volatile local  uint* p, uint val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    uint old = *p;
    if (val < old) *p = val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF int  atomic_max(volatile global int*  p, int  val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    int old = *p;
    if (val > old) *p = val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_max(volatile global uint* p, uint val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    uint old = *p;
    if (val > old) *p = val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF int  atomic_max(volatile local  int*  p, int  val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    int old = *p;
    if (val > old) *p = val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_max(volatile local  uint* p, uint val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    uint old = *p;
    if (val > old) *p = val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}


_CLC_OVERLOAD _CLC_DEF int  atomic_and(volatile global int*  p, int  val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    int old = *p;
    *p = old & val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_and(volatile global uint* p, uint val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    uint old = *p;
    *p = old & val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF int  atomic_and(volatile local  int*  p, int  val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    int old = *p;
    *p = old & val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_and(volatile local  uint* p, uint val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    uint old = *p;
    *p = old & val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}


_CLC_OVERLOAD _CLC_DEF int  atomic_or(volatile global int*  p, int  val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    int old = *p;
    *p = old | val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_or(volatile global uint* p, uint val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    uint old = *p;
    *p = old | val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF int  atomic_or(volatile local  int*  p, int  val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    int old = *p;
    *p = old | val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_or(volatile local  uint* p, uint val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    uint old = *p;
    *p = old | val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}


_CLC_OVERLOAD _CLC_DEF int  atomic_xor(volatile global int*  p, int  val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    int old = *p;
    *p = old ^ val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_xor(volatile global uint* p, uint val) 
{
    INV_GLOBAL(p, sizeof(*p));
    LOCK_GLOBAL;
    uint old = *p;
    *p = old ^ val;
    WB_GLOBAL(p, sizeof(*p));
    UNLOCK_GLOBAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF int  atomic_xor(volatile local  int*  p, int  val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    int old = *p;
    *p = old ^ val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

_CLC_OVERLOAD _CLC_DEF uint atomic_xor(volatile local  uint* p, uint val) 
{
    INV_LOCAL(p, sizeof(*p));
    LOCK_LOCAL;
    uint old = *p;
    *p = old ^ val;
    WB_LOCAL(p, sizeof(*p));
    UNLOCK_LOCAL;
    return old;
}

