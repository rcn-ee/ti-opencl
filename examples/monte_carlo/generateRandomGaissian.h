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


/*
*               Long Pseudo Random Generation
*/

#include "gaussRandom.h"


#define  TWO_POWER_48    (1024*1024*1024*1024*256)
#define  TWO_POWER_49    (1024*1024*1024*1024*512)

#define  PRINTF      //printf



static void  longPseudoRandom(struct initial_t value[], 
                              int index, 
                              int N,
                              float *p_out )
{
     // unsigned long  next, next2;

      int  counter  ;
      union long_t  vx, vy ,vz,vv   ;
      struct initial_t str   ;
      unsigned long mulV   ;
      unsigned long addV  ;
      unsigned long l_l, l_h   ;
      unsigned long long aux1 ,aux2 ;

      //unsigned long a1,a2,a3,a4  ;
      float w, x1, x2, y1, y2   ;
      float  x_aux, y_aux   ;
      float  a_aux, b_aux   ;
      float dividValue   ;

      dividValue = (float) (1.0 /(1024.0 * 1024.0 * 1024.0 ) )  ;
      x_aux = (float) (1.0 / (1024.0 * 512.0) ) ;
      dividValue = dividValue * x_aux   ;


      str = value[index]  ;
      vx.ll1 = str.seed.ll1  ;  //   The initial value for x and y


      l_h = vx.l2[1]  ;
      l_l = vx.l2[0]  ;
      mulV = str.mulitiplyValue ;


      addV = (long long) str.addValue   ;



       counter = N/2 ;  //   N  ;

      do
      {
    	  vy.ll1 = ((unsigned long long) l_h ) * mulV  ; //  this is 2**32 value because l_h is 2**32 value
    	  vz.l2[1] = vy.l2[0]  ;  //  Only lower 17bits of the multiplication survive the MOD(2**49)
    	  vz.l2[0] = (unsigned long) addV ;                  //   because lower 32 bits are 0
    	  aux1  = (long long) ((( unsigned long long)l_l  ) * mulV)   ;
          vv.ll1 = ( aux1 + vz.ll1)    ;
          //   vv is the random number variable  - translate it into l_h ans l_l
          l_h = vv.l2[1] & 0x1ffff  ;  //   this is the modulo 49 that zero out upper bits of the upper register
          l_l  = vv.l2[0]          ; // lower 32 bits of the result


          x_aux = (float) l_h *FLOAT_ONE_OVER_17_bit  ;
          y_aux = (float) l_l * dividValue  ;
          x1 = x_aux + y_aux  ;
          x1 = 2.0 * x1 - 1.0 ;


    	  vy.ll1 = ((unsigned long long) l_h ) * mulV  ; //  this is 2**32 value because l_h is 2**32 value


    	  vz.l2[1] = vy.l2[0]  ;  //  Only lower 17bits of the multiplication survive the MOD(2**49)
    	  vz.l2[0] = (unsigned long) addV ;                  //   because lower 32 bits are 0


    	  aux1  = (long long) ((( unsigned long long)l_l  ) * mulV)   ;
          vv.ll1 = ( aux1 + vz.ll1)    ;

          //   vv is the random number variable  - translate it into l_h ans l_l
          l_h = vv.l2[1] & 0x1ffff  ;  //   this is the modulo 49 that zero out upper bits of the upper register
          l_l  = vv.l2[0]          ; // lower 32 bits of the result


          x_aux = (float) l_h *FLOAT_ONE_OVER_17_bit  ;
          y_aux = (float) l_l * dividValue  ;
          x2 = x_aux + y_aux  ;
          x2 = 2.0 * x2 -1.0 ;
          w = x1 * x1 + x2 * x2  ;
          if (w < 1) 
          {
               counter--   ;
               x_aux = -log(w)  ;
               y_aux = 2.0 / w  ;
               a_aux = x_aux * y_aux  ;
               b_aux = sqrt (a_aux)  ;
               //w = sqrt( (-2.0 * log(w) ) /w)  ;
               y1 = x1 * b_aux   ;  //w   ;
               y2 = x2 * b_aux   ;  //w   ;
//               printf (" results   %d  %f %f %f  \n", counter,x1,x2,w)  ;
               //printf("\n %d -> w  %f -log  %f  two_oneOver  %f  \n", 
                                      //counter,w,x_aux,y_aux) ;
               //printf(" multiply %f  sqrt  %f y1 %f  y2   %f  \n", 
                                     //a_aux, b_aux, y1, y2 ) ;
               *p_out++ = y1   ;
               *p_out++ = y2   ;
          }

      } while (counter > 0 ) ;

//    Load back the new seeds to the structure





}

