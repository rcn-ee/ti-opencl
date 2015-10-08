include ../host/Makefile.inc

ifeq ($(TI_OCL_CGT_INSTALL),)
   $(error Builtins build requires TI_OCL_CGT_INSTALL env var for C6x CGT toolset)
endif

AR=$(TI_OCL_CGT_INSTALL)/bin/ar6x
CC=$(TI_OCL_CGT_INSTALL)/bin/cl6x 
CFLAGS=-o3 -mv6600 --use_g3 --abi=eabi --mem_model:data=far -I$(TI_OCL_CGT_INSTALL)/include -Isrc -Iinclude -Ic66 -eoo -DASSEMBLER -D__BSD_VISIBLE --visibility=protected 

VPATH=c66:src:bsdsrc
SRCS =


#---------------------------
# FROM C66 subdir
#---------------------------
SRCS += c_div.c
SRCS += c_exp10.c
SRCS += c_floorf.c
SRCS += c_fmaf.c
SRCS += c_fmax.c
SRCS += c_hypotf.c
SRCS += c_ldexp.c
SRCS += c_maxmag.c
SRCS += c_powr.c
SRCS += c_pown.c
SRCS += c_remquof.c
SRCS += c_rootn.c
SRCS += c_rsqrt.c
SRCS += c_sinpi.c
SRCS += c_sqrt.c
SRCS += c_truncf.c
SRCS += c_frexp.c
SRCS += c_recip.c
SRCS += c_exp10.c
SRCS += c_fpconv.c
SRCS += c_outlined.c
SRCS += fenv.c
SRCS += normalize.c

#---------------------------
# FROM BSDSRC subdir
#---------------------------
SRCS += b_exp.c
SRCS += b_log.c
SRCS += b_tgamma.c

#---------------------------
# FROM SRC subdir
#---------------------------
SRCS += common.c
SRCS += e_acos.c
SRCS += e_acosf.c
SRCS += e_acosh.c
SRCS += e_acoshf.c
SRCS += e_asin.c
SRCS += e_asinf.c
SRCS += e_atan2.c
SRCS += e_atan2f.c
SRCS += e_atanh.c
SRCS += e_atanhf.c
SRCS += e_cosh.c
SRCS += e_coshf.c
SRCS += e_exp.c
SRCS += e_expf.c
SRCS += e_fmod.c
SRCS += e_fmodf.c
SRCS += e_hypot.c
###YUAN SRCS += e_hypotf.c
SRCS += e_lgamma.c
SRCS += e_lgammaf.c
SRCS += e_lgammaf_r.c
SRCS += e_lgamma_r.c
SRCS += e_log10.c
SRCS += e_log10f.c
SRCS += e_log2.c
SRCS += e_log2f.c
SRCS += e_log.c
SRCS += e_logf.c
SRCS += e_pow.c
SRCS += e_powf.c
SRCS += e_remainder.c
SRCS += e_remainderf.c
SRCS += e_rem_pio2.c
SRCS += e_rem_pio2f.c
SRCS += e_scalb.c
SRCS += e_scalbf.c
SRCS += e_sinh.c
SRCS += e_sinhf.c
SRCS += k_cos.c
SRCS += k_cosf.c
SRCS += k_exp.c
SRCS += k_expf.c
SRCS += k_rem_pio2.c
SRCS += k_sin.c
SRCS += k_sinf.c
SRCS += k_tan.c
SRCS += k_tanf.c
SRCS += s_asinh.c
SRCS += s_asinhf.c
SRCS += s_atan.c
SRCS += s_atanf.c
SRCS += s_cbrt.c
SRCS += s_cbrtf.c
SRCS += s_ceil.c
SRCS += s_ceilf.c
SRCS += s_copysign.c
SRCS += s_copysignf.c
SRCS += s_cos.c
SRCS += s_cosf.c
SRCS += s_erf.c
SRCS += s_erff.c
SRCS += s_exp2.c
SRCS += s_exp2f.c
SRCS += s_expm1.c
SRCS += s_expm1f.c
SRCS += s_fabs.c
SRCS += s_fabsf.c
SRCS += s_fdim.c
SRCS += s_finite.c
SRCS += s_finitef.c
SRCS += s_floor.c
SRCS += s_fma.c
SRCS += s_fpclassify.c
SRCS += s_ilogb.c
SRCS += s_ilogbf.c
SRCS += s_isfinite.c
SRCS += s_isinf.c
SRCS += s_isnan.c
SRCS += s_isnormal.c
SRCS += s_log1p.c
SRCS += s_log1pf.c
SRCS += s_logb.c
SRCS += s_logbf.c
SRCS += s_modf.c
SRCS += s_modff.c
SRCS += s_nextafter.c
SRCS += s_nextafterf.c
SRCS += s_remquo.c
SRCS += s_rint.c
SRCS += s_rintf.c
SRCS += s_round.c
SRCS += s_roundf.c
SRCS += s_scalbn.c
SRCS += s_scalbnf.c
SRCS += s_signbit.c
SRCS += s_signgam.c
SRCS += s_sin.c
SRCS += s_sincos.c
SRCS += s_sincosf.c
SRCS += s_sinf.c
SRCS += s_tan.c
SRCS += s_tanf.c
SRCS += s_tanh.c
SRCS += s_tanhf.c
SRCS += s_tgammaf.c
SRCS += s_trunc.c

OBJST = $(patsubst %.c,%.o,$(SRCS))
OBJS = $(patsubst %.asm,%.o,$(OBJST))

libm.lib: $(OBJS)
	@$(AR) r libm.lib $(OBJS)

%.o:%.c
	@echo cl6x ... $<
	@$(CC) $(CFLAGS) $<

%.o:%.asm
	@echo cl6x ... $<
	@$(CC) $(CFLAGS) $<

clean:
	rm -f *.o libm.lib *.asm

