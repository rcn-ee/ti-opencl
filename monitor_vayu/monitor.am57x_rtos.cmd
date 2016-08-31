/******************************************************************************
 * Copyright (c) 2013-2016, Texas Instruments Incorporated - http://www.ti.com/
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

/*-----------------------------------------------------------------------------
* Monitor requires run time initialization (rom model) as it requires
* initialization of L2 data
*----------------------------------------------------------------------------*/
--rom_model

--retain="*/dsp.lib<*>(*)"
--retain="*/libm.lib<*>(*)"
--retain="GOMP_*"
--retain="omp_*"
--retain="__gomp_flush0"
--retain="__ti_omp_dsp*"
--retain="EdmaMgr_*"
--retain="__touch"
--retain="__core_num"
--retain="__sem_*"
--retain="_local_*"
--retain="ti_sysbios_family_c66_Cache_w*"
--retain="ti_sysbios_family_c66_Cache_inv*"

--retain="printf"
--retain="puts"
--retain="_minit"
--retain="malloc"
--retain="calloc"
--retain="realloc"
--retain="log"
--retain="logf"
--retain="sqrt"
--retain="__heap_init*"
--retain="__malloc_*"
--retain="__calloc_*"
--retain="__realloc_*"
--retain="__free_*"
--retain="__memalign_*"

#define L2_LINE_SIZE 128

SECTIONS
{
    .text:
    {
        *(.text:ti_sysbios_family_shared_vayu_Mmu_initTableBuf__I)
        . = align(0x800);
        *(.text)
    } load > DDR3
    .ti.decompress: load > DDR3
    .stack: load > L2SRAM
    GROUP: load > DDR3
    {
        .bss:
        .neardata:
        .rodata:
    }
    .cinit: load > DDR3
    .pinit: load >> L2SRAM
    .init_array: load > L2SRAM
    .const: load >> DDR3
    .data: load >> L2SRAM
    .switch: load >> DDR3
    .sysmem: load > DDR3
    .args: load > L2SRAM align = 0x4, fill = 0 {_argsize = 0x0; }
    .cio: load >> L2SRAM
    .ti.handler_table: load > L2SRAM
    .c6xabi.extab: load >> L2SRAM
    .ddr: load > DDR3
    .private: load > L2SRAM, fill = 0x0
    .fast_shared_noncached: load > L2SRAM, fill = 0x0
}

SECTIONS
{
    /*-------------------------------------------------------------------------
    * Needed for MPM on Hawking, which has a 10 bit alignment rqmt for the 
    * entry point.  Can be removed when that reqmt is lifted.
    *------------------------------------------------------------------------*/
    .text:_c_int00 > DDR3 align(0x400)

    .workgroup_config: > L2SRAM  palign(L2_LINE_SIZE)

    .mbox_d2h:         > L2SRAM, fill=0
		          load_start(mbox_d2h_phys) size(mbox_d2h_size) 

    .mbox_h2d:         > L2SRAM, fill=0
		          load_start(mbox_h2d_phys) size(mbox_h2d_size) 
}

/*-----------------------------------------------------------------------------
* Define symbols that mark the OpenCL global and local memory ranges.
*----------------------------------------------------------------------------*/
ocl_l1d_mem_start    = start(L1DSRAM);
ocl_l1d_mem_size     = size (L1DSRAM);

ocl_local_mem_start  = start(OCL_LOCAL);
ocl_local_mem_size   = size (OCL_LOCAL);

nocache_phys_start   = start(OCL_OMP_NOCACHE);
nocache_virt_start   = start(OCL_OMP_NOCACHE);
nocache_size         = size(OCL_OMP_NOCACHE);

nocache2_phys_start   = start(SR_0);
nocache2_virt_start   = start(SR_0);
nocache2_size         = size(SR_0);

service_stack_start   = start(OCL_OMP_STACK);
service_stack_size    = size(OCL_OMP_STACK);

--export ocl_l1d_mem_start
--export ocl_l1d_mem_size
--export ocl_local_mem_start
--export ocl_local_mem_size


/*-----------------------------------------------------------------------------
* Place the far data from the framework components into l2 rather than ddr, 
* becuase they are core private
*----------------------------------------------------------------------------*/
SECTIONS
{
    /*
     *  The .far and .fardata sections of FC and EDMA3LLD need to be local to each core.
     *  These sections are therefore placed in memory local to each core.
     */
    .fclocalfar :
    {
                "ecpy.ae66"         (.fardata)
                "edma3.ae66"        (.fardata)
                /* "edma3Chan.ae66"    (.fardata) */
                "edma3_lld_rm.ae66" (.fardata)
                "edmamgr.ae66"      (.fardata)
                "fcsettings.ae66"   (.fardata)
                "rman.ae66"         (.fardata)

                "edma3.ae66"        (.far)
                /* "edma3Chan.ae66"    (.far) */
                "edma3_lld_rm.ae66" (.far)
                "edmamgr.ae66"      (.far)
                "fcsettings.ae66"   (.far)
                "rman.ae66"         (.far)
    } > DDR3
    .fclocalfarsyms :
    {
                "nullres.ae66"      (.fardata)
                "nullres.ae66"      (.far)
    } > L2SRAM

    .fardata: load >> DDR3
    .far: load >> DDR3
    .gdb_server: load = L2SRAM, type = NOLOAD, fill = 0x0
}

--symbol_map=acosd=acos
--symbol_map=acoshd=acosh
--symbol_map=acospid=acospi
--symbol_map=asind=asin
--symbol_map=asinhd=asinh
--symbol_map=asinpid=asinpi
--symbol_map=atand=atan
--symbol_map=atan2d=atan2
--symbol_map=atanhd=atanh
--symbol_map=atanpid=atanpi
--symbol_map=atan2pid=atan2pi
--symbol_map=cbrtd=cbrt
--symbol_map=ceild=ceil
--symbol_map=copysignd=copysign
--symbol_map=cosd=cos
--symbol_map=coshd=cosh
--symbol_map=cospid=cospi
--symbol_map=erfcd=erfc
--symbol_map=erfd=erf
--symbol_map=expd=exp
--symbol_map=exp2d=exp2
--symbol_map=exp10d=exp10
--symbol_map=expm1d=expm1
--symbol_map=fabsd=fabs
--symbol_map=fdimd=fdim
--symbol_map=floord=floor
--symbol_map=fmad=fma
--symbol_map=fmaxd=fmax
--symbol_map=fmind=fmin
--symbol_map=fmodd=fmod
--symbol_map=fractd=fract
--symbol_map=frexpd=frexp
--symbol_map=hypotd=hypot
--symbol_map=ilogbd=ilogb
--symbol_map=ldexpd=ldexp
--symbol_map=lgammad=lgamma
--symbol_map=lgammad_r=lgamma_r
--symbol_map=logd=log
--symbol_map=log2d=log2
--symbol_map=log10d=log10
--symbol_map=log1pd=log1p
--symbol_map=logbd=logb
--symbol_map=madd=mad
--symbol_map=maxmagd=maxmag
--symbol_map=minmagd=minmag
--symbol_map=modfd=modf
--symbol_map=nand=nan
--symbol_map=nextafterd=nextafter
--symbol_map=powd=pow
--symbol_map=pownd=pown
--symbol_map=powrd=powr
--symbol_map=remainderd=remainder
--symbol_map=remquod=remquo
--symbol_map=rintd=rint
--symbol_map=rootnd=rootn
--symbol_map=roundd=round
--symbol_map=rsqrtd=rsqrt
--symbol_map=sind=sin
--symbol_map=sincosd=sincos
--symbol_map=sinhd=sinh
--symbol_map=sinpid=sinpi
--symbol_map=sqrtd=sqrt
--symbol_map=tand=tan
--symbol_map=tanhd=tanh
--symbol_map=tanpid=tanpi
--symbol_map=tgammad=tgamma
--symbol_map=truncd=trunc
--symbol_map=native_cosd=native_cos
--symbol_map=native_divided=native_divide
--symbol_map=native_expd=native_exp
--symbol_map=native_exp2d=native_exp2
--symbol_map=native_exp10d=native_exp10
--symbol_map=native_logd=native_log
--symbol_map=native_log2d=native_log2
--symbol_map=native_log10d=native_log10
--symbol_map=native_powrd=native_powr
--symbol_map=native_recipd=native_recip
--symbol_map=native_rsqrtd=native_rsqrt
--symbol_map=native_sind=native_sin
--symbol_map=native_sqrtd=native_sqrt
--symbol_map=native_tand=native_tan
--symbol_map=isinfd=isinf
--symbol_map=isnand=isnan
--symbol_map=__isfinited=__isfinite
--symbol_map=__isnormald=__isnormal

/*-----------------------------------------------------------------------------
* __ocl_<x> names deprecated, but created here for transition period
*----------------------------------------------------------------------------*/
--symbol_map=__ocl_cache_l1d_off=__cache_l1d_none
--symbol_map=__ocl_cache_l1d_std=__cache_l1d_all
--symbol_map=__ocl_cache_l1d_half=__cache_l1d_16k
--symbol_map=__ocl_cache_l1d_wbinv_all=__cache_l1d_wbinv_all
