/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
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

#ifndef _IMAGE_DEFS_H_
#define _IMAGE_DEFS_H_

#define CLK_NORMALIZED_COORDS_FALSE 0x00000000
#define CLK_NORMALIZED_COORDS_TRUE  0x00000001
#define CLK_ADDRESS_NONE            0x00000000
#define CLK_ADDRESS_MIRRORED_REPEAT 0x00000010
#define CLK_ADDRESS_REPEAT          0x00000020
#define CLK_ADDRESS_CLAMP_TO_EDGE   0x00000030
#define CLK_ADDRESS_CLAMP           0x00000040
#define CLK_FILTER_NEAREST          0x00000000
#define CLK_FILTER_LINEAR           0x00000100
#define CLK_LOCAL_MEM_FENCE         0x00000001
#define CLK_GLOBAL_MEM_FENCE        0x00000002
#define CLK_R                       0x10B0
#define CLK_A                       0x10B1
#define CLK_RG                      0x10B2
#define CLK_RA                      0x10B3
#define CLK_RGB                     0x10B4
#define CLK_RGBA                    0x10B5
#define CLK_BGRA                    0x10B6
#define CLK_ARGB                    0x10B7
#define CLK_INTENSITY               0x10B8
#define CLK_LUMINANCE               0x10B9
#define CLK_Rx                      0x10BA
#define CLK_RGx                     0x10BB
#define CLK_RGBx                    0x10BC
#define CLK_SNORM_INT8              0x10D0
#define CLK_SNORM_INT16             0x10D1
#define CLK_UNORM_INT8              0x10D2
#define CLK_UNORM_INT16             0x10D3
#define CLK_UNORM_SHORT_565         0x10D4
#define CLK_UNORM_SHORT_555         0x10D5
#define CLK_UNORM_INT_101010        0x10D6
#define CLK_SIGNED_INT8             0x10D7
#define CLK_SIGNED_INT16            0x10D8
#define CLK_SIGNED_INT32            0x10D9
#define CLK_UNSIGNED_INT8           0x10DA
#define CLK_UNSIGNED_INT16          0x10DB
#define CLK_UNSIGNED_INT32          0x10DC
#define CLK_HALF_FLOAT              0x10DD
#define CLK_FLOAT                   0x10DE

#endif //_IMAGE_DEFS_H_
