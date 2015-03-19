/* @(#)e_fmod.c 1.3 95/01/18 */
/*-
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Copyright (C) 2014 by Texas Instruments, Inc. 
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

#include "cdefs-compat.h"
//__FBSDID("$FreeBSD: src/lib/msun/src/s_remquof.c,v 1.1 2005/03/25 04:40:44 das Exp $");

#include "openlibm.h"
#include "math_private.h"

static const float Zero[] = {0.0, -0.0,};

/*
 * Return the IEEE remainder and set *quo to the last n bits of the
 * quotient, rounded to the nearest integer.  We choose n=31 because
 * we wind up computing all the integer bits of the quotient anyway as
 * a side-effect of computing the remainder by the shift and subtract
 * method.  In practice, this is far more bits than are needed to use
 * remquo in reduction algorithms.
 */
DLLEXPORT float
remquof(float x, float y, int *quo)
{
	int32_t n,hx,hy,hz,ix,iy,sx,i;
	u_int32_t q,sxy;

	GET_FLOAT_WORD(hx,x);
	GET_FLOAT_WORD(hy,y);

    /* TI: flush denormalized inputs to zero */
        if (((hx>>23)&0xFF) == 0) { x = 0.0f; hx = 0; }
        if (((hy>>23)&0xFF) == 0) { y = 0.0f; hy = 0; }

	sxy = (hx ^ hy) & 0x80000000;
	sx = hx&0x80000000;		/* sign of x */
	hx ^=sx;		/* |x| */
	hy &= 0x7fffffff;	/* |y| */

    /* purge off exception values */
	if(hy==0||hx>=0x7f800000||hy>0x7f800000) /* y=0,NaN;or x not finite */
        {
            *quo = 0;
	    return (x*y)/(x*y);
        }
	if(hx<hy) {
	    q = 0;
	    goto fixup;	/* |x|<|y| return x or x-y */
	} else if(hx==hy) {
	    *quo = (sxy ? -1 : 1);  /* TI fix */
            return Zero[(u_int32_t)sx>>31];	/* |x|=|y| return x*0*/
	}

    /* determine ix = ilogb(x) */
	if(hx<0x00800000) {	/* subnormal x */
	    for (ix = -126,i=(hx<<8); i>0; i<<=1) ix -=1;
	} else ix = (hx>>23)-127;

    /* determine iy = ilogb(y) */
	if(hy<0x00800000) {	/* subnormal y */
	    for (iy = -126,i=(hy<<8); i>0; i<<=1) iy -=1;
	} else iy = (hy>>23)-127;

    /* set up {hx,lx}, {hy,ly} and align y to x */
	if(ix >= -126)
	    hx = 0x00800000|(0x007fffff&hx);
	else {		/* subnormal x, shift x to normal */
	    n = -126-ix;
	    hx <<= n;
	}
	if(iy >= -126)
	    hy = 0x00800000|(0x007fffff&hy);
	else {		/* subnormal y, shift y to normal */
	    n = -126-iy;
	    hy <<= n;
	}

    /* fix point fmod */
	n = ix - iy;
	q = 0;
	while(n--) {
	    hz=hx-hy;
	    if(hz<0) hx = hx << 1;
	    else {hx = hz << 1; q++;}
	    q <<= 1;
	}
	hz=hx-hy;
	if(hz>=0) {hx=hz;q++;}

    /* convert back to floating value and restore the sign */
	if(hx==0) {				/* return sign(x)*0 */
	    *quo = (sxy ? -q : q);
	    return Zero[(u_int32_t)sx>>31];
	}
	while(hx<0x00800000) {		/* normalize x */
	    hx <<= 1;
	    iy -= 1;
	}
	if(iy>= -126) {		/* normalize output */
	    hx = ((hx-0x00800000)|((iy+127)<<23));
	} else {		/* subnormal output */
	    n = -126 - iy;
	    hx >>= n;
	}
fixup:
	SET_FLOAT_WORD(x,hx);
	y = fabsf(y);
	if (y < 0x1p-125f) {
    /* TI: x could be degenerated, (x+x, x-y) won't work anymore */
    /* TI: now x, y only contain mantissa, compute with mantissa: hx, hy */
	    GET_FLOAT_WORD(hy,y);
	    /* if (x+x>y || (x+x==y && (q & 1))) { */
	    if (hx+hx>hy || (hx+hx==hy && (q & 1))) {
		q++;
		/* x-=y; */
                hx -= hy;
                if (hx < 0) hx = (-hx) | 0x80000000;
	        SET_FLOAT_WORD(x,hx);
	    }
	} else if (x>0.5f*y || (x==0.5f*y && (q & 1))) {
	    q++;
	    x-=y;
	}
	GET_FLOAT_WORD(hx,x);
	SET_FLOAT_WORD(x,hx^sx);
	/* q &= 0x7fffffff; */
	q &= 0x7f;  /* TI: OpenCL: quo stores last 7 bits */
	*quo = (sxy ? -q : q);
	return x;
}
