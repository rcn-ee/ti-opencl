/******************************************************************************
 * Copyright (c) 2013-2015, Texas Instruments Incorporated - http://www.ti.com/
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
/******************************************************************************
*            
*    initializeInit.h    
*
*    This file contains the function initializeInit that initializes the 
*    structure that is used by the DSP to load initial seed value and
*    to save seed between calls
*    In addition the file has the multiply values and the additive values
*
*    The initialization structure is define in the include file gaussRandom.h
*    
*    A utility function printInitial_t() will print the values of the 
*    structure.  This is used for debug 
*
******************************************************************************/
#ifndef INITIALIZE_INIT_H
#define INITIALIZE_INIT_H

#include <stdint.h>

union long_t
{
    uint64_t ll1;
    uint32_t l2[2];
};

typedef struct initial1_t
{
    union long_t seed;
    union long_t seed2;
    uint32_t addValue;
    uint32_t addValue2;
    uint32_t multiplyValue;
} initial1_v;


/******************************************************************************
*    Here is a list of prime numbers to be used as addValues and initial seeds 
*    They are taken from
*     http://compoasso.free.fr/primelistweb/page/prime/liste_online_en.php
*
*   1068811 1151981 1244167 1421857 1677787 1681903 1750999 1837393
*   1906477 1907783 1910663 1971503 1978709 1980289 2003257 2004529
******************************************************************************/
static void
initializeInit1 (struct initial1_t *p)
{
    p[0].seed.ll1 = 1068811;
    p[0].seed2.ll1 = 1151981;
    p[0].addValue = 1244167;
    p[0].addValue2 = 1677787;
    p[0].multiplyValue = 0x40000001;    //0x7fffffff; //Mersenne Prime 2147483647;

    p[1].seed.ll1 = 1681903;
    p[1].seed2.ll1 = 1750999;
    p[1].addValue = 1837393;
    p[1].addValue2 = 1906477;
    p[1].multiplyValue = 0x40000001;    //0x7fffffff; //Mersenne Prime 2147483647;

    p[2].seed.ll1 = 7523;
    p[2].seed2.ll1 = 7867;
    p[2].addValue = 19069;
    p[2].addValue2 = 20983;
    p[2].multiplyValue = 0x40000001;    //0x7fffffff; //Mersenne Prime 2147483647;

    p[3].seed.ll1 = 6971;
    p[3].seed2.ll1 = 7541;
    p[3].addValue = 21001;
    p[3].addValue2 = 21383;
    p[3].multiplyValue = 0x40000001;    //0x7fffffff; //Mersenne Prime 2147483647;
}
#endif //INITIALIZE_INIT_H
