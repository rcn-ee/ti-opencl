/* s_scalbnf.c -- float version of s_scalbn.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Copyright (C) 2014 by Texas Instruments, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include "cdefs-compat.h"
#include "openlibm.h"
#include "math_private.h"
#include "c66_helper.h"
#include "stdio.h"

static const float
//two25   =  0x1p25, 
twom25  =  0x1p-25, 
huge   = 1.0e+30,
tiny   = 1.0e-30;

DLLEXPORT float ldexpf (float x, int n)
{
    int32_t ix = _ftoi(x);
    int32_t k  = EXPF(x);
    int32_t save_sign = ix & 0x80000000;

    if (k==0) 
    {				/* 0 or subnormal x */
        if (MANTF(x) == 0) return x; /* +-0 */

        // x *= two25; 
        /*---------------------------------------------------------------------
        * c6x mpysp flushs denorms to 0, so the above will not work.  we 
        * simulate it with the below
        *--------------------------------------------------------------------*/
        int lmb  = _lmbd(1, MANTF(x)); // assert 9 <= lmb <= 31
        int shft = lmb - 8;
        x  = _itof((ix << shft) | save_sign); 

        ix = _ftoi(x);
        k = EXPF(x) - shft;
        if (n < -50000) return tiny * x; /*underflow*/
    }

    if (k == 0xff) return x+x;		/* NaN or Inf */

    k += n;

    /*-------------------------------------------------------------------------
    * overflow
    *------------------------------------------------------------------------*/
    if (k >  0xfe) return huge*copysignf(huge,x); 

    /*-------------------------------------------------------------------------
    * normal result
    *------------------------------------------------------------------------*/
    if (k > 0) return _itof((ix & 0x807fffff) | (k<<23));

    if (k <= -25) 
    {
        if (n > 50000) 	/* in case integer overflow in n+k */
             return huge * copysignf(huge,x);	/*overflow*/
        else return tiny * copysignf(tiny,x);	/*underflow*/
    }

    k += 25;				/* subnormal result */
    return twom25 * _itof((ix & 0x807fffff) | (k<<23));
}

static const double
//two54   =  1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
twom54  =  5.55111512312578270212e-17, /* 0x3C900000, 0x00000000 */
dhuge   = 1.0e+300,
dtiny   = 1.0e-300;

DLLEXPORT double ldexp (double x, int n)
{
    int32_t k,hx,lx;
    EXTRACT_WORDS(hx,lx,x);
    k = (hx&0x7ff00000)>>20;                /* extract exponent */

    if (k==0) /* 0 or subnormal x */
    {                             
        if ((lx|(hx&0x7fffffff))==0) return x; /* +-0 */

        int lmb  = _lmbd(1, MANTD_HI(x));
        if (lmb == 32) lmb += _lmbd(1, MANTD_LO(x));
        int shft = lmb - 11;

        x = _lltod((_dtoll(x) << shft) | hx&0x8000000000000000);

        GET_HIGH_WORD(hx,x);

        k = ((hx&0x7ff00000)>>20) - shft;

        if (n< -50000) return dtiny*x;       /*underflow*/
    }

    if (k==0x7ff) return x+x;               /* NaN or Inf */
    k = k+n;
    if (k >  0x7fe) return dhuge*copysign(dhuge,x); /* overflow  */
    if (k > 0)                              /* normal result */
        {SET_HIGH_WORD(x,(hx&0x800fffff)|(k<<20)); return x;}
    if (k <= -54) {
        if (n > 50000)      /* in case integer overflow in n+k */
            return dhuge*copysign(dhuge,x);   /*overflow*/
        else return dtiny*copysign(dtiny,x);  /*underflow*/
    }
    k += 54;                                /* subnormal result */
    SET_HIGH_WORD(x,(hx&0x800fffff)|(k<<20));
    return x*twom54;
}

#if LDBL_MANT_DIG == DBL_MANT_DIG
DLLEXPORT long double  ldexpl(long double x, int n) __attribute__((__alias__("ldexp")));
#endif
