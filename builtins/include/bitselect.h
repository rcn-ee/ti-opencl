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
#ifndef _BITSELECT_H_
#define _BITSELECT_H_

#include <clc.h>

_CLC_OVERLOAD _CLC_INLINE char   bitselect(char a,   char b,   char c)   { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE uchar  bitselect(uchar a,  uchar b,  uchar c)  { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE short  bitselect(short a,  short b,  short c)  { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE ushort bitselect(ushort a, ushort b, ushort c) { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE int    bitselect(int a,    int b,    int c)    { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE uint   bitselect(uint a,   uint b,   uint c)   { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE long   bitselect(long a,   long b,   long c)   { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE ulong  bitselect(ulong a,  ulong b,  ulong c)  { return a^(c&(b^a)); }

_CLC_OVERLOAD _CLC_INLINE char   bitselect(char2 a,   char2 b,   char2 c)   { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE uchar  bitselect(uchar2 a,  uchar2 b,  uchar2 c)  { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE short  bitselect(short2 a,  short2 b,  short2 c)  { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE ushort bitselect(ushort2 a, ushort2 b, ushort2 c) { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE int    bitselect(int2 a,    int2 b,    int2 c)    { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE uint   bitselect(uint2 a,   uint2 b,   uint2 c)   { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE long   bitselect(long2 a,   long2 b,   long2 c)   { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE ulong  bitselect(ulong2 a,  ulong2 b,  ulong2 c)  { return a^(c&(b^a)); }

_CLC_OVERLOAD _CLC_INLINE char   bitselect(char3 a,   char3 b,   char3 c)   { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE uchar  bitselect(uchar3 a,  uchar3 b,  uchar3 c)  { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE short  bitselect(short3 a,  short3 b,  short3 c)  { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE ushort bitselect(ushort3 a, ushort3 b, ushort3 c) { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE int    bitselect(int3 a,    int3 b,    int3 c)    { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE uint   bitselect(uint3 a,   uint3 b,   uint3 c)   { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_DECL   long   bitselect(long3 a,   long3 b,   long3 c);   
_CLC_OVERLOAD _CLC_DECL   ulong  bitselect(ulong3 a,  ulong3 b,  ulong3 c);  

_CLC_OVERLOAD _CLC_INLINE char   bitselect(char4 a,   char4 b,   char4 c)   { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE uchar  bitselect(uchar4 a,  uchar4 b,  uchar4 c)  { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE short  bitselect(short4 a,  short4 b,  short4 c)  { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE ushort bitselect(ushort4 a, ushort4 b, ushort4 c) { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE int    bitselect(int4 a,    int4 b,    int4 c)    { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE uint   bitselect(uint4 a,   uint4 b,   uint4 c)   { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_DECL   long   bitselect(long4 a,   long4 b,   long4 c);   
_CLC_OVERLOAD _CLC_DECL   ulong  bitselect(ulong4 a,  ulong4 b,  ulong4 c);  

_CLC_OVERLOAD _CLC_INLINE char   bitselect(char8 a,   char8 b,   char8 c)   { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE uchar  bitselect(uchar8 a,  uchar8 b,  uchar8 c)  { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE short  bitselect(short8 a,  short8 b,  short8 c)  { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE ushort bitselect(ushort8 a, ushort8 b, ushort8 c) { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_DECL   int    bitselect(int8 a,    int8 b,    int8 c);    
_CLC_OVERLOAD _CLC_DECL   uint   bitselect(uint8 a,   uint8 b,   uint8 c);   
_CLC_OVERLOAD _CLC_DECL   long   bitselect(long8 a,   long8 b,   long8 c);   
_CLC_OVERLOAD _CLC_DECL   ulong  bitselect(ulong8 a,  ulong8 b,  ulong8 c);  

_CLC_OVERLOAD _CLC_INLINE char   bitselect(char16 a,   char16 b,   char16 c)   { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_INLINE uchar  bitselect(uchar16 a,  uchar16 b,  uchar16 c)  { return a^(c&(b^a)); }
_CLC_OVERLOAD _CLC_DECL   short  bitselect(short16 a,  short16 b,  short16 c);  
_CLC_OVERLOAD _CLC_DECL   ushort bitselect(ushort16 a, ushort16 b, ushort16 c); 
_CLC_OVERLOAD _CLC_DECL   int    bitselect(int16 a,    int16 b,    int16 c);    
_CLC_OVERLOAD _CLC_DECL   uint   bitselect(uint16 a,   uint16 b,   uint16 c);   
_CLC_OVERLOAD _CLC_DECL   long   bitselect(long16 a,   long16 b,   long16 c);   
_CLC_OVERLOAD _CLC_DECL   ulong  bitselect(ulong16 a,  ulong16 b,  ulong16 c);  

#endif // _BITSELECT_H_
