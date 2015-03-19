/******************************************************************************
 * Copyright (c) 2011-2014, Peter Collingbourne <peter@pcc.me.uk>
 * Copyright (c) 2011-2014, Texas Instruments Incorporated - http://www.ti.com/
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

_CLC_PROTECTED void __mfence(void);
_CLC_PROTECTED void barrier(cl_mem_fence_flags flags) { }
_CLC_PROTECTED void mem_fence(cl_mem_fence_flags flags)       { __mfence(); }
_CLC_PROTECTED void read_mem_fence(cl_mem_fence_flags flags)  { __mfence(); }
_CLC_PROTECTED void write_mem_fence(cl_mem_fence_flags flags) { __mfence(); }


_CLC_OVERLOAD _CLC_PROTECTED void wait_group_events(int num_events, event_t *event_list)
{
   int i;

   if (event_list)
     for ( i = 0; i < num_events; ++i,event_list++)
       __copy_wait(*event_list);
}

#define VEC_TYPE(type,sz) type##sz

#define CROSS_SIZES(type) \
    TEMPLATE(type)              \
    TEMPLATE(VEC_TYPE(type,2))  \
    TEMPLATE(VEC_TYPE(type,3))  \
    TEMPLATE(VEC_TYPE(type,4))  \
    TEMPLATE(VEC_TYPE(type,8))  \
    TEMPLATE(VEC_TYPE(type,16)) \

#define CROSS_TYPES()   \
    CROSS_SIZES(char)   \
    CROSS_SIZES(uchar)  \
    CROSS_SIZES(short)  \
    CROSS_SIZES(ushort) \
    CROSS_SIZES(int)    \
    CROSS_SIZES(uint)   \
    CROSS_SIZES(long)   \
    CROSS_SIZES(ulong)  \
    CROSS_SIZES(float)  \
    CROSS_SIZES(double) \

#define TEMPLATE(gentype) \
_CLC_OVERLOAD _CLC_PROTECTED event_t async_work_group_copy(local gentype *dst, \
              const global gentype *src, size_t num_gentypes, event_t event) \
{ return __copy_1D1D(event, (void *)dst,(void *)src,num_gentypes * sizeof(gentype)); } \
_CLC_OVERLOAD _CLC_PROTECTED event_t async_work_group_copy(global gentype *dst, \
              const local gentype *src, size_t num_gentypes, event_t event) \
{ return __copy_1D1D(event, (void *)dst,(void *)src,num_gentypes * sizeof(gentype)); } \
_CLC_OVERLOAD _CLC_PROTECTED event_t async_work_group_copy(global gentype *dst, \
              const global gentype *src, size_t num_gentypes, event_t event) \
{ return __copy_1D1D(event, (void *)dst,(void *)src,num_gentypes * sizeof(gentype)); } \
_CLC_OVERLOAD _CLC_PROTECTED event_t async_work_group_strided_copy(local gentype *dst, \
        const global gentype *src, size_t num_gentypes, size_t src_stride, event_t event) \
{ return __copy_2D1D(event, (void *)dst,(void *)src,sizeof(gentype),num_gentypes,src_stride); } \
_CLC_OVERLOAD _CLC_PROTECTED event_t async_work_group_strided_copy(global gentype *dst, \
        const local gentype *src, size_t num_gentypes, size_t dst_stride, event_t event) \
{ return __copy_1D2D(event, (void *)dst,(void *)src,sizeof(gentype),num_gentypes,dst_stride); }

CROSS_TYPES()

#undef VEC_TYPE
#undef CROSS_SIZES
#undef CROSS_TYPES
#undef TEMPLATE

