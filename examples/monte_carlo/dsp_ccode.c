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
#define NORMS 512

#include "initial.h"
#include "dsp_c.h"

char *l1d_alloc(int size);
void  l1d_free_all();

/******************************************************************************
* return the reciprocal of a float
******************************************************************************/
static inline float recipsp_i (float a)
{
    float TWO = 2.0;
    float Big = 3.402823466E+38;
    float X, Y;

    Y = _fabsf(a);
    X = _rcpsp(a);
    X = X  * (TWO - a*X);
    X = X  * (TWO - a*X);

    if (Y > Big) X = 0.0;

    return (X);
}

/******************************************************************************
* The sqrtsp function returns the square root of a real floating-point value.
* Newton-Rhapson algorithm is applied for better precision. 
******************************************************************************/
double FLT_MAX = (double) 0x7f7f0000ffffffff;
static inline float sqrtsp_i (float x)
{
    const float Half  = 0.5f;
    const float OneP5 = 1.5f;

    float x1, x2, y;

    x1 = _rsqrsp(x);
    x2 = x1 * (OneP5 - (x * x1*x1*Half));
    x2 = x2 * (OneP5 - (x * x2*x2*Half));
    y  = x*x2;

    if (x <= 0.0f)   y = 0.0f;
    if (x > FLT_MAX) y = FLT_MAX;

    return y;
}

/******************************************************************************
* Return log of float input
******************************************************************************/

static inline float logsp_i (float a, double *logtable)
{
    double  ln2  =  0.693147180559945;
    float   c1   = -0.2302894f;
    float   c2   =  0.1908169f;
    float   c3   = -0.2505905f;
    float   c4   =  0.3333164f;
    float   c5   = -0.5000002f;
    float   MAXe =  3.402823466E+38;
    float   pol, r1, r2, r3, r4, res;
    double  dr, frcpax, rcp, T;
    int     N, T_index;

    /* r = x * frcpa(x) -1 */
    rcp    = _rcpdp((double) a);
    frcpax = _itod(_clr(_hi(rcp),0,16), 0);
    dr     = frcpax * (double) a  - 1.0; 

    /* Polynomial p(r) that approximates ln(1+r) - r */
    r1 = (float) dr;
    r2 = r1*r1;
    r3 = r1*r2;
    r4 = r2*r2; 

    pol = c5*r2 + ((c4*r3) + ((c2*r1 + c3) + c1*r2)*r4);

    /* Reconstruction: result = T + r + p(r) */
    N       = _extu(_hi(frcpax),  1, 21) - 1023;
    T_index = _extu(_hi(frcpax), 12, 29);
    T       = logtable[T_index] - ln2 * (double) N;
    res     = (dr + T) + (pol);

    if (a <= 0) res = _itof(0xFF800000);
    if (a > MAXe) res = 709.7827;

    return (res);
}

/******************************************************************************
* For a buffer of floats, replace inline with their reciprocal
******************************************************************************/
void oneOver(float *p)
{
    int i;

    _nassert((long) p % 8 == 0);

    for (i=0; i<1024; i++) p[i] = recipsp_i(p[i]);
}

/******************************************************************************
* For a buffer of floats f, replace inline with 2/f
******************************************************************************/
void oneOverMultiply_2(float *p)
{
    int i;

    _nassert((long) p % 8 == 0);

    for (i=0; i<NORMS; i++) p[i] = 2.0 * recipsp_i(p[i]);
}

/******************************************************************************
* Multiply float buffer one by float buffer two into float buffer one.
******************************************************************************/
void multiplyFloat(float * restrict p1, float * restrict p2)
{
   int i;

   _nassert((long) p1 % 8 == 0);
   _nassert((long) p2 % 8 == 0);

    for (i=0; i<NORMS; i++) p1[i] *= p2[i];
}

/******************************************************************************
* doubleMultiply
******************************************************************************/
void doubleMultiply(float * restrict p0, float * restrict p1, float * restrict p2)
{
   int i;

   _nassert((long) p0 % 8 == 0);
   _nassert((long) p1 % 8 == 0);
   _nassert((long) p2 % 8 == 0);

   for (i=0; i<NORMS; i++)
   {
      *p0++ = *p1++ * *p2;
      *p0++ = *p1++ * *p2++;
   }
}

/******************************************************************************
* For a float buffer, replace each element with its negative log
******************************************************************************/
void log_1024_1(float *dst, float *in, double *logtable)
{
    int i;
    _nassert((long) dst % 8 == 0);
    _nassert((long) in  % 8 == 0);

    {
        logtable[0] = (double)  0.0000000000;
        logtable[1] = (double) -0.1177830356;
        logtable[2] = (double) -0.2231435513;            
        logtable[3] = (double) -0.3184537311;            
        logtable[4] = (double) -0.4054651081;            
        logtable[5] = (double) -0.4855078157;            
        logtable[6] = (double) -0.5596157879;            
        logtable[7] = (double) -0.6286086594;            
    }

    for (i=0; i<NORMS; i++) dst[i] = -logsp_i(in[i],logtable);
}


/******************************************************************************
* For a float buffer, replace each element with its square root
******************************************************************************/
void sqrtsp_v (float * restrict a, float * restrict output)
{
    int i;

    _nassert((long) a       % 8 == 0);
    _nassert((long) output  % 8 == 0);

    for (i = 0; i < NORMS; i++) output[i] = sqrtsp_i(a[i]);
}


/********************************************************************************
* 
*              Long Pseudo Random Generation
*
* Monte-Carlo method is a popular technique in Physics, Engineering, Biology,
* printf Mathematics games Cryptographic and Finance
*
* For Monte Carlo method to work properly, the sequence of random number must
* be indeed random  nassert Several techniques are used to test the
* "Randomness" of pseudo random sequences, see for example NIST publication "A
* Statistical Test Suite for Random and Pseudorandom Number Generators for
* Cryptographic Applications" A common technique for generating pseudo random
* numbers is Linear congruential generator where the algorithm is X(n+1) =
* (a*X(n)+c) Mod M  where a is a prime number, M is the sequence cycle (that
* is, after M elements the sequence will start repeat itself ) and C is a
* constant
*
* The "randomness" of the sequence depends on the choice of a, c and M (and the
* initial seed X0)
*
* In this program a is 2 to the power of 31 minus 1, and M is 2 to the power of
* 49 (249). The sequence cycle is more than twice 10 to the power of 14 (1014).
* If one can count 1000 values in a second, it will take more than  8925 years
* to count all the values in this sequence.
*
* Parallel Processing
* The Linear Congruential generation algorithm is not friendly to parallel
* processing, see for example Parallel Pseudorandom Number Generation By
* Michael Mascagni from SIAM News, Volume 32, Number 5.  The paper suggests
* choosing prime-wise additive (the C value) values.  This is not a good
* solution for massive parallel machines.  The algorithm that is implemented in
* this paper is not define for heavy computational problem, rather it assumes
* generating of random numbers on limited number of cores, 2, or 8.  For this
* number of cores it is possible to generate set 8 prime numbers as the adding
* term C for each core.  In addition, we expect the simulation that uses these
* numbers to consume millions of numbers, say up to 10**9 which is really very
* small (1/200000) percentage of the total sequence
*
* Initial Condition
*
* The Initial seed as well as the adaptive constant are stored in a global
* vector. Each core reads its own values and updated the seed at the end of
* execution in preparation to a second call
*
* Output results are unsigned 64-bit values uniformly distributed between 1 and
* 2 to the power of 49 minus 1 (249-1).  To use the random numbers in
* simulations the algorithm convers the uniformly distributed numbers into
* float precision floating point Gaussian random values with mean zero and
* standard deviation of 1.
********************************************************************************/
#define FLOAT_ONE_OVER_17_bit  ((float)(1.0f / (1<<17)))

void  longPseudoRandom(struct initial1_t *value, int index, float *pp1, float *pp2)
{
    //unsigned long  next, next2;
    //union long_t   vx, vx2;
    //unsigned long a1,a2,a3,a4;
    
    int                counter;
    union long_t       vy, vz,vv;
    union long_t       vy2 ,vz2,vv2;
    struct initial1_t * str;
    unsigned long      mulV;
    unsigned long      addV;
    unsigned long      l_l, l_h;
    unsigned long      l2_l, l2_h;
    unsigned long long aux1 ,aux2;
    unsigned long      addV2;
    float              w, x1, x2;
    float              x_aux, y_aux;
    float              x2_aux, y2_aux;
    float              dividValue;

    _nassert((long) pp1 % 8 == 0);
    _nassert((long) pp2 % 8 == 0);

    //     pp3 = pp1 + 512;  //  Ran Katzur Change
    //     pp4 = pp2 + 256;

    /*
     *  First loop - calculate the N/2 x values , N/2 y values and N/2 w values
     *  The algorithms uses intrinsic
     *
     */

    dividValue = (float) (1.0 /(1024.0 * 1024.0 * 1024.0 ) );
    x_aux      = (float) (1.0 / (1024.0 * 512.0) );
    dividValue = dividValue * x_aux;
    str        = &value[index];

    //vx.ll1 = str->seed.ll1;    //   The initial value for x and y
    //vx2.ll1 = str->seed2.ll1;  //   The initial value for x and y

    l_h  = str->seed.l2[1];  //vx.l2[1];
    l2_h = str->seed2.l2[1]; //vx2.l2[1];
    l_l  = str->seed.l2[0];  //vx.l2[0];
    l2_l = str->seed2.l2[0]; //vx2.l2[0];

    /*-------------------------------------------------------------------------
    * Stupid Debug
    *------------------------------------------------------------------------*/
    str->seed.l2[0]  = l_l;  // = vx.ll1;  //   The initial value for x and y
    str->seed.l2[1]  = l_h;  // = vx.ll1;  //   The initial value for x and y
    str->seed2.l2[0] = l2_l; // = vx.ll1;  //   The initial value for x and y
    str->seed2.l2[1] = l2_h; // = vx.ll1;  //   The initial value for x and y
    str->seed.l2[1]  = l_h + 2;  // = vx.ll1;  //   The initial value for x and y
    str->seed2.l2[1] = l2_h + 2; // = vx.ll1;  //   The initial value for x and y
    str->seed.l2[0]  = l_l + 3;  // = vx.ll1;  //   The initial value for x and y
    str->seed2.l2[0] = l2_l + 3; // = vx.ll1;  //   The initial value for x and y

    /*************************************************************************/
    mulV  = str->multiplyValue;
    addV  = (long long) str->addValue;
    addV2 = (long long) str->addValue2;

    counter = NORMS + 1;  

    // In each loop 4 values are generated x1, x2, x1 of the second loop and x2 of the second loop
    //   and there are two decrements and check for more than 0
    //
    do
    {
        vy.ll1 = ((unsigned long long) l_h ) * mulV; //  this is 2**32 value because l_h is 2**32 value
        vz.l2[1] = vy.l2[0];  //  Only lower 17bits of the multiplication survive the MOD(2**49)
        vz.l2[0] = (unsigned long) addV;                  //   because lower 32 bits are 0
        aux1  = (long long) ((( unsigned long long)l_l  ) * mulV);
        vv.ll1 = ( aux1 + vz.ll1);
        l_h = vv.l2[1] & 0x1ffff;  //   this is the modulo 49 that zero out upper bits of the upper register
        l_l  = vv.l2[0]; // lower 32 bits of the result
        x_aux = (float) l_h *FLOAT_ONE_OVER_17_bit;
        y_aux = (float) l_l * dividValue;
        x1 = x_aux + y_aux;
        x1 = 2.0 * x1 - 1.0;

        vy.ll1 = ((unsigned long long) l_h ) * mulV; //  this is 2**32 value because l_h is 2**32 value
        vz.l2[1] = vy.l2[0];  //  Only lower 17bits of the multiplication survive the MOD(2**49)
        vz.l2[0] = (unsigned long) addV;                  //   because lower 32 bits are 0
        aux1  = (long long) ((( unsigned long long)l_l  ) * mulV);
        vv.ll1 = ( aux1 + vz.ll1);
        l_h = vv.l2[1] & 0x1ffff;  //   this is the modulo 49 that zero out upper bits of the upper register
        l_l  = vv.l2[0]; // lower 32 bits of the result
        x_aux = (float) l_h *FLOAT_ONE_OVER_17_bit;
        y_aux = (float) l_l * dividValue;
        x2 = x_aux + y_aux;
        x2 = 2.0 * x2 -1.0;

        w = x1 * x1 + x2 * x2;
        if (w < 1) counter--;
        if (w < 1) *pp2++ = w;
        if (w < 1) *pp1++ = x1;
        if (w < 1) *pp1++ = x2;

        vy2.ll1 = ((unsigned long long) l2_h ) * mulV; //  this is 2**32 value because l_h is 2**32 value
        vz2.l2[1] = vy2.l2[0];  //  Only lower 17bits of the multiplication survive the MOD(2**49)
        vz2.l2[0] = (unsigned long) addV2;                  //   because lower 32 bits are 0
        aux2  = (long long) ((( unsigned long long)l2_l  ) * mulV);

        vv2.ll1 = ( aux2 + vz2.ll1);

        //   vv is the random number variable  - translate it into l_h ans l_l
        l2_h = vv2.l2[1] & 0x1ffff;  //   this is the modulo 49 that zero out upper bits of the upper register
        l2_l  = vv2.l2[0]; // lower 32 bits of the result

        x2_aux = (float) l2_h *FLOAT_ONE_OVER_17_bit;
        y2_aux = (float) l2_l * dividValue;
        x1 = x2_aux + y2_aux;
        x1 = 2.0 * x1 - 1.0;

        vy2.ll1 = ((unsigned long long) l2_h ) * mulV; //  this is 2**32 value because l_h is 2**32 value
        vz2.l2[1] = vy2.l2[0];  //  Only lower 17bits of the multiplication survive the MOD(2**49)
        vz2.l2[0] = (unsigned long) addV2;                  //   because lower 32 bits are 0
        aux2  = (long long) ((( unsigned long long)l2_l  ) * mulV);
        vv2.ll1 = ( aux2 + vz2.ll1);

        //   vv is the random number variable  - translate it into l_h ans l_l
        l2_h = vv2.l2[1] & 0x1ffff;  // this is the modulo 49 that zero out upper bits of the upper register
        l2_l  = vv2.l2[0]; // lower 32 bits of the result

        x2_aux = (float) l2_h *FLOAT_ONE_OVER_17_bit;
        y2_aux = (float) l2_l * dividValue;
        x2 = x2_aux + y2_aux;
        x2 = 2.0 * x2 - 1.0;
        w = x1 * x1 + x2 * x2;
        if (w < 1) counter--;
        if (w < 1) *pp2++ = w; // the two sequences are combined together
        if (w < 1) *pp1++ = x1;// Ran Katzur Change
        if (w < 1) *pp1++ = x2; // Ran Katzur change
    } while (counter > 0 );

    //    Load back the new seeds to the structure

    //      vx.l2[1]  = l_h;
    //      vx2.l2[1] = l2_h;
    //      vx.l2[0]  = l_l;
    //      vx2.l2[0] = l2_l;

    str->seed.l2[0]  = l_l;  // = vx.ll1;  //   The initial value for x and y
    str->seed.l2[1]  = l_h;  // = vx.ll1;  //   The initial value for x and y
    str->seed2.l2[0] = l2_l; // = vx.ll1;  //   The initial value for x and y
    str->seed2.l2[1] = l2_h; // = vx.ll1;  //   The initial value for x and y
    //      str->seed2.ll1  =  vx2.ll1;  //   The initial value for x and y
}

/******************************************************************************
* generate_1024_GaussianRandom
******************************************************************************/
void  generate_1024_GaussianRandom(struct initial1_t *vector, int index,
                                   float *v1, float *scratch1, 
                                   float *scratch2, float * scratch3,
                                   double * logtable)
{
    /*-------------------------------------------------------------------------
    * This function generates 1024 Gaussian numbers following the method that 
    * is described above
    *------------------------------------------------------------------------*/
    longPseudoRandom (vector, index, scratch1, scratch2);
    log_1024_1       (scratch3, scratch2, logtable);
    oneOverMultiply_2(scratch2);
    multiplyFloat    (scratch2, scratch3);
    sqrtsp_v         (scratch2, scratch3);
    doubleMultiply   (v1, scratch1, scratch3);
}

/******************************************************************************
* generateRandomGauss
******************************************************************************/
void generateRandomGauss(float *outBuffer, int size, struct initial1_t *vector, 
                         int index)
{
    float *scratch1 = (float*)l1d_alloc (2 * (NORMS + 2) * sizeof(float));
    float *scratch2 = (float*)l1d_alloc (    (NORMS + 2) * sizeof(float));
    float *scratch3 = (float*)l1d_alloc (    (NORMS + 2) * sizeof(float));
    double  *pad = (double *)l1d_alloc (    8* sizeof(double));
    double  *logtable = (double *)l1d_alloc (    8* sizeof(double));

    int i;
    for (i=0; i < size; i+=1024)
        generate_1024_GaussianRandom(vector, index, &outBuffer[i], 
                                     scratch1, scratch2, scratch3,
                                     logtable);

    l1d_free_all();
}

/******************************************************************************
* __scratch_l1d_start() is defined in OpenCL version 01.01.05.00 and later.
******************************************************************************/
#if (__TI_OCL_VERSION < 0x01010500)
void  *__scratch_l1d_start() { return (char*)0x00F00000; }
#endif

/******************************************************************************
* l1d_alloc - Simple allocate of available l1d.
******************************************************************************/
#define ROUNDUP(val, pow2)   (((val) + (pow2) - 1) & ~((pow2) - 1))
static char* l1d_ptr = 0;
char *l1d_alloc(int size)
{
    if (!l1d_ptr) l1d_ptr = __scratch_l1d_start();
    char *p = l1d_ptr;
    l1d_ptr += ROUNDUP(size,8);
    return p;
}

void l1d_free_all() { l1d_ptr = __scratch_l1d_start(); }

