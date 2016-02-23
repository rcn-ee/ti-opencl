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


#ifndef RANDOM1HEADER_H_
#define RANDOM1HEADER_H_

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "initial.h"


#define DEBUG_COUNTER   16
#define DEBUG_VALUES
#define  DEBUG



extern FILE  *pf   ;
extern int w_i  ;


#if 0

union long_t
{
	unsigned long long ll1 ;
	unsigned long l2[2]   ;
};

#endif




typedef struct initial_t {
	union long_t seed ;
	union long_t seed2 ;
	unsigned long  addValue  ;
	unsigned long  addValue2  ;
	unsigned long  mulitiplyValue ;
} initial_v;


#define  NUMBER_OF_ELEMENTS   (1024)
#define  NUMBER_OF_TOTAL_ELEMENTS   (1024 * 1024)


#define  TWO_POWER_48    (1024*1024*1024*1024*256)
#define  TWO_POWER_49    (1024*1024*1024*1024*512)
#define   FLOAT_ONE_OVER_RANGE  (float) ((float) 1.0 / (float) TWO_POWER_49 )
#define   FLOAT_ONE_OVER_16_bit  (float) ((float) 1.0 /  65536.0)
#define   FLOAT_ONE_OVER_17_bit  (float) ((float) 1.0 /  (2.0* 65536.0))




#endif /* RANDOM1HEADER_H_ */



