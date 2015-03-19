/*
 * ====================================================
 * Copyright (C) 2014 by Texas Instruments, Inc. All rights reserved.
 * ====================================================
 */

#include "cdefs-compat.h"
#include "openlibm.h"
#include "math_private.h"
#include "c66_helper.h"

DLLEXPORT float __fnormalize(float x, int *powof2)
{
    *powof2 = 0;

    int32_t ix = _ftoi(x);
    int32_t save_sign = ix & 0x80000000;

    int lmb  = _lmbd(1, MANTF(x)); // assert 9 <= lmb <= 31
    int shft = lmb - 8;

    *powof2 = shft;
    return _itof((ix << shft) | save_sign); 
}


DLLEXPORT float __fmpy_by_0x1p25(float x)
{
    int powof2;
    x = __fnormalize(x, &powof2);

    union IEEEf2bits u; u.f = x;
    u.bits.exp += 25 - powof2;
    return u.f;
}

DLLEXPORT float __fmpy_by_0x1p24(float x)
{
    int powof2;
    x = __fnormalize(x, &powof2);

    union IEEEf2bits u; u.f = x;
    u.bits.exp += 24 - powof2;
    return u.f;
}

DLLEXPORT double __dnormalize(double x, int *powof2)
{
    *powof2 = 0;

    int64_t ix = _dtol(x);
    int64_t save_sign = ix & 0x8000000000000000;

    int lmb  = _lmbd(1, MANTD_HI(x));
    if (lmb == 32) lmb += _lmbd(1, MANTD_LO(x));
    int shft = lmb - 11;

    *powof2 = shft;
    return _ltod((ix << shft) | save_sign); 
}


DLLEXPORT double __dmpy_by_0x1p54(double x)
{
    int powof2;
    x = __dnormalize(x, &powof2);

    union IEEEd2bits u; u.d = x;
    u.bits.exp += 54 - powof2;
    return u.d;
}
