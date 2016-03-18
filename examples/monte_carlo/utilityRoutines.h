/******************************************************************************
 * Copyright (c) 2013-2016, Texas Instruments Incorporated - http://www.ti.com/
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Texas Instruments Incorporated nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

/*    Here is a list of prime numbers to be used as addValues and initial seeds  They are taken from
 *
 *
 *   http://compoasso.free.fr/primelistweb/page/prime/liste_online_en.php
 *
 *
 *   1068811
 *   1151981
 *   1244167
 *   1421857
 *   1677787
 *   1681903
 *   1750999
 *   1837393
 *   1906477
 *   1907783
 *   1910663
 *   1971503
 *   1978709
 *   1980289
 *   2003257
 *   2004529
 *
 *
 *
 *
 *
 */


static void
initializeInit (struct initial_t value[])
{
    value[0].seed.l2[1] = 0x0;
    value[0].seed.l2[0] = 1068811;
    value[0].addValue = 1151981;

    value[0].seed2.l2[1] = 0x0;
    value[0].seed2.l2[0] = 12441677;
    value[0].addValue2 = 1677787;

    value[0].mulitiplyValue = 0x40000001;   //0x7fffffff  ;  //  This is Mersenne Prime 2147483647 ;

    value[1].seed.l2[1] = 0x0;
    value[1].seed.l2[0] = 1681903;
    value[1].addValue = 1750999;

    value[1].seed2.l2[1] = 0x0;
    value[1].seed2.l2[0] = 1837393;
    value[1].addValue2 = 1906477;

    value[1].mulitiplyValue = 0x40000001;   // 0x7fffffff  ;  //  This is Mersenne Prime 2147483647 ;


    value[2].seed.l2[1] = 0x0;
    value[2].seed.l2[0] = 7523;
    value[2].addValue = 7867;

    value[2].seed2.l2[1] = 0x0;
    value[2].seed2.l2[0] = 19069;
    value[2].addValue2 = 20983;

    value[2].mulitiplyValue = 0x40000001;   //0x7fffffff  ;  //  This is Mersenne Prime 2147483647 ;

    value[3].seed.l2[1] = 0x0;
    value[3].seed.l2[0] = 6971;
    value[3].addValue = 7541;

    value[3].seed2.l2[1] = 0x0;
    value[3].seed2.l2[0] = 21001;
    value[3].addValue2 = 21383;

    value[3].mulitiplyValue = 0x40000001;   //  0x7fffffff  ;  //  This is Mersenne Prime 2147483647 ;




}

static void
printLong (unsigned long x)
{
    printf ("  %lx   %ld  \n", x, x);
}

static void
printLongLong (unsigned long long x)
{
    printf ("  %llx   %lld  \n", x, x);
}

static void
printFloat (float x)
{
    printf ("  %x   %e  \n", (unsigned int) x, x);
}



static void
fprintLongLong (FILE * p, int *i, unsigned long long v)
{
    int ii;
    ii = *i;
    if (ii >= DEBUG_COUNTER)
        return;
    fprintf (p, "->  %llx  \n", v);
    ii++;
    *i = ii;
}





static void
write_X_Y_W_to_file (FILE * fp, float *scratch1, float *scratch2, int N)
{
    int i;
    float *p1, *p2, *p3;

    p1 = (float *) scratch1;
    p2 = (float *) scratch1 + 1;
    p3 = (float *) scratch2;
    for (i = 0; i < N / 2; i++)
    {
        fprintf (fp, " #%d  ->  %f %f %f \n", i, *p1, *p2, *p3++);
        p1 = p1 + 2;
        p2 = p2 + 2;

    }
}

static void
write_float_file (FILE * fp, float *scratch1, int N)
{
    int i, j;
    float *p1;

    p1 = (float *) scratch1;
    j = 0;

    for (i = 0; i < N; i++)
    {
        if ((j % 8 == 0))
        {
            fprintf (fp, " \n #%d  ->  ", i);
        }
        fprintf (fp, " %f ", *p1++);
        j++;



    }
}
