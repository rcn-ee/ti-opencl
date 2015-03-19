/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _CLC_H_
#define _CLC_H_

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define _CLC_OVERLOAD __attribute__((overloadable))
#define _CLC_DECL     
#define _CLC_DEF      
#define _CLC_INLINE   __attribute__((always_inline)) inline


typedef unsigned int cl_mem_fence_flags;

#include "clcmacro.h"
#include "types.h"
#include "opencl_defs.h"
#include "image_defs.h"
#include "astype.h"
#include "workitem.h"
#include "convert.h"

#include "vload.h"

/*-----------------------------------------------------------------------------
* Math
*----------------------------------------------------------------------------*/
#include "math_defs.h"
#include "math.h"
#include "mad.h"

/*-----------------------------------------------------------------------------
* Integer
*----------------------------------------------------------------------------*/
#include "hadd.h"
#include "clamp.h"
#include "max.h"
#include "mix.h"

/*-----------------------------------------------------------------------------
* Common
*----------------------------------------------------------------------------*/
#include "degrees.h"
#include "step.h"
#include "smoothstep.h"
#include "sign.h"

/*-----------------------------------------------------------------------------
* Geometric
*----------------------------------------------------------------------------*/
#include "dot.h"
#include "cross.h"
#include "length.h"

/*-----------------------------------------------------------------------------
* Relational
*----------------------------------------------------------------------------*/
#include "isnan.h"
#include "bitselect.h"
#include "isequal.h"

#endif //_CLC_H_
