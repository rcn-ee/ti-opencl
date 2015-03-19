/******************************************************************************
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef _C66_HELPER_H
#define _C66_HELPER_H

#define SIGNF(x)        _extu(_ftoi(x), 0, 31)
#define EXPF(x)         _extu(_ftoi(x), 1, 24)
#define MANTF(x)        _extu(_ftoi(x), 9, 9)

#define ISINFF(x)       (_extu(_ftoi(x), 1, 1) == 0x7F800000)
#define ISPOSINFF(x)    (_ftoi(x) == 0x7F800000)
#define ISNEGINFF(x)    (_ftoi(x) == 0xFF800000)
#define ISNANF(x)       (_extu(_ftoi(x), 1, 1) >  0x7F800000)
#define ISINFORNANF(x)  (_extu(_ftoi(x), 1, 1) >= 0x7F800000)
#define ISDENORMF(x)    (EXPF(x) == 0 && MANTF(x) != 0)
#define ISANYZEROF(x)   (_extu(_ftoi(x),1, 1) == 0)
#define FABSF(x)        _itof(_extu(_ftoi(x), 1, 1))
#define NANF            _itof(0x7FC00000)
#define INFF            _itof(0x7F800000)
#define NEGNANF         _itof(0xFFC00000)
#define NEGINFF         _itof(0xFF800000)
#define NEGZEROF        _itof(0x80000000)

#define SIGND(x)        _extu(_hi(x), 0, 31)
#define EXPD(x)         _extu(_hi(x), 1, 21)
#define MANTD(x)        _itoll(_extu(_hi(x), 12, 12), _lo(x))
#define MANTD_HI(x)     _extu(_hi(x), 12, 12)
#define MANTD_LO(x)     _lo(x)
#define MANTD_ZERO(x)   (MANTD_HI(x) == 0 && MANTD_LO(x) == 0)

#define ISINFD(x)       (EXPD(x) == 2047 && MANTD_ZERO(x))
#define ISANYZEROD(x)   (EXPD(x) == 0    && MANTD_ZERO(x))
#define ISPOSINFD(x)    (ISINFD(x) && !SIGND(x))
#define ISNEGINFD(x)    (ISINFD(x) && SIGND(x))
#define ISNAND(x)       (EXPD(x) == 2047 && !MANTD_ZERO(x))
#define ISDENORMD(x)    (EXPD(x) == 0    && !MANTD_ZERO(x))
#define NAND            _itod(0x7FF80000,0x00000000)
#define INFD            _itod(0x7FF00000,0x00000000)
#define NEGNAND         _itod(0xFFF80000,0x00000000)
#define NEGINFD         _itod(0xFFF00000,0x00000000)
#define NEGZEROD        _itod(0x80000000,0x00000000)

#endif // _C66_HELPER_H
