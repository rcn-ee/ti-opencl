/*-
 * Copyright (c) 2004 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/msun/arm/fenv.c,v 1.3 2011/10/16 05:37:56 das Exp $
 */

#include "fenv.h"

/*
 * Hopefully the system ID byte is immutable, so it's valid to use
 * this as a default environment.
 */
const fenv_t __fe_dfl_env = 0;

extern inline int feclearexcept(int __excepts);
extern inline int fegetexceptflag(fexcept_t *__flagp, int __excepts);
extern inline int fesetexceptflag(const fexcept_t *__flagp, int __excepts);
extern inline int feraiseexcept(int __excepts);
extern inline int fetestexcept(int __excepts);
extern inline int fegetround(void);
extern inline int fesetround(int __round);
extern inline int fegetenv(fenv_t *__envp);
extern inline int feholdexcept(fenv_t *__envp);
extern inline int fesetenv(const fenv_t *__envp);
extern inline int feupdateenv(const fenv_t *__envp);

extern __cregister volatile unsigned int FADCR;
extern __cregister volatile unsigned int FMCR;

unsigned int c66_get_round()
{
    /*-------------------------------------------------------------------------
    * There are actually 4 sets of bits for l1,l2,m1 and m2. Assume they agree
    *------------------------------------------------------------------------*/
    return _extu(FADCR, 21, 30);
}

unsigned int c66_set_round_reg(unsigned int t, unsigned int __round)
{
    __round &= 3;

    switch (__round)
    {
        case 0: t = _clr(t, 9, 10);  t = _clr(t, 25, 26); break;

        case 1: t = _clr(t, 9, 10);  t = _clr(t, 25, 26);
                t = _set(t, 9, 9);   t = _set(t, 25, 25); break;

        case 2: t = _clr(t, 9, 10);  t = _clr(t, 25, 26);
                t = _set(t, 10, 10); t = _set(t, 26, 26); break;

        case 3: t = _set(t, 9, 10);  t = _set(t, 25, 26); break;
    }

    return t;
}

void c66_set_round(int __round)
{
     FADCR = c66_set_round_reg(FADCR, __round);
     FMCR  = c66_set_round_reg(FMCR,  __round);
}

