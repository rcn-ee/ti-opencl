/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Copyright (C) 2014 by Texas Instruments, Inc. 
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include "cdefs-compat.h"
//__FBSDID("$FreeBSD: src/lib/msun/src/s_frexpf.c,v 1.10 2008/02/22 02:30:35 das Exp $");

#include "openlibm.h"
#include "math_private.h"
#include "c66_helper.h"

#define FLOAT_SUBNORMAL  0
#define DOUBLE_SUBNORMAL 1

DLLEXPORT float
frexpf(float x, int *eptr)
{
    int32_t hx = _ftoi(x);
    int32_t ix = 0x7fffffff & hx;
    *eptr = 0;

    if (ISINFORNANF(x) || (ix==0)) return x;	/* 0,inf,nan */

    /* subnormal */

#if FLOAT_SUBNORMAL
    if (ix < 0x00800000) 
    {		
        int lmb  = _lmbd(1, ix); // assert 9 <= lmb <= 31
        int shft = lmb - 8;
        ix <<= shft;
        hx = ix | (hx&0x80000000);
        *eptr = -shft;
    }
#else 
    if (ix < 0x00800000) return 0.0f;
#endif

    *eptr += (ix >> 23) - 126;

    return _itof((hx & 0x807fffff) | 0x3f000000);
}

DLLEXPORT double
frexp(double x, int *eptr)
{
    double remember_sign = x;
    *eptr = 0;
    if (ISNAND(x) || ((_dtoll(x)<<1)==0)) return x;	/* 0,inf,nan */

#if DOUBLE_SUBNORMAL
    if (ISDENORMD(x)) 
    {		
        int lmb  = _lmbd(1, MANTD_HI(x)); 
        if (lmb == 32) lmb += _lmbd(1, MANTD_LO(x)); 
        int shft = lmb - 11;
        x = _lltod(_dtoll(x) << shft);
        *eptr = -shft;
    }
#else
    if (ISDENORMD(x)) return 0.0;
#endif

    *eptr += EXPD(x) - 1022;

    return copysign(_itod(((_hi(x) & 0x800fffff) | (1022) << 20), _lo(x)), remember_sign);
}

#if LDBL_MANT_DIG == DBL_MANT_DIG
DLLEXPORT long double frexpl(long double x, int *eptr) __attribute__((__alias__("frexp")));
#endif
