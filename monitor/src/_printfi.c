/*****************************************************************************/
/*  _PRINTFI.C                                                               */
/*                                                                           */
/* Copyright (c) 1995 Texas Instruments Incorporated                         */
/* http://www.ti.com/                                                        */
/*                                                                           */
/*  Redistribution and  use in source  and binary forms, with  or without    */
/*  modification,  are permitted provided  that the  following conditions    */
/*  are met:                                                                 */
/*                                                                           */
/*     Redistributions  of source  code must  retain the  above copyright    */
/*     notice, this list of conditions and the following disclaimer.         */
/*                                                                           */
/*     Redistributions in binary form  must reproduce the above copyright    */
/*     notice, this  list of conditions  and the following  disclaimer in    */
/*     the  documentation  and/or   other  materials  provided  with  the    */
/*     distribution.                                                         */
/*                                                                           */
/*     Neither the  name of Texas Instruments Incorporated  nor the names    */
/*     of its  contributors may  be used to  endorse or  promote products    */
/*     derived  from   this  software  without   specific  prior  written    */
/*     permission.                                                           */
/*                                                                           */
/*  THIS SOFTWARE  IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS    */
/*  "AS IS"  AND ANY  EXPRESS OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT    */
/*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR    */
/*  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT    */
/*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    */
/*  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT  NOT    */
/*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,    */
/*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    */
/*  THEORY OF  LIABILITY, WHETHER IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE    */
/*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*    This file contains the main routines that all eight variations of the  */
/*    printf function use.  The main function in the file is __TI_printfi,   */
/*    and the other functions here are called by it.                         */
/*                                                                           */
/*    All of these functions are non-standard and are provided only as       */
/*    buildling blocks for printf.  These functions are not meant to be      */
/*    called directly by any user code, and are subject to change or         */
/*    removal without notice                                                 */
/*                                                                           */
/* FUNCTIONS:                                                                */
/*    __TI_printfi   -  Perform the main printf routine                      */
/*    _pproc_fflags  -  Process the format flags for a conversion            */
/*    _pproc_fwp     -  Convert field width and precision into numbers       */
/*    _pproc_str     -  Process the string (%s) conversion                   */
/*    _setfield      -  Performs conversions when the '%' is encountered     */
/*    _pproc_fgea    -  Process the conversion for f, g, G, e, E, a and A    */
/*    fcvt           -  Convert and round to the requested frac digits       */
/*    ecvt           -  Convert and round to the requested sig digits        */
/*    acvt           -  Convert and round to the requested frac digits       */
/*    _pconv_f       -  Perform the %f conversion                            */
/*    _pconv_e       -  Perform the %e conversion                            */
/*    _pconv_g       -  Perform the %g conversion                            */
/*    _pconv_a       -  Perform the %a conversion                            */
/*    _fcpy          -  Copy the fraction part of a float to a string        */
/*    _ecpy          -  Copy the "E+xx" part of a float to a string          */
/*    _mcpy          -  Copy the whole number part of a float to a string    */
/*    _pproc_diouxp  -  Process the conversion for d, i, o, u, x, and p      */
/*    _getarg_diouxp -  Get the argument for d, i, o, u, x, or p conversion  */
/*    _ltostr        -  Convert an integer to a string of up to base 16      */
/*    _div           -  Specialized integer division, used by _ltostr        */
/*    _pproc_fgea_flag_sign_conv - Process flags, sign, conversion for       */
/*                                 f, g, G, e, E, a, and A                   */
/*                                                                           */
/* FUNCTIONS IMPLEMENTED FOR VECTOR TYPE SUPPORT:                            */
/*    _pproc_vn             - Process vector format specifier                */
/*    _pproc_v_fgea         - Process vectors for f, g, G, e, E, a and A     */
/*    _pproc_v_dioux        - Process vectors for d, i, o, u, and x          */
/*    _pproc_v_dioux_elem   - Process a single vector integer element        */
/*    _pproc_v_fgea_elem    - Process a single vector floating-point element */
/*    _pproc_v_elem_prelude - Process flags, width and precision for element */
/*    _process_v            - Performs vector conversions when '%' found     */
/*                                                                           */
/* Note:  If NOFLOAT is defined at compile time, this file will be compiled  */
/*        without floating point support.                                    */
/*****************************************************************************/
#include <stdio.h>
#include <format.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>
#include <wchar.h>
#include <errno.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Log.h>
#ifndef VECTOR_PRINTF_SUPPORT
#define VECTOR_PRINTF_SUPPORT
#endif

/******************************************************************************/
/* For vector datatypes cl_long2 and cl_double2, the alignment of the datatype*/
/* should be 16 bytes as per the OpenCL specification. The TI CGT only        */
/* guarantees 8 byte alignment for the corresponding longlong2 and double2    */
/* data types.                                                                */
/* The alignment is done relative to the start of the stack and therefore     */
/* We must hold on to the start of the va_list i.e. _ap_orig in order to      */
/* calculate the alignment difference between it and _ap for the vector data. */
/* An offset is calculated based on the difference in alignment using the     */
/* __TI_ALIGN_128BIT_PAD macro and added to the _ap to obtain the address of  */
/* the vector data payload for _ap.                                           */
/******************************************************************************/
#define __TI_ALIGN_128BIT_PAD(eb, e)                 \
        ((((unsigned)(e - eb) & 0xF) == 0x0) ? 0x8 : \
        ((((unsigned)(e - eb) & 0xF) == 0x8) ? 0x0 : \
          ((unsigned)(e - eb) & 0xF)))

#define __TI_ALIGN_128BIT(eb, e)                                 \
        ((char *)(((unsigned)(e) + __TI_ALIGN_128BIT_PAD(eb, e))))

#define va_arg_128bit(_ap_orig, _ap, _type)                       \
        (_ap = __TI_ALIGN_128BIT(_ap_orig, _ap),                  \
         _ap += __TI_SIZEOF_PROMOTED_TYPE(_type),                 \
         __TI_DEREF(_ap - __TI_SIZEOF_PROMOTED_TYPE(_type), _type))

#define HEX_CONV(conv) (conv == 'x' || conv == 'X' || conv == 'p')

#define SIGNED_CONV(conv) (conv != 'u' && conv != 'o' && !HEX_CONV(conv))

enum { NO_FLAG, MINUS_FLAG, ISNAN_FLAG, ISPINF_FLAG, ISNINF_FLAG };

_CODE_ACCESS char *memccpy(char *dest, const char *src, int ch, int count);
_CODE_ACCESS int __TI_ltoa(long val, char *buffer);


#if !defined(NOFLOAT) && !defined(MINIMAL) && !defined(VECTOR_PRINTF_SUPPORT)
#define _PRINTFI __TI_printfi
#elif defined(NOFLOAT)
#define _PRINTFI __TI_printfi_nofloat
#elif defined(MINIMAL)
#define _PRINTFI __TI_printfi_minimal
#elif defined(VECTOR_PRINTF_SUPPORT)
#define _PRINTFI __TI_printfi_vector
#endif

extern _CODE_ACCESS int _PRINTFI(char ** __restrict _format,
                                 va_list _ap, void * __restrict _op,
                                 int (*_outc)(char, void *),
				 int (*_outs)(char *, void *, int));

#if !defined(MINIMAL)
static void _pproc_fflags(_PFIELD *pfield, char **it);
static void _pproc_fwp(_PFIELD *pfield, char **it, va_list *_ap);
#endif
static void _pproc_str(_PFIELD *pfield, void *_op, va_list *_ap, int *count,
                       int (*_outc)(char, void *));
#if !defined(MINIMAL)
static void _pproc_wstr(_PFIELD *pfield, void *_op, va_list *_ap, int *count,
                        int (*_outc)(char, void *));
#endif
static int _setfield(char *fld, _PFIELD *pfield, va_list *_ap);

#if !defined(NOFLOAT) && !defined(MINIMAL)
static char *fcvt(char *buf, long double value,int ndigit,int *decpt);
static char *ecvt(char *buf, long double value,int ndigit,int *decpt);
static char *acvt(char *buf, long double value,int ndigit,int *decpt, int conv);
static void _pproc_fgea(_PFIELD *pfield, int *flags, char **a_it, va_list *_ap);
static void _pconv_f(long double cvt, _PFIELD *pfield, char **a_it);
static void _pconv_e(long double cvt, _PFIELD *pfield, char **a_it);
static void _pconv_g(long double cvt, _PFIELD *pfield, char **a_it);
static void _pconv_a(long double cvt, _PFIELD *pfield, char **a_it);

static void _fcpy(const char *tmpbuf, int dpt, int precision, char **a_it);
static void _ecpy(int exp, char letter, char **a_it);
static void _mcpy(const char *tmpbuf, int dpt, int putdec, char **a_it);
#if !defined(VECTOR_PRINTF_SUPPORT)
__attribute__((always_inline))
#endif
static void _pproc_fgea_flag_sign_conv(_PFIELD *pfield, long double cvt,
                                       int *flags, char **a_it);
#endif

/*****************************************************************************/
/* When "MINIMAL" is defined, use "unsigned int" instead of "uintmax_t" as   */
/* the largest integer type printf will handle.  In MINIMAL mode, we don't   */
/* allow format specifiers for "long" or "long long", so we don't need       */
/* uintmax_t in the helper functions.  Using "unsigned int" allows us to     */
/* avoid having the _div function drag in the "long" and "long long"         */
/* division helper functions, which would never be used.                     */
/*****************************************************************************/
#if defined(MINIMAL)
typedef          int  big_t;
typedef unsigned int ubig_t;
#else
typedef  intmax_t  big_t;
typedef uintmax_t ubig_t;
#endif

#if __has_include(<sys/types.h>)
#include <sys/types.h>
#else
/* ssize_t normally comes from unistd.h or sys/types.h */
#if SIZE_MAX == UINT_MAX
typedef int __ssize_t;
#elif SIZE_MAX == ULONG_MAX
typedef long __ssize_t;
#else
#error "How big is ssize_t?"
#endif
#endif

/* uptrdiff_t doesn't really exist; users would normally use size_t */
#if PTRDIFF_MAX == INT_MAX
typedef unsigned int __uptrdiff_t;
#elif PTRDIFF_MAX == LONG_MAX
typedef unsigned long __uptrdiff_t;
#else
#error "How big is uptrdiff_t?"
#endif

static int _pproc_diouxp(_PFIELD *pfield, int *minus_flag, char **a_it,
                         va_list *_ap);
static ubig_t _getarg_diouxp(_PFIELD *pfield, va_list *_ap);
static int _ltostr(ubig_t cvt, int base, char conv, char **a_it);
static ubig_t _div(ubig_t cvt, int base);

#if defined(VECTOR_PRINTF_SUPPORT)
static void _pproc_vn(_PFIELD *pfield, char **it, va_list *_ap);
static  int _pproc_v_dioux(_PFIELD *pfield, int* flags, int *minus_flag,
                           char **a_it, va_list *_ap,
                           void * __restrict _op, int (*_outc)(char, void *),
                           int (*_outs)(char *, void *, int),
                           unsigned int _ap_orig);
static  int _pproc_v_dioux_elem(_PFIELD *pfield, ulonglong elem,
                                int *minus_flag, char **a_it);
static  int _pproc_v_fgea_elem(_PFIELD *pfield, double cvt,
                               int *flags, char **a_it);
static  int _pproc_v_fgea(_PFIELD *pfield, int *flags, int* minus_flag,
                          char**a_it, va_list *_ap,
                          void * __restrict _op, int (*_outc)(char, void *),
                          int (*_outs)(char *, void *, int),
                          unsigned int _ap_orig);
static  int _pproc_v_elem_prelude(_PFIELD *pfield, char **a_it, int *flags,
                                  int *minus_flag, int last_len_added);
static  int _process_v(char *fld, _PFIELD *pfield, va_list *_ap,
                       void * __restrict _op, int (*_outc)(char, void *),
                       int (*_outs)(char *, void *, int),
                       unsigned int _ap_orig);
__attribute__((always_inline))
static  int _write_comma(int* total_length, void * __restrict _op,
                         int (*_outc)(char, void *));
__attribute__((always_inline))
static  int _write_v_elem(int *total_length, char **a_it, int elem_length,
                          void * __restrict _op,
                          int (*_outs)(char *, void *, int));
#endif

#if 0
/* Debug Helper Functions */
/*****************************************************************************/
/* Debug va_list by dumping out the stack up to a certain dump_level         */
/* with increments of dump_step addresses                                    */
/* Usage: _ap_dump(_ap, 6, 4)                                                */
/*****************************************************************************/
static void _ap_dump(va_list* _ap, signed char dump_level,
                     signed char dump_step)
{
    signed char i, step = 0;
    signed char dl_start = -1 * dump_level;
    char* ap_step;
    int* ap_step_val;
    step = dump_level*dump_step;

    Log_print3(Diags_USER6, "[-%d <- %p -> +%d] =>\n", step, (char*)*_ap, step);

    for(i=dl_start; i<=dump_level; i++)
    {
        step = i*dump_step;
        ap_step = ((char*)*_ap) + step;
        ap_step_val = (int*) ap_step;
        Log_print3(Diags_USER6, "[%3d][_ap: %p]: 0x%x\n",
                                step,
                                ap_step,
                                *ap_step_val);

    }
}
#endif
/*****************************************************************************/
/* _PRINTFI -  Perform the main printf routine                               */
/*                                                                           */
/*    This function processes the format string.  It copies the format       */
/*    string into the result string until a '%' is encountered, where any    */
/*    flags, the field width, the precision, and the type of conversion are  */
/*    read in, stored in a structure called PFIELD, and passed to _SETFIELD, */
/*    where the actual conversion is processed.  This function returns       */
/*    the number of characters output.                                       */
/*                                                                           */
/*    For multi-threaded applications, when this function is called on       */
/*    behalf of fprintf, printf, vfprintf, or vprintf, the call site should  */
/*    be enclosed in a critical section that guarantees single-threaded      */
/*    access to the file stream.                                             */
/*                                                                           */
/*****************************************************************************/
_CODE_ACCESS int _PRINTFI(char ** __restrict _format, va_list _ap,
                          void * __restrict _op,
                          int (*_outc)(char, void *),
			  int (*_outs)(char *, void *, int))
{
   /*------------------------------------------------------------------------*/
   /* Local Variables                                                        */
   /*                                                                        */
   /*    *end     -  A pointer to the end of the format string               */
   /*    *pfield  -  A pointer to a structure _PFIELD, which stores all of   */
   /*                flags and parameters needed to perform a conversion.    */
   /*------------------------------------------------------------------------*/
   char     *end           =  *_format + strlen(*_format);
   int      count          =  0;
   unsigned int _ap_orig   = (unsigned int)_ap;

#if defined(VECTOR_PRINTF_SUPPORT)
   /**************************************************************************/
   /*  CODEGEN-4880: WORKAROUND                                              */
   /**************************************************************************/
   _PFIELD  pfield             =
   {
        .arguc2                = (uchar2)(0,0),
        .arguc2_leftover       = 0,
        .last_v_dioux_arg_size = 0
   };
   /**************************************************************************/
   /*  CODEGEN-4880: WORKAROUND                                              */
   /**************************************************************************/
#else
   _PFIELD  pfield;
#endif
   /*------------------------------------------------------------------------*/
   /* Iterate through the format string until the end of it is reached.      */
   /*------------------------------------------------------------------------*/
   while (*_format < end)
   {
      /*---------------------------------------------------------------------*/
      /* Initialize PFIELD.                                                  */
      /*---------------------------------------------------------------------*/
      pfield.flags     = 0;
      pfield.conv      = 0;
#if !defined(MINIMAL)
      pfield.fwidth    = 0;
      pfield.precision = -1;
#if defined(VECTOR_PRINTF_SUPPORT)
      pfield.vn        = -1;
#endif
#endif

      /*---------------------------------------------------------------------*/
      /* Copy the format string directly to the target string until a '%'    */
      /* is encountered.                                                     */
      /*---------------------------------------------------------------------*/
      for (; **_format != '%' && **_format != '\0';
          _outc(*((*_format)++), _op), count++);

      /*---------------------------------------------------------------------*/
      /* If the end of the format string has been reached, break out of the  */
      /* while loop.                                                         */
      /*---------------------------------------------------------------------*/
      if (! (**_format)) break;

      (*_format)++;                   /* Skip to the character after the '%' */

#if !defined(MINIMAL)
      /*---------------------------------------------------------------------*/
      /* Process the flags immediately after the '%'.                        */
      /*---------------------------------------------------------------------*/
      _pproc_fflags(&pfield, _format);

      /*---------------------------------------------------------------------*/
      /* Convert the field width and precision into numbers.                 */
      /*---------------------------------------------------------------------*/
      _pproc_fwp(&pfield, _format, &_ap);

#if defined(VECTOR_PRINTF_SUPPORT)
      /*---------------------------------------------------------------------*/
      /* Process vector specifier v and length n                             */
      /*---------------------------------------------------------------------*/
      _pproc_vn(&pfield, _format, &_ap);
#endif

      /*---------------------------------------------------------------------*/
      /* If the h, l, or L flag was specified, set the corresponding flag    */
      /* in pfield.                                                          */
      /*---------------------------------------------------------------------*/
      switch (**_format)
      {
#if !defined(NOFLOAT)
	  case 'L': _SET(&pfield, _MFLD); (*_format)++; break;
#endif
	  case 'h':
	  {
	      (*_format)++;
	      if (**_format == 'h') { _SET(&pfield, _MFHH); (*_format)++; }
#if defined(VECTOR_PRINTF_SUPPORT)
          else if (**_format == 'l') { _SET(&pfield, _MFHL); (*_format)++; }
#endif
	      else _SET(&pfield, _MFH);
	      break;
	  }
	  case 'l':
	  {
	      (*_format)++;
	      if (**_format == 'l') { _SET(&pfield, _MFLL); (*_format)++; }
	      else _SET(&pfield, _MFL);
              break;
	  }
	  case 'j': _SET(&pfield, _MFJ); (*_format)++;  break;
	  case 'z': _SET(&pfield, _MFZ); (*_format)++;  break;
	  case 't': _SET(&pfield, _MFT); (*_format)++;  break;
#ifdef __TI_INT40_T__
          case 'I':
          {
             if ((*(*_format+1)) == '4' && (*(*_format+2)) == '0')
             { _SET(&pfield, _MFI40); (*_format)+=3; }
             break;
          }
#endif
      }
#endif

      /*---------------------------------------------------------------------*/
      /* Set the conversion character in pfield.                             */
      /*---------------------------------------------------------------------*/
      pfield.conv = *((*_format)++);

#if !defined(MINIMAL)
      /*---------------------------------------------------------------------*/
      /* If 'n' is the conversion specifier, process it in this function,    */
      /* since it is the only one that makes no conversions.  It just stores */
      /* the number of characters printed so far into the next argument.     */
      /* Otherwise, call _SETFIELD which performs the conversion.            */
      /*---------------------------------------------------------------------*/
      if (pfield.conv == 'n')
         switch (pfield.flags & (_MFLL | _MFL | _MFH | _MFHH | _MFI40 | _MFJ | _MFZ | _MFT))
         {
#ifdef LLONG_MAX
                           /* The 'll' flag was specified */
            case _MFLL  :  *(va_arg(_ap, long*)) = (long long)count;
                           break;
#endif
#ifdef __TI_INT40_T__
                           /* The 'I40' flag was specified */
            case _MFI40 :  *(va_arg(_ap, __int40_t *)) = (__int40_t)count;
                           break;
#endif
                           /* The 'l' flag was specified */
            case _MFL   :  *(va_arg(_ap, long*)) = (long)count;
                           break;

                           /* The 'h' flag was specified */
            case _MFH   :  *(va_arg(_ap, short*)) = (short)count;
                           break;

                           /* The 'hh' flag was specified */
            case _MFHH  :  *(va_arg(_ap, char*)) = (char)count;
                           break;

                           /* The 'j' flag was specified */
            case _MFJ   :  *(va_arg(_ap, intmax_t*)) = (intmax_t)count;
                           break;

                           /* The 'z' flag was specified */
            case _MFZ   :  *(va_arg(_ap, __ssize_t*)) = (__ssize_t)count;
                           break;

                           /* The 't' flag was specified */
            case _MFT   :  *(va_arg(_ap, ptrdiff_t*)) = (ptrdiff_t)count;
                           break;

            default     :  *(va_arg(_ap, int*)) = (int)count;
                           break;

         }
      else
#endif
      if (pfield.conv == 's')
      {
#if !defined(MINIMAL)
         if (pfield.flags & _MFL)
            _pproc_wstr(&pfield, _op, &_ap, &count, _outc);
         else
#endif
            _pproc_str(&pfield, _op, &_ap, &count, _outc);
      }
      else
      {
         /*------------------------------------------------------------------*/
         /* Append the converted string to the result string, and reposition */
         /* its iterator, it2.                                               */
         /*------------------------------------------------------------------*/
         char fld[FORMAT_CONVERSION_BUFSIZE];
         int  length = 0;
#if defined(VECTOR_PRINTF_SUPPORT)
         if (pfield.vn > 0)
         {
            length = _process_v(fld, &pfield, &_ap, _op, _outc, _outs, _ap_orig);
            if (length == EOF)
               count = EOF;
            else
               count += length;
         }
         else
#endif
         length = _setfield(fld, &pfield, &_ap);
#if defined(VECTOR_PRINTF_SUPPORT)
         if (pfield.vn < 0) {
#endif
         int _outs_result = _outs(fld, _op, length);
         if (_outs_result == EOF)
            count = EOF;
         else
            count += _outs_result;
#if defined(VECTOR_PRINTF_SUPPORT)
         }
#endif
      }

      // If count == EOF an error has occured, exit the loop
      if (count == EOF)
         break;
   }
   return count;
}

#if !defined(MINIMAL)
/*****************************************************************************/
/* _PPROC_FFLAGS   -  Process the format flags for a conversion              */
/*                                                                           */
/*    This function takes the flags directly after the '%' and stores them   */
/*    in the _PFIELD structure PFIELD for later reference.                   */
/*                                                                           */
/*****************************************************************************/
static void _pproc_fflags(_PFIELD *pfield, char **it)
{
   /*------------------------------------------------------------------------*/
   /* Local variables                                                        */
   /*------------------------------------------------------------------------*/
   int flags_done = 0;

   /*---------------------------------------------------------------------*/
   /* Read in all of the flags associated with this conversion, and set   */
   /* the corresponding flags in the PFIELD structure.                    */
   /*---------------------------------------------------------------------*/
   while (! flags_done)
      switch (**it)
      {
         case '-' :  _SET(pfield, _PFMINUS);
                     (*it)++;
                     break;

         case '+' :  _SET(pfield, _PFPLUS);
                     (*it)++;
                     break;

         case ' ' :  _SET(pfield, _PFSPACE);
                     (*it)++;
                     break;

         case '#' :  _SET(pfield, _PFPOUND);
                     (*it)++;
                     break;

         case '0' :  _SET(pfield, _PFZERO);
                     (*it)++;
                     break;

         default  :  flags_done = 1;
      }

   return;
}

/*****************************************************************************/
/* _PPROC_FWP   -  Convert the field width and precision from the format     */
/*                 string into numbers.                                      */
/*                                                                           */
/*    This function reads the field and precision out of the format string   */
/*    and converts them into numbers that will be stored in the _PFIELD      */
/*    structure pointed to by PFIELD.  They will be needed for future        */
/*    reference.                                                             */
/*                                                                           */
/*****************************************************************************/
static void _pproc_fwp(_PFIELD *pfield, char **it, va_list *_ap)
{
   char tmpstr[10];
   char *tmpptr;

   /*------------------------------------------------------------------------*/
   /* If a '*' was given for the field width, use the next argument as       */
   /* the field width for the conversion.  Otherwise, copy the following     */
   /* numerical characters into a temporary string, and convert that         */
   /* string into an integer, which will be used for the field width.        */
   /*------------------------------------------------------------------------*/
   if (**it == '*')
   {
      pfield->fwidth = va_arg(*_ap, int);

      /*---------------------------------------------------------------------*/
      /* If the argument given for the field width is negative, treat it as  */
      /* if the '-' flag was used, and the field width was positive.         */
      /*---------------------------------------------------------------------*/
      if (pfield->fwidth < 0)
      {
         pfield->fwidth = -(pfield->fwidth);
         _SET(pfield, _PFMINUS);
      }

      (*it)++;
   }
   else
   {
      /*---------------------------------------------------------------------*/
      /* Initialize the temporary string and iterator that will hold the     */
      /* field width temporarily.                                            */
      /*---------------------------------------------------------------------*/
      tmpptr = tmpstr;
      while ((**it >= '0') && (**it <= '9')) *(tmpptr++) = *((*it)++);
      *tmpptr = '\0';

      if (*tmpstr != '\0') pfield->fwidth = atoi(tmpstr);
   }

   /*------------------------------------------------------------------------*/
   /* If a '.' appears as the next character, process the following          */
   /* characters as a precision.                                             */
   /*------------------------------------------------------------------------*/
   if (**it == '.')
   {
      (*it)++;

      /*---------------------------------------------------------------------*/
      /* If a '*' was given for the precision, use the next argument as      */
      /* the precision for the conversion.  Otherwise, copy the following    */
      /* numerical characters into a temporary string, and convert that      */
      /* string into an integer, which will be used for the precision.       */
      /*---------------------------------------------------------------------*/
      if (**it == '*')
      {
         pfield->precision = va_arg(*_ap, int);
         (*it)++;
      }
      else
      {
         /*------------------------------------------------------------------*/
         /* Initialize the temporary string and iterator that will hold      */
         /* the field width temporarily.                                     */
         /*------------------------------------------------------------------*/
         tmpptr = tmpstr;
         while ((**it >= '0') && (**it <= '9'))  *(tmpptr++) = *((*it)++);
         *tmpptr = '\0';

         if (*tmpstr != '\0') pfield->precision = atoi(tmpstr);
         else                pfield->precision = 0;
      }
   }
   return;
}

#if defined(VECTOR_PRINTF_SUPPORT)
/*****************************************************************************/
/* _PPROC_VN   -  Process the vector format specifier                        */
/*                                                                           */
/*    This function reads the vector specifier 'v' and associated vector     */
/*    length 'n' [2,3,4,8,16] out of the format string and converts the      */
/*    value of n into numbers stored in the _PFIELD->vn for future           */
/*    reference.                                                             */
/*                                                                           */
/*****************************************************************************/
static void _pproc_vn(_PFIELD *pfield, char **it, va_list *_ap)
{
#define TMP_STR_SIZE 2
    char tmpstr[TMP_STR_SIZE];
    char *tmpptr;
    int vn = -1;

    /*-----------------------------------------------------------------------*/
    /* If a 'v' appears as the next character, process the following         */
    /* characters as a vector length with values between 2,3,4,8 or 16.      */
    /*-----------------------------------------------------------------------*/
    if (**it == 'v')
    {
        (*it)++;

        /*-------------------------------------------------------------------*/
        /* Initialize the temporary string and iterator that will hold       */
        /* the vector length temporarily.                                    */
        /* Constrain the temporary string from reading in more than the first*/
        /* TMP_STR_SIZE i.e. 2 digits. If there are more digits which were   */
        /* incorrectly added, skip them.                                     */
        /*-------------------------------------------------------------------*/
        tmpptr = tmpstr;
        while (isdigit(**it) && ((tmpptr - tmpstr) < TMP_STR_SIZE))
        {
            *(tmpptr++) = *((*it)++);
        }
        *tmpptr = '\0';

        /*-------------------------------------------------------------------*/
        /* Check for more digits. Skip them. Since this implies incorrect vn */
        /* specification, fail.                                              */
        /*-------------------------------------------------------------------*/
        if (isdigit(**it))
        {
            while (isdigit(**it)) *it++;
            return;
        }

        /*-------------------------------------------------------------------*/
        /* Check for leading 0. Fail if present.                             */
        /*-------------------------------------------------------------------*/
        if (tmpstr[0] == '0') return;

        vn = atoi(tmpstr);

        switch(vn)
        {
        case 2:
        case 3:
        case 4:
        case 8:
        case 16:
            pfield->vn = vn;
            break;
        }
    }
    return;
}
#endif /* defined(VECTOR_PRINTF_SUPPORT) */
#endif /* !defined(MINIMAL) */

#if !defined(MINIMAL)
/*****************************************************************************/
/* _PPROC_WSTR  -  Processes the string conversion (%ls)                     */
/*                                                                           */
/*    This function places all or a portion of the input string into the     */
/*    the temporary string.  It returns a zero, unless the input string had  */
/*    a length of zero.  In this case, a one is returned.                    */
/*                                                                           */
/*****************************************************************************/
static void _pproc_wstr(_PFIELD *pfield, void *_op, va_list *_ap, int *count,
                        int (*_outc)(char, void *))
{
    /*------------------------------------------------------------------------*/
    /* Local variables                                                        */
    /*------------------------------------------------------------------------*/
    wchar_t* strbuf;
    int   buflen;
    int   i;
    int   len;

    /*------------------------------------------------------------------------*/
    /* Get the next argument.                                                 */
    /*------------------------------------------------------------------------*/
    strbuf = va_arg(*_ap, wchar_t*);

    /*------------------------------------------------------------------------*/
    /* Handle NULL strings.                                                   */
    /*------------------------------------------------------------------------*/
    if (strbuf == NULL)
    {
       _outc('\0', _op);
       return;
    }

    buflen = (pfield->precision >= 0 && pfield->precision < wcslen(strbuf)) ?
    pfield->precision : wcslen(strbuf);
    len = (pfield->fwidth > buflen) ? pfield->fwidth : buflen;
    *count += len;

    if (buflen < len && !_STCHK(pfield, _PFMINUS))
        for (i = 0; i < len - buflen; i++)
            _outc(' ', _op);

    for (i = 0; i < buflen; i++)
    {
       wchar_t wc = *strbuf++;
       if (wc != (char)wc)
       {
          *count = EOF;
          errno = EILSEQ;
          return;
       }

       _outc((char)wc, _op);
    }

    if (buflen < len && _STCHK(pfield, _PFMINUS))
        for (i = 0; i < len - buflen; i++)
            _outc(' ', _op);
}
#endif /* !defined(MINIMAL) */

/*****************************************************************************/
/* _PPROC_STR  -  Processes the string conversion (%s)                       */
/*                                                                           */
/*    This function places all or a portion of the input string into the     */
/*    the temporary string.  It returns a zero, unless the input string had  */
/*    a length of zero.  In this case, a one is returned.                    */
/*                                                                           */
/*****************************************************************************/
static void _pproc_str(_PFIELD *pfield, void *_op, va_list *_ap, int *count,
                       int (*_outc)(char, void *))
{
    /*------------------------------------------------------------------------*/
    /* Local variables                                                        */
    /*------------------------------------------------------------------------*/
    char *strbuf;
    int   buflen;
    int   i;
#if !defined(MINIMAL)
    int   len;
#endif

    /*------------------------------------------------------------------------*/
    /* Get the next argument.                                                 */
    /*------------------------------------------------------------------------*/
    strbuf = va_arg(*_ap, char*);

    /*------------------------------------------------------------------------*/
    /* Handle NULL strings.                                                   */
    /*------------------------------------------------------------------------*/
    if (strbuf == NULL)
    {
       _outc('\0', _op);
       return;
    }

#if !defined(MINIMAL)
    buflen = (pfield->precision >= 0 && pfield->precision < strlen(strbuf)) ?
    pfield->precision : strlen(strbuf);
    len = (pfield->fwidth > buflen) ? pfield->fwidth : buflen;
    *count += len;

    if (buflen < len && !_STCHK(pfield, _PFMINUS))
        for (i = 0; i < len - buflen; i++)
            _outc(' ', _op);

    for (i = 0; i < buflen; i++)
        _outc(*strbuf++, _op);

    if (buflen < len && _STCHK(pfield, _PFMINUS))
        for (i = 0; i < len - buflen; i++)
            _outc(' ', _op);
#else
    buflen = strlen(strbuf);
    *count += buflen;

    for (i = 0; i < buflen; i++)
        _outc(*strbuf++, _op);
#endif
}

/*****************************************************************************/
/* _SETFIELD   -  Performs conversions when the '%' is encountered           */
/*                                                                           */
/*    This function takes pfield, and calls the appropriate processing       */
/*    function for the conversion required in the _PFIELD structure.  It     */
/*    returns the length of the result string and sets fld to point to the   */
/*    result string.                                                         */
/*                                                                           */
/*****************************************************************************/
static int _setfield(char *fld, _PFIELD *pfield, va_list *_ap)
{
   /*------------------------------------------------------------------------*/
   /* Local variable declarations, and a description of their use            */
   /*                                                                        */
   /*    FLD is a temporary string that will hold the conversion.  F_START   */
   /*    will be a pointer to the beginning of the field, and if a           */
   /*    field width was specified, F_END will be a pointer to the end of    */
   /*    the field.  This designated field is located at the beginning of    */
   /*    the string FLD.   A_END is a pointer to the end of the string FLD,  */
   /*    and this is where the primary conversion will take place.  Using    */
   /*    A_IT, an iterator beginning at the end of FLD, the number will be   */
   /*    written one digit at a time starting with the rightmost digit.      */
   /*    Using the pointer WHERE, the number in string form will be moved    */
   /*    to its appropriate place within the field after making adjustments  */
   /*    called for by various flags in the format specification (a minus    */
   /*    sign, leading zeros, etc.).  The string FLD will then be returned.  */
   /*                                                                        */
   /*    MINUS_FLAG is exactly what it says ( = 1 if number is negative).    */
   /*                                                                        */
   /*------------------------------------------------------------------------*/
   char *f_start   =  (char *)fld;
#if !defined(MINIMAL)
   char *f_end     =  f_start + pfield->fwidth;
   int  plus_flag  = 0;
#else
   char *f_end     =  f_start;
#endif
   char *a_end     =  f_start + FORMAT_CONVERSION_BUFSIZE -1;
   char *a_it      =  a_end;
   char *where;
   int  minus_flag = 0;
   int  flags = NO_FLAG;
   char temp_char = 0;
   int  str_terminator = 0;

   /*------------------------------------------------------------------------*/
   /* Initialize the temporary string.  Then, since we are working from      */
   /* right to left, begin with the NULL character.                          */
   /*------------------------------------------------------------------------*/
   memset(fld, ' ', FORMAT_CONVERSION_BUFSIZE);
   *(a_it--) = '\0';

   /*------------------------------------------------------------------------*/
   /* Call the appropriate processing function.                              */
   /*------------------------------------------------------------------------*/
   switch (pfield->conv)
   {
      case 'd' :
#if !defined (MINIMAL)
      case 'i' :
      case 'u' :
      case 'X' :
      case 'p' :
#endif
      case 'o' :
      case 'x' :  _pproc_diouxp(pfield, &minus_flag, &a_it, _ap);
                  break;

#if !defined(NOFLOAT) && !defined(MINIMAL)
      case 'a' :
      case 'A' :
      case 'g' :
      case 'G' :
      case 'e' :
      case 'E' :
      case 'f' :
      case 'F' :  _pproc_fgea(pfield, &flags, &a_it, _ap);
                  break;
#endif

      case 'c' :
                  /*---------------------------------------------------------*/
                  /* If wchar_t is smaller than an int, it will be promoted  */
                  /* to int because this is a variadic function.  If wchar_t */
                  /* is larger than int, grab a wchar_t unit.                */
                  /*---------------------------------------------------------*/
                  if ((sizeof(wchar_t) > sizeof(int)) && pfield->flags & _MFL)
                      temp_char = va_arg(*_ap, wchar_t);
                  else
                      temp_char = va_arg(*_ap, int);

                  /*---------------------------------------------------------*/
                  /* Set str_terminator to 1 if '\0' is seen.                */
                  /*---------------------------------------------------------*/
                  str_terminator = (temp_char == '\0')? 1:0;
                  *(a_it--) = temp_char;
#if !defined(MINIMAL)
                  _UNSET(pfield, _PFPLUS);
#endif
                  break;
      case '%' :  strcpy(f_start, "%");
                  /*---------------------------------------------------------*/
                  /* strcpy() will append terminating null '\0' to the end,  */
                  /* so string length is 1.                                  */
                  /*---------------------------------------------------------*/
	          return 1;
   }

   /*------------------------------------------------------------------------*/
   /* Potentially print a sign indicator, but only for signed conversions.   */
   /* The '+' and ' ' flags are only valid for signed conversions.           */
   /*------------------------------------------------------------------------*/
#if !defined(MINIMAL)
   if (SIGNED_CONV(pfield->conv))
#endif
       switch (flags)
       {
           case MINUS_FLAG: minus_flag = 1;
           case NO_FLAG:
           {
#if !defined(MINIMAL)
               plus_flag = _STCHK(pfield, _PFPLUS);
#endif

               /*------------------------------------------------------------*/
               /* If the number was negative, or the '+' flag was used,      */
               /* insert the sign.                                           */
               /*------------------------------------------------------------*/
               if (minus_flag) *(a_it--) = '-';
#if !defined(MINIMAL)
               else if (plus_flag) *(a_it--) = '+';

               /*------------------------------------------------------------*/
               /* If the number was positive, the '+' flag was not used, and */
               /* the ' ' flag was used, insert a space.                     */
               /*------------------------------------------------------------*/
               if (! minus_flag && ! plus_flag && _STCHK(pfield, _PFSPACE))
                   *(a_it--) = ' ';
#endif
           }
       }

   /*------------------------------------------------------------------------*/
   /* If the '-' flag was used or the resulting string is larger than the    */
   /* field, left justify the result in the array.  Otherwise right-justify  */
   /* it.                                                                    */
   /*------------------------------------------------------------------------*/
#if !defined(MINIMAL)
   where = (_STCHK(pfield, _PFMINUS) ||
           ((a_end - a_it) > pfield->fwidth)) ? f_start :
           (f_end - (a_end - a_it)+1);
#else
   where = (((a_end - a_it) > 0)) ? f_start :
           (f_end - (a_end - a_it)+1);
#endif

   a_it = (char *)memccpy(where, a_it+1, '\0', FORMAT_CONVERSION_BUFSIZE);

   /*------------------------------------------------------------------------*/
   /* In case of (..."%c", '\0'...), the above memccpy() will miss the TRUE  */
   /* '\0', treating it as a terminator. Here we need to add it back.        */
   /*------------------------------------------------------------------------*/
   if (str_terminator)
	*(a_it++) = '\0';

   /*------------------------------------------------------------------------*/
   /* If a resulting left-justified string is smaller than the field width,  */
   /* move the terminating NULL character to the end of the field.           */
   /*------------------------------------------------------------------------*/
   if (a_it <= f_end)
   {
      memset(a_it - 1, ' ', f_end - a_it + 1);
      *f_end = '\0';
   }

#if !defined(MINIMAL)
   /*------------------------------------------------------------------------*/
   /* If the '0' flag was used, and the resulting string is right-justified, */
   /* fill in the leading zeros.                                             */
   /*------------------------------------------------------------------------*/
   if (_STCHK(pfield, _PFZERO))
   {
       /*--------------------------------------------------------------------*/
       /* Skip if no '0' were inserted.  If we didn't skip, the swap would   */
       /* mistakenly clobber the sign or base indicator.                     */
       /*--------------------------------------------------------------------*/
       if (where != f_start)
       {
           memset(f_start, '0', (where - f_start));

           char* swap_ptr = f_start;

           /*----------------------------------------------------------------*/
           /* The sign-indicating flags ('+' and ' ') are only valid for     */
           /* signed conversions, and are ignored for unsigned conversions.  */
           /*----------------------------------------------------------------*/
           if (minus_flag || plus_flag || _STCHK(pfield, _PFSPACE))
           {
               /*------------------------------------------------------------*/
               /* Swap the sign indicator and the first zero                 */
               /*------------------------------------------------------------*/
               *swap_ptr++ = *where;
               *where++ = '0';
           }

           /*----------------------------------------------------------------*/
           /* Those '0's were supposed to go after any indication of sign or */
           /* base.  Move the zeros to the right of such indication.         */
           /*----------------------------------------------------------------*/
           if ((HEX_CONV(pfield->conv) && _STCHK(pfield, _PFPOUND)) ||
               pfield->conv == 'a' || pfield->conv == 'A')
           {
              /*--------------------------------------------------------*/
              /* Swap the 'x' or 'X' and the first zero.                */
              /*--------------------------------------------------------*/
              *(swap_ptr+1) = *(where+1);
              *(where+1) = '0';
           }
       }
   }
   else
#endif
      memset(f_start, ' ', (where - f_start));

   /*------------------------------------------------------------------------*/
   /* Return the length of the result string. Because strlen() depends on    */
   /* '\0' to terminate, in case of (...%c,'\0'...), we need to count one    */
   /* more character for length.                                             */
   /*------------------------------------------------------------------------*/
   return strlen(fld)+str_terminator;
}

#if defined(VECTOR_PRINTF_SUPPORT)
/*****************************************************************************/
/* _WRITE_COMMA - Write a single comma out to the stream and increment the   */
/*                total_length counter if successful                         */
/*                                                                           */
/*  This function returns 0 on success and EOF on failure                    */
/*****************************************************************************/
static  int _write_comma(int* total_length, void * __restrict _op,
                         int (*_outc)(char, void *))
{
    int _outc_result;
    _outc_result = _outc(',', _op);
    if (_outc_result != EOF) *total_length++;
    else                     return _outc_result;

    return 0;
}

/*****************************************************************************/
/* _WRITE_V_ELEM - Write an element string out to the stream and increment   */
/*                 the total_length counter if successful.                   */
/*                                                                           */
/*  This expects *a_it to be pointing to the beginning of the processed      */
/*  element string with length = len. After writing to the stream, *a_it is  */
/*  pointed back to the character before the terminating null character.     */
/*  The element string in *a_it is also reset to blank spaces to make ready  */
/*  for processing the next element.                                         */
/*                                                                           */
/*  This function returns 0 on success and EOF on failure                    */
/*****************************************************************************/
static  int _write_v_elem(int *total_length, char **a_it, int elem_length,
                          void * __restrict _op,
                          int (*_outs)(char *, void *, int))
{
    int _outs_result;
    _outs_result = _outs(*a_it, _op, elem_length);

    if (_outs_result != EOF) *total_length += elem_length;
    else                     return _outs_result;

    memset(*a_it, ' ', elem_length);
    *a_it += elem_length-1;

    return 0;
}

/*****************************************************************************/
/* _PPROC_V_ELEM_PRELUDE -  Processes the flags, width and precision of an   */
/*                          element of a vector type                         */
/*                                                                           */
/*      This function finishes processing an element after it has been       */
/*      written to a_it. It assumes that last_len_added is the length of     */
/*      the element it is processing, and that a_it points to one character  */
/*      before the element starts. It processes the flags, the field width,  */
/*      required padding and resets the flag and minus_flag pointer for the  */
/*      next element.                                                        */
/*      It returns the length of the processed element and sets *a_it to     */
/*      point to the start of the processed element.                         */
/*****************************************************************************/
static int _pproc_v_elem_prelude(_PFIELD *pfield, char **a_it, int *flags,
                                  int *minus_flag, int last_len_added)
{
    char *pad_start         = *a_it;
    char *a_start           = pad_start + 1;
    char *a_end             = pad_start + last_len_added;
    int  element_size       = last_len_added;
    int  plus_flag          = 0;
    int  num_pad_required   = 0;

    /*-----------------------------------------------------------------------*/
    /* Potentially print a sign indicator, but only for signed conversions.  */
    /* The '+' and ' ' flags are only valid for signed conversions.          */
    /*-----------------------------------------------------------------------*/
    if (SIGNED_CONV(pfield->conv))
        switch (*flags)
        {
        case MINUS_FLAG:
            *minus_flag = 1;
        case NO_FLAG:
        {
            plus_flag = _STCHK(pfield, _PFPLUS);

            /*------------------------------------------------------------*/
            /* If the number was negative, or the '+' flag was used,      */
            /* insert the sign.                                           */
            /*------------------------------------------------------------*/
            if (*minus_flag)    {*((*a_it)--) = '-'; element_size++;}
            else if (plus_flag) {*((*a_it)--) = '+'; element_size++;}

            /*------------------------------------------------------------*/
            /* If the number was positive, the '+' flag was not used, and */
            /* the ' ' flag was used, insert a space.                     */
            /*------------------------------------------------------------*/
            if (! *minus_flag && ! plus_flag && _STCHK(pfield, _PFSPACE))
                {*((*a_it)--) = ' '; element_size++;}
        }
        }

    /*-----------------------------------------------------------------------*/
    /* If the current length of this vector element (including +/- sign)     */
    /* is less than the field width, then padding may be required.           */
    /* Calculate the number of characters to pad in front of the element     */
    /*-----------------------------------------------------------------------*/
    if ((a_end - *a_it) < pfield->fwidth)
    {
        num_pad_required = pfield->fwidth - (a_end - *a_it);
        pad_start = *a_it - num_pad_required + 1;
        a_start = *a_it + 1;
    }

    /*-----------------------------------------------------------------------*/
    /* If the '0' flag was used, and the resulting string is right-justified,*/
    /* fill in the leading zeros.                                            */
    /*-----------------------------------------------------------------------*/
    if (_STCHK(pfield, _PFZERO))
    {
        /*-------------------------------------------------------------------*/
        /* Skip if no '0' were inserted.  If we didn't skip, the swap would  */
        /* mistakenly clobber the sign or base indicator.                    */
        /*-------------------------------------------------------------------*/
        if (num_pad_required > 0 && !_STCHK(pfield, _PFMINUS))
        {
            memset(pad_start, '0', num_pad_required);

            char* swap_ptr = pad_start;

            /*---------------------------------------------------------------*/
            /* The sign-indicating flags ('+' and ' ') are only valid for    */
            /* signed conversions, and are ignored for unsigned conversions. */
            /*---------------------------------------------------------------*/
            if (*minus_flag || plus_flag || _STCHK(pfield, _PFSPACE))
            {
                /*-----------------------------------------------------------*/
                /* Swap the sign indicator and the first zero                */
                /*-----------------------------------------------------------*/
                *swap_ptr++ = *a_start;
                *a_start++ = '0';
            }

            /*---------------------------------------------------------------*/
            /* Those '0's were supposed to go after any indication of sign or*/
            /* base.  Move the zeros to the right of such indication.        */
            /*---------------------------------------------------------------*/
            if ((HEX_CONV(pfield->conv) && _STCHK(pfield, _PFPOUND)) ||
                    pfield->conv == 'a' || pfield->conv == 'A')
            {
                /*--------------------------------------------------------*/
                /* Swap the 'x' or 'X' and the first zero.                */
                /*--------------------------------------------------------*/
                *(swap_ptr+1) = *(a_start+1);
                *(a_start+1) = '0';
            }
        }
    }
    else if (num_pad_required > 0)
        memset(pad_start, ' ', num_pad_required);

    /*-----------------------------------------------------------------------*/
    /* If the '-' flag was used and element size is less than fwidth,        */
    /* left justify the element.                                             */
    /*-----------------------------------------------------------------------*/
    if(_STCHK(pfield, _PFMINUS) && num_pad_required > 0)
    {
        *a_it = (char *)memmove(pad_start, *a_it+1, element_size);
        memset(pad_start+element_size, ' ', num_pad_required);
    }

    /*-----------------------------------------------------------------------*/
    /* Point a_it to first pad space or element start                        */
    /*-----------------------------------------------------------------------*/
    if(num_pad_required) *a_it = pad_start;
    else                 *a_it += 1;

    /*-----------------------------------------------------------------------*/
    /* Reset flags and minus_flag for next element                           */
    /*-----------------------------------------------------------------------*/
    *minus_flag = 0;
    *flags = NO_FLAG;

    return strlen(*a_it);
}

/*****************************************************************************/
/* _PROCESS_V    -  Performs vector conversions when the '%' is encountered  */
/*                                                                           */
/*    This function takes pfield, and calls the appropriate processing       */
/*    function for the conversion required in the _PFIELD structure.  It     */
/*    writes the result string one vector element at a time to _outs with    */
/*    intermediate use of _outc for writing comma separators. Once writing   */
/*    is finished it returns the length of the result string.                */
/*                                                                           */
/*    Note, at any point of writing to _outc or _outs, if EOF is returned,   */
/*    this function stops processing and returns EOF.                        */
/*****************************************************************************/
static  int _process_v(char *fld, _PFIELD *pfield, va_list *_ap,
                       void * __restrict _op, int (*_outc)(char, void *),
			           int (*_outs)(char *, void *, int),
                       unsigned int _ap_orig)
{
    /*-----------------------------------------------------------------------*/
    /* Local variable declarations, and a description of their use           */
    /*                                                                       */
    /*  FLD is a temporary string that will hold the conversion for one      */
    /*  vector element at a time. Using A_IT, an iterator beginning at the   */
    /*  end of FLD, the vector number will be processed one element at a     */
    /*  time starting with the leftmost element.                             */
    /*  Note that individual elements are processed for flags, width,        */
    /*  precision and justification within the _pproc functions.             */
    /*                                                                       */
    /*  MINUS_FLAG is exactly what it says ( = 1 if number is negative).     */
    /*                                                                       */
    /*  Vector types for conversions 'c', 's', 'p', 'n' and '%' are not      */
    /*  handled.                                                             */
    /*-----------------------------------------------------------------------*/
    char *a_it      =  fld + FORMAT_CONVERSION_BUFSIZE -1;
    int  minus_flag = 0;
    int  flags      = NO_FLAG;
    int length      = 0;

    /*-----------------------------------------------------------------------*/
    /* Initialize the temporary string.  Then, since we are working from     */
    /* right to left, begin with the NULL character.                         */
    /*-----------------------------------------------------------------------*/
    memset(fld, ' ', FORMAT_CONVERSION_BUFSIZE);
    *(a_it--) = '\0';
    /*-----------------------------------------------------------------------*/
    /* Call the appropriate processing function.                             */
    /*-----------------------------------------------------------------------*/
    switch (pfield->conv)
    {
    case 'd' :
    case 'i' :
    case 'u' :
    case 'X' :
    case 'o' :
    case 'x' :
        length = _pproc_v_dioux(pfield, &flags, &minus_flag, &a_it, _ap,
                                _op, _outc, _outs, _ap_orig);
        break;

#if !defined(NOFLOAT)
    case 'a' :
    case 'A' :
    case 'g' :
    case 'G' :
    case 'e' :
    case 'E' :
    case 'f' :
    case 'F' :
        length = _pproc_v_fgea(pfield, &flags, &minus_flag, &a_it, _ap,
                                _op, _outc, _outs, _ap_orig);
        break;
#endif
    default:
        /* No conversion for 'c', 's', 'p', 'n' and '%' */
        break;
    }

    /*-----------------------------------------------------------------------*/
    /* Return the length of the result string.                               */
    /*-----------------------------------------------------------------------*/
    return length;
}
#endif /* defined(VECTOR_PRINTF_SUPPORT) */

#if !defined(NOFLOAT) && !defined(MINIMAL)
/*****************************************************************************/
/* FCVT - Convert and round to the requested number of fractional digits.    */
/*                                                                           */
/*    Converts "value" into buffer "out" as a digit string representation    */
/*    rounded to the requested number of fractional digits, but without      */
/*    sign, decimal point, or exponent.  This string might not contain all   */
/*    of the '0' digits required.                                            */
/*                                                                           */
/*    "fdigit" is the requested number of fractional digits to appear after  */
/*    the decimal point.  Upon input, fdigit is a non-negative integer.      */
/*                                                                           */
/*    Returns a pointer to the most significant (left-most) digit of the     */
/*    converted string, which might not be the start of buffer "out."        */
/*    For %e, this string will have exactly fdigit+1 digits.                 */
/*    For %f, this string may have 0 to an arbitrary number of digits,       */
/*    depending on its magnitude (see below).                                */
/*                                                                           */
/*    "decpt" is an output parameter indicating where the decimal point      */
/*    should appear, in terms of digits from the start of the returned       */
/*    string.                                                                */
/*    For %e, decpt will be 1, unless rounding carried, in which case it's 2 */
/*    For %f, decpt can range from [LDBL_MIN_10_EXP, LDBL_MAX_10_EXP+1]      */
/*                                                                           */
/*    When called from pconv_f, value is in [0, LDBL_MAX]                    */
/*    When called from pconv_e, value is in [0, 10)                          */
/*                                                                           */
/*****************************************************************************/
static char *fcvt(char *out, long double value, int fdigit, int *decpt)
{
    int   scale = 0;
    char *pos   = out + 1;
    int   ip_len;

    /*-----------------------------------------------------------------------*/
    /* Reserve one extra digit in case rounding carries into a new highest   */
    /* position; for instance, rounding 9 up to 10.  We start out with a 0   */
    /* in that position to catch the carry.  We also compute one extra       */
    /* digit, which will be the rounding digit.                              */
    /*-----------------------------------------------------------------------*/
    out[0] = '0';
    fdigit++;

    /*-----------------------------------------------------------------------*/
    /* Pre-scale non-zero values to [1, LONG_MAX] so we can use __TI_ltoa to */
    /* snag the first few digits of the value.  Keep track of the scale.     */
    /*-----------------------------------------------------------------------*/
    /* For %f, the scale influences how many digits will be in the returned  */
    /* string.  If no scaling occured, the output string will be the length  */
    /* of the decimal representation of the whole number part plus           */
    /* precision characters.  For instance, for values [0, 10), the length   */
    /* will be precion+1; for values [10, 100), the length will be           */
    /* precision+2.  If scaling did occur, add scale to the length of the    */
    /* string, not to go below zero (very small values will have             */
    /* zero-length return string).  The longest string will be for LDBL_MAX  */
    /* (309 + precision digits)                                              */
    /*-----------------------------------------------------------------------*/
    /* For %e, scale will always be 0.                                       */
    /*-----------------------------------------------------------------------*/
    if (value < 0) value = -value;
    while (value > LONG_MAX)   { value /= 10; scale++; }
    while (value && value < 1) { value *= 10; scale--; }

    /*-----------------------------------------------------------------------*/
    /* Convert the integral part of the scaled number.                       */
    /*-----------------------------------------------------------------------*/
    pos += ip_len = __TI_ltoa((long)value, pos);

    /*-----------------------------------------------------------------------*/
    /* Set *decpt to indicate where the decimal point would go in the digit  */
    /* sequence, in terms of digits from the start of the returned string.   */
    /* If it weren't for the prescaling, it would be just "ip_len."  If we   */
    /* pre-scaled up, scale will be negative and we'll move the decimal      */
    /* point to the left, and vice versa.  Keep in mind that the string we   */
    /* return might not be exactly where "out" is at this point.             */
    /*-----------------------------------------------------------------------*/
    /* When *dpt < 0, the calling function needs to insert leading zeroes.   */
    /* When *dpt > strlen(return value), the calling function needs to       */
    /* insert trailing zeroes.                                               */
    /*-----------------------------------------------------------------------*/
    *decpt = scale + ip_len;

    /*-----------------------------------------------------------------------*/
    /* We want exactly fdigit digits after the decimal point.  If we         */
    /* pre-scaled up, scale will be negative and we've already got a few     */
    /* fractional digits.  If we pre-scaled down, scale will be positive     */
    /* and we'll need extra digits before we start producing fractional      */
    /* digits.                                                               */
    /*-----------------------------------------------------------------------*/
    fdigit += scale;

    /*-----------------------------------------------------------------------*/
    /* Convert the fractional part of the scaled number until we have        */
    /* precision+1 fractional digits.                                        */
    /*-----------------------------------------------------------------------*/
    if (fdigit > 0)
    {
        do
        {
            value -= (long)value;
            *pos++ = (int)(value *= 10.0) + '0';
        }
        while (--fdigit);
    }

    /*-----------------------------------------------------------------------*/
    /* At this point, "pos" points one past the last digit written, which is */
    /* an extra digit for rounding.                                          */
    /*-----------------------------------------------------------------------*/

    /*-----------------------------------------------------------------------*/
    /* Perform rounding.  Note that this does not perform "round-to-even" as */
    /* it should.  Consider printf("%.1g", 2.5), which should be "2"         */
    /*-----------------------------------------------------------------------*/
    if (*--pos >= '5')
    {
	char *ptr = pos;
	while ((*--ptr += 1) > '9') *ptr = '0';

        /*-------------------------------------------------------------------*/
        /* ptr now points one before the last digit affected by the          */
        /* rounding.  If ptr==out, rounding carried all the way into a new   */
        /* most significant digit.  In that case, we'll need to adjust       */
        /* decpt, and return out, which points to the new most significant   */
        /* digit.  Unlike %g, we will not delete the least significant       */
        /* digit in this case (which is a 0).                                */
        /*-------------------------------------------------------------------*/
	if (ptr == out) { *pos = 0; *decpt += 1; return out; }
    }

    /*-----------------------------------------------------------------------*/
    /* If rounding did not carry all the way into a new most significant     */
    /* digit, NULL terminate the string and return out+1, which points to    */
    /* the most significant digit.                                           */
    /*-----------------------------------------------------------------------*/
    *pos = 0;
    return out + 1;
}


/*****************************************************************************/
/* ECVT - Convert and round to the requested number of significant digits.   */
/*                                                                           */
/*    Converts "value" into buffer "out" as a digit string representation    */
/*    rounded to the requested number of significant digits, but without     */
/*    sign, decimal point, or exponent.  This string might not contain all   */
/*    of the '0' digits required.                                            */
/*                                                                           */
/*    "sigdigit" is the requested number of significant digits to appear.    */
/*    Upon input, sigdigit is a non-negative integer.                        */
/*                                                                           */
/*    Returns a pointer to the most significant (left-most) digit of the     */
/*    converted string, which might not be the start of buffer "out."        */
/*    For %g, this string will have exactly sigdigit digits.                 */
/*                                                                           */
/*    "decpt" is an output parameter indicating where the decimal point      */
/*    should appear, in terms of digits from the start of the returned       */
/*    string.                                                                */
/*    For %g, decpt can range from [LDBL_MIN_10_EXP, LDBL_MAX_10_EXP+1]      */
/*                                                                           */
/*    When called from pconv_g, value is in [0, LDBL_MAX]                    */
/*    This function is not called from pconv_e, despite the name.            */
/*                                                                           */
/*****************************************************************************/
static char *ecvt(char *out, long double value, int sigdigit, int *decpt)
{
    int   scale = 0;
    char *pos    = out + 1;
    int   ip_len;

    /*-----------------------------------------------------------------------*/
    /* Reserve one extra digit in case rounding carries into a new highest   */
    /* position; for instance, rounding 9 up to 10.  We start out with a 0   */
    /* in that position to catch the carry.  We also compute one extra       */
    /* digit, which will be the rounding digit.                              */
    /*-----------------------------------------------------------------------*/
    out[0] = '0';
    sigdigit++;

    /*-----------------------------------------------------------------------*/
    /* Pre-scale non-zero values to [1, LONG_MAX] so we can use __TI_ltoa to */
    /* snag the first few digits of the value.  Keep track of the scale.     */
    /*-----------------------------------------------------------------------*/
    if (value < 0) value = -value;
    while (value > LONG_MAX)   { value /= 10; scale++; }
    while (value && value < 1) { value *= 10; scale--; }

    /*-----------------------------------------------------------------------*/
    /* Convert the integral part of the scaled number.                       */
    /*-----------------------------------------------------------------------*/
    pos    += ip_len = __TI_ltoa((long)value, pos);

    /*-----------------------------------------------------------------------*/
    /* Set *decpt to indicate where the decimal point would go in the digit  */
    /* sequence, in terms of digits from the start of the string.  If it     */
    /* weren't for the prescaling, it would be just "ip_len."  If we         */
    /* pre-scaled up, scale will be negative and we'll move the decimal      */
    /* point to the left, and vice versa.                                    */
    /*-----------------------------------------------------------------------*/
    *decpt  = scale + ip_len;

    /*-----------------------------------------------------------------------*/
    /* Convert the fractional part of the scaled number.                     */
    /*-----------------------------------------------------------------------*/
    if (ip_len >= sigdigit)
    {
        /*-------------------------------------------------------------------*/
        /* If the integer part of the scaled value alone had enough digits   */
        /* (precision + 1 for rounding), set pos one past the last digit.    */
        /*-------------------------------------------------------------------*/
	pos = out + sigdigit + 1;
    }
    else
    {
        sigdigit -= ip_len;

        /*-------------------------------------------------------------------*/
        /* Get as many more digits as are needed                             */
        /*-------------------------------------------------------------------*/
        do
        {
            value -= (long)value;
            *pos++ = (int)(value *= 10.0) + '0';
        }
        while (--sigdigit);
    }

    /*-----------------------------------------------------------------------*/
    /* At this point, "pos" points one past the last digit written, which is */
    /* an extra digit for rounding.                                          */
    /*-----------------------------------------------------------------------*/

    /*-----------------------------------------------------------------------*/
    /* Perform rounding.  Note that this does not perform "round-to-even" as */
    /* it should.  Consider printf("%.1g", 2.5), which should be "2"         */
    /*-----------------------------------------------------------------------*/
    if (*--pos >= '5')
    {
	char *ptr = pos;
	while ((*--ptr += 1) > '9') *ptr = '0';

        /*-------------------------------------------------------------------*/
        /* ptr now points one before the last digit affected by the          */
        /* rounding.  If ptr==out, rounding carried all the way into a new   */
        /* most significant digit.  In that case, chop off the least         */
        /* significant digit, adjust *decpt, and return out, which points    */
        /* to the new most significant digit.                                */
        /*-------------------------------------------------------------------*/
	if (ptr == out) { *--pos = 0; *decpt += 1; return out; }
    }

    /*-----------------------------------------------------------------------*/
    /* If rounding did not carry all the way into a new most significant     */
    /* digit, NULL terminate the string and return out+1, which points to    */
    /* the most significant digit.                                           */
    /*-----------------------------------------------------------------------*/
    *pos = 0;
    return out + 1;
}

static int val_hex(char *pos, int caps)
{
    char  *bnum = "0123456789abcdef";
    if (caps) bnum = "0123456789ABCDEF";
    return strchr(bnum, *pos) - bnum;
}

static int incr_hex(char *pos, int caps)
{
    int val;
    char  *bnum = "0123456789abcdef";
    if (caps) bnum = "0123456789ABCDEF";
    val = strchr(bnum, *pos) - bnum + 1;
    if (val > 0xf) *pos = '0';
    else *pos = bnum[val];
    return val;
}

/*****************************************************************************/
/* ACVT - Convert and round to the requested number of fractional HEX digits */
/*                                                                           */
/*    Converts "value" into buffer "out" as a digit string representation    */
/*    rounded to the requested number of fractional digits, but without      */
/*    sign, radix point, or exponent.  This string might not contain all     */
/*    of the '0' digits required.                                            */
/*                                                                           */
/*    "fdigit" is the requested number of fractional digits to appear after  */
/*    the radix point.  Upon input, fdigit may be -1, indicating that        */
/*    no rounding should occur, and enough digits should be printed for      */
/*    exact precision.                                                       */
/*                                                                           */
/*    Returns a pointer to the most significant (left-most) digit of the     */
/*    converted string, which might not be the start of buffer "out."        */
/*    For %a, if a precision was specified, this string will be precision+1  */
/*    hex digits.  If no precision was specified, this string will be        */
/*    exactly long enough to represent the exact value, at most              */
/*    ceil(DBL_MANT_DIG/4.0) [ 14 for 64-bit IEEE ]                          */
/*                                                                           */
/*    "decpt" is an output parameter indicating where the radix point        */
/*    should appear, in terms of digits from the start of the returned       */
/*    string.                                                                */
/*                                                                           */
/*    This function is called by both pconv_f and pconv_e                    */
/*                                                                           */
/*****************************************************************************/
static char *acvt(char *out, long double value, int fdigit,
                  int *decpt, int conv)
{
    char *pos    = out + 1;
    int   ip_len, exp;
    int   full_prec = fdigit == -1;

    const char *bnum = "0123456789abcdef";
    if (conv == 'A') bnum = "0123456789ABCDEF";

    /*-----------------------------------------------------------------------*/
    /* Reserve one extra digit in case rounding carries into a new highest   */
    /* position; for instance, rounding 9 up to 10.  We start out with a 0   */
    /* in that position so we can increment it if we need to round up.       */
    /*-----------------------------------------------------------------------*/
    out[0] = '0';

    /*-----------------------------------------------------------------------*/
    /* Pre-scale non-zero values to [1, 2) so we can use __TI_ltoa to snag   */
    /* the first few digits of the value, and so that the first digit is     */
    /* always 1                                                              */
    /*-----------------------------------------------------------------------*/
    if (value < 0) value = -value;
    if (value) { frexpl(value, &exp); value = ldexpl(value, -(exp-1)); }

    /*-----------------------------------------------------------------------*/
    /* Convert the integral part of the scaled number.                       */
    /*-----------------------------------------------------------------------*/
    pos += ip_len = __TI_ltoa((long)value, pos);
    value -= (long)value;

    /*-----------------------------------------------------------------------*/
    /* Set *decpt to indicate where the radix point would go in the digit    */
    /* sequence, in terms of digits from the start of the string.  This is   */
    /* the value of ip_len, which is always 1 because of the pre-scaling.    */
    /*-----------------------------------------------------------------------*/
    *decpt = ip_len;

    if (full_prec)
    {
        /*-------------------------------------------------------------------*/
        /* Print the exact value, using as many digits as necessary.         */
        /*-------------------------------------------------------------------*/
        while (value)
        {
            int ival = (int)(value *= 16);
            *pos++ = bnum[ival];
            value -= ival;
        }
    }
    else
    {
        /*-------------------------------------------------------------------*/
        /* Print to the specified precision, with rounding.                  */
        /*-------------------------------------------------------------------*/

        /*-------------------------------------------------------------------*/
        /* Compute one extra digit, which will be the rounding digit.        */
        /*-------------------------------------------------------------------*/
        fdigit++;

        /*-------------------------------------------------------------------*/
        /* Get as many more digits as are needed                             */
        /*-------------------------------------------------------------------*/
        while (fdigit--)
        {
            int ival = (int)(value *= 16);
            *pos++ = bnum[ival];
            value -= ival;
        }

        /*-------------------------------------------------------------------*/
        /* At this point, "pos" points one past the last digit written,      */
        /* which is an extra digit for rounding.                             */
        /*-------------------------------------------------------------------*/

        /*-------------------------------------------------------------------*/
        /* Perform rounding.  Note that this does not perform                */
        /* "round-to-even" as it should.                                     */
        /*-------------------------------------------------------------------*/
        if (val_hex(--pos, conv == 'A') >= 0x8)
        {
            char *ptr = pos;
            while (incr_hex(--ptr, conv == 'A') > 0xf) *ptr = 0;
        }
    }

    /*-----------------------------------------------------------------------*/
    /* NULL terminate the string and return out+1, which points to the most  */
    /* significant digit.                                                    */
    /*-----------------------------------------------------------------------*/
    *pos = 0;
    return out + 1;
}

/*****************************************************************************/
/* _PPROC_FGEA_FLAG_SIGN_CONV      -  Perform flag check, sign check and     */
/*                                    conversion for f, g, G, e, E, a, and A */
/*                                                                           */
/*    This function takes the structure PFIELD, a floating-point number cvt  */
/*    and processes its flags, sign conversion. The result in stored in the  */
/*    string pointed to by *A_IT.                                            */
/*                                                                           */
/*    This function is common to both scalar and vector FGEA processing      */
/*****************************************************************************/
static void _pproc_fgea_flag_sign_conv(_PFIELD *pfield, long double cvt,
                                       int *flags, char **a_it)
{
    /*-----------------------------------------------------------------------*/
    /* If the value is a special IEEE representation, return a flag          */
    /*-----------------------------------------------------------------------*/
#if defined(isnan) && defined(isinf)
    int is_cap = pfield->conv == toupper(pfield->conv);

    if (isnan(cvt))
    {
        *((*a_it)--) = is_cap ? 'N' : 'n';
        *((*a_it)--) = is_cap ? 'A' : 'a';
        *((*a_it)--) = is_cap ? 'N' : 'n';
        *flags = ISNAN_FLAG;
        return;
    }
    else if (isinf(cvt))
    {
        if (cvt < 0)
        {
            *((*a_it)--) = is_cap ? 'F' : 'f';
            *((*a_it)--) = is_cap ? 'N' : 'n';
            *((*a_it)--) = is_cap ? 'I' : 'i';
            *((*a_it)--) = '-';
            *flags = ISNINF_FLAG;
            return;
        }
        else
        {
            *((*a_it)--) = is_cap ? 'F' : 'f';
            *((*a_it)--) = is_cap ? 'N' : 'n';
            *((*a_it)--) = is_cap ? 'I' : 'i';
            *flags = ISPINF_FLAG;
            return;
        }
    }
#endif
    /*-----------------------------------------------------------------------*/
    /* If CVT is negative, set the MINUS_FLAG and reverse the sign of CVT.   */
    /*-----------------------------------------------------------------------*/
    if (signbit(cvt)) {
        *flags = MINUS_FLAG;
        cvt = -cvt;
    }

    /*-----------------------------------------------------------------------*/
    /* Call the proper conversion function                                   */
    /*-----------------------------------------------------------------------*/
    switch (pfield->conv)
    {
    case 'f' :
    case 'F' :
        _pconv_f(cvt, pfield, a_it);
        break;
    case 'e' :
    case 'E' :
        _pconv_e(cvt, pfield, a_it);
        break;
    case 'g' :
    case 'G' :
        _pconv_g(cvt, pfield, a_it);
        break;
    case 'a' :
    case 'A' :
        _pconv_a(cvt, pfield, a_it);
    }
}

#if defined(VECTOR_PRINTF_SUPPORT)
/*****************************************************************************/
/* _PPPROC_V_FGEA_ELEM   -  Process a single float/double number with        */
/*                          f, g, G, e, E, a, and A conversion               */
/*                                                                           */
/*    This function takes the structure PFIELD, which contains all of the    */
/*    flags and parameters to process the conversion of a single             */
/*    floating-point number cvt, stores the result in the string pointed     */
/*    to by *A_IT.                                                           */
/*                                                                           */
/*    It returns the number of characters written to *A_IT                   */
/*****************************************************************************/
static int _pproc_v_fgea_elem(_PFIELD *pfield, double cvt, int *flags,
                              char **a_it)
{
    char* a_it_initial_pos  = *a_it;

    _pproc_fgea_flag_sign_conv(pfield, cvt, flags, a_it);

    return (a_it_initial_pos - *a_it);
}
#endif /* defined(VECTOR_PRINTF_SUPPORT) */

/*****************************************************************************/
/* _PPPROC_FGEA   -  Process the conversion for f, g, G, e, E, a, and A      */
/*                                                                           */
/*    This function takes the structure PFIELD, which contains all of the    */
/*    flags and parameters to process the conversion, and it does this       */
/*    conversion, and stores the result in the string pointed to by          */
/*    *A_IT.                                                                 */
/*****************************************************************************/
static void _pproc_fgea(_PFIELD *pfield, int *flags, char **a_it, va_list *_ap)
{
    /*-----------------------------------------------------------------------*/
    /* Local variables                                                       */
    /*-----------------------------------------------------------------------*/
    long double cvt =  0;

    /*-----------------------------------------------------------------------*/
    /* Determine what kind of argument is coming next, and read it into CVT. */
    /*-----------------------------------------------------------------------*/
    switch (_STCHK(pfield, _MFLD))
    {
    case  0  :
        cvt = (double)va_arg((*_ap), double);
        break;

    default  :
        cvt = va_arg((*_ap), long double);

    }

    _pproc_fgea_flag_sign_conv(pfield, cvt, flags, a_it);
}

#if defined(VECTOR_PRINTF_SUPPORT)
/*****************************************************************************/
/* _PPPROC_V_FGEA   -  Process the vector conversion for                     */
/*                      f, g, G, e, E, a, and A                              */
/*                                                                           */
/*    This function takes the structure PFIELD, which contains all of the    */
/*    flags and parameters to process the conversion, and it does this       */
/*    conversion element by element, stores each result in the string        */
/*    pointed to by *A_IT, then writes out the element string to a stream    */
/*    using the provided _outs and _outc functions.                          */
/*                                                                           */
/*    The accepted vector lengths are 2, 3, 4, 8 and 16.                     */
/*    The accepted length sub-specifiers are hh (_MFHH), h (_MFH),           */
/*    hl (_MFHL) and l (_MFL).                                               */
/*                                                                           */
/*    The function returns the number of characters written to the stream.   */
/*****************************************************************************/
static  int _pproc_v_fgea(_PFIELD *pfield, int *flags, int* minus_flag,
                          char**a_it, va_list *_ap,
                          void * __restrict _op, int (*_outc)(char, void *),
			              int (*_outs)(char *, void *, int),
                          unsigned int _ap_orig)
{
    /*-----------------------------------------------------------------------*/
    /* Local variables                                                       */
    /*-----------------------------------------------------------------------*/
    int      i;
    int      _outs_result         = 0;
    int      _outc_result         = 0;
    int      elem_length          = 0;
    int      last_len_added       = 0;
    int      total_length_written = 0;
    double   cvt                  = 0;

    union PTRTYPE
    {
        float   *fptr;
        double  *dptr;
    };

    union PTRTYPE vecptr = {NULL};

    union FVECTYPE
    {
        float2   f2;
        float3   f3;
        float4   f4;
        float8   f8;
        float16  f16;
        double2  d2;
        double3  d3;
        double4  d4;
        double8  d8;
        double16 d16;
    } fvec;

    /*-----------------------------------------------------------------------*/
    /* Helper macros                                                         */
    /*-----------------------------------------------------------------------*/
#define _PPROC_FLOAT(var) {\
    cvt = (double)var;\
    last_len_added = _pproc_v_fgea_elem(pfield, cvt, flags, a_it);\
    elem_length = _pproc_v_elem_prelude(pfield, a_it, flags, minus_flag,\
                  last_len_added);\
    _outs_result = _write_v_elem(&total_length_written, a_it, elem_length,\
                                 _op, _outs);\
    if (_outs_result == EOF) return _outs_result;\
    }

#define _PPROC_DOUBLE(var) {\
    cvt = var;\
    last_len_added = _pproc_v_fgea_elem(pfield, cvt, flags, a_it);\
    elem_length = _pproc_v_elem_prelude(pfield, a_it, flags, minus_flag,\
                  last_len_added);\
    _outs_result = _write_v_elem(&total_length_written, a_it, elem_length,\
                                 _op, _outs);\
    if (_outs_result == EOF) return _outs_result;\
    }

    /*-----------------------------------------------------------------------*/
    /* Read and process argument                                             */
    /*-----------------------------------------------------------------------*/
    switch (_STCHK(pfield, (_MFHL | _MFL)))
    {

    case _MFHL  : /* floatn */
        switch(pfield->vn)
        {
        case 2 :
            /* float2 => 2x32-bit = 64-bit */
            fvec.f2     = va_arg(*_ap, float2);
            vecptr.fptr = (float*)&fvec.f2;
            break;
        case 3 :
            /* float3 => 3x32-bit = 96-bit */
            fvec.f3     = va_arg(*_ap, float3);
            vecptr.fptr = (float*)&fvec.f3;
            break;
        case 4 :
            /* float4 => 4x32-bit = 128-bit */
            fvec.f4     = va_arg(*_ap, float4);
            vecptr.fptr = (float*)&fvec.f4;
            break;
        case 8 :
            /* float8 => 8x32-bit = 256-bit */
            fvec.f8     = va_arg(*_ap, float8);
            vecptr.fptr = (float*)&fvec.f8;
            break;
        case 16 :
            /* float16 => 16x32-bit = 512-bit */
            fvec.f16    = va_arg(*_ap, float16);
            vecptr.fptr = (float*)&fvec.f16;
            break;
        }
        if (vecptr.fptr != NULL)
        {
            for(i=0; i<pfield->vn; i++)
            {
                _PPROC_FLOAT(vecptr.fptr[i]);
                if (i != pfield->vn - 1)
                {
                    _outc_result = _write_comma(&total_length_written,
                                                _op, _outc);
                    if (_outc_result == EOF) return _outc_result;
                }
            }
        }
        break;

    case _MFL   : /* doublen */
        switch(pfield->vn)
        {
        case 2 :
            /* double2 => 2x64-bit = 128-bit */
            fvec.d2     = va_arg_128bit(_ap_orig, *_ap, double2);
            vecptr.dptr = (double*)&fvec.d2;
            break;
        case 3 :
            /* double3 => 3x64-bit = 192-bit */
            fvec.d3     = va_arg(*_ap, double3);
            vecptr.dptr = (double*)&fvec.d3;
            break;
        case 4 :
            /* double4 => 4x64-bit = 256-bit */
            fvec.d4     = va_arg(*_ap, double4);
            vecptr.dptr = (double*)&fvec.d4;
            break;
        case 8 :
            /* double8 => 8x64-bit = 512-bit */
            fvec.d8     = va_arg(*_ap, double8);
            vecptr.dptr = (double*)&fvec.d8;
            break;
        case 16 :
            /* double16 => 16x64-bit = 1024-bit */
            fvec.d16    = va_arg(*_ap, double16);
            vecptr.dptr = (double*)&fvec.d16;
            break;
        }
        if (vecptr.dptr != NULL)
        {
            for(i=0; i<pfield->vn; i++)
            {
                _PPROC_DOUBLE(vecptr.dptr[i]);
                if (i != pfield->vn - 1)
                {
                    _outc_result = _write_comma(&total_length_written,
                                                _op, _outc);
                    if (_outc_result == EOF) return _outc_result;
                }
            }
        }
        break;

    default     :
        /* Not handling other length sub-specifiers */
        break;
    }

    return total_length_written;
}
#endif

/*****************************************************************************/
/* _PCONV_F -  Perform the %f conversion                                     */
/*                                                                           */
/*    Upon input, **a_it points to where the least significant digit will    */
/*    be written; this function writes backwards from this point.  *a_it is  */
/*    moved backward as characters are written; upon return, *a_it points    */
/*    to the character before the digit sequence.                            */
/*                                                                           */
/*    Upon input, cvt is in [0, LDBL_MAX]                                    */
/*                                                                           */
/*****************************************************************************/
static void _pconv_f(long double cvt, _PFIELD *pfield, char **a_it)
{
   /*------------------------------------------------------------------------*/
   /* Local variables                                                        */
   /*------------------------------------------------------------------------*/
   char  tmpbuf[F_CONVERSION_BUFSIZE];
   char *digits;
   char *frac_ptr;
   int   dpt,
         putdec;

   /*------------------------------------------------------------------------*/
   /* If no precision was specified, set it to 6, as the standard requires.  */
   /*------------------------------------------------------------------------*/
   if (pfield->precision < 0) pfield->precision = 6;

   /*------------------------------------------------------------------------*/
   /* Save the current location of *a_it, which points to where the least    */
   /* significant digit (or '.') will be placed.                             */
   /*------------------------------------------------------------------------*/
   frac_ptr = *a_it;

   /*------------------------------------------------------------------------*/
   /* Convert and round to the number of decimal digits.  The precision      */
   /* for %f is the number of digits after the decimal point.                */
   /*------------------------------------------------------------------------*/
   digits = fcvt(tmpbuf, cvt, pfield->precision, &dpt);

   /*------------------------------------------------------------------------*/
   /* Copy exactly precision digits from the fractional part.                */
   /*------------------------------------------------------------------------*/
   _fcpy(digits, dpt, pfield->precision, a_it);

   /*------------------------------------------------------------------------*/
   /* If the '#' flag was specified, or there was a fractional part to the   */
   /* number, a decimal point will be placed.  We know there was a           */
   /* fractional part if *a_it was changed by the call to _fcpy.             */
   /*------------------------------------------------------------------------*/
   putdec = (frac_ptr != *a_it || _STCHK(pfield, _PFPOUND));

   /*------------------------------------------------------------------------*/
   /* Copy the whole number part.                                            */
   /*------------------------------------------------------------------------*/
   _mcpy(digits, dpt, putdec, a_it);
}

/*****************************************************************************/
/* _PCONV_E -  Perform the %e conversion                                     */
/*                                                                           */
/*    Upon input, **a_it points to where the least significant digit will    */
/*    be written; this function writes backwards from this point.  *a_it is  */
/*    moved backward as characters are written; upon return, *a_it points    */
/*    to the character before the digit sequence.                            */
/*                                                                           */
/*    Upon input, cvt is in [0, LDBL_MAX]                                    */
/*                                                                           */
/*****************************************************************************/
static void _pconv_e(long double cvt, _PFIELD *pfield, char **a_it)
{
   /*------------------------------------------------------------------------*/
   /* Local variables                                                        */
   /*------------------------------------------------------------------------*/
   char  tmpbuf[FLOAT_VALUE_BUFSIZE];
   char *digits;
   int   dpt,
         putdec,
         exp      = 0;

   /*------------------------------------------------------------------------*/
   /* If no precision was specified, set it to 6, as the standard requires.  */
   /*------------------------------------------------------------------------*/
   if (pfield->precision < 0) pfield->precision = 6;

   /*------------------------------------------------------------------------*/
   /* Check if cvt is within legal range -HUGE_VALL <= cvt <= HUGE_VALL.     */
   /* Ideally, HUGE_VALL should be INFINITY, so this should have no effect.  */
   /*------------------------------------------------------------------------*/
   if (cvt >= HUGE_VALL) cvt = HUGE_VALL;
   else if (cvt <= -HUGE_VALL) cvt = -HUGE_VALL;

   /*------------------------------------------------------------------------*/
   /* Scale non-zero values of cvt to [1, 10) so that it will fit the        */
   /* x.xxxe+xx form.  Keep track of the change to the exponent.             */
   /*------------------------------------------------------------------------*/
   if (cvt)
   {
      for (;cvt < 1; cvt *= 10, exp--);
      for (;cvt >= 10; cvt /= 10, exp++);
   }

   /*------------------------------------------------------------------------*/
   /* Convert and round to the number of significant digits.  The precision  */
   /* for %e is the number of digits after the decimal point, so we should   */
   /* get precision+1 digits out of fcvt, including trailing zeros.          */
   /*------------------------------------------------------------------------*/
   digits = fcvt(tmpbuf, cvt, pfield->precision, &dpt);

   /*------------------------------------------------------------------------*/
   /* dpt should always be 1 here unless fcvt rounded up into the next       */
   /* digit, in which case dpt should be two.                                */
   /*------------------------------------------------------------------------*/
   if (dpt==2)
   {
      /*---------------------------------------------------------------------*/
      /* fcvt rounded the number, so re-scale to [1, 10).                    */
      /*---------------------------------------------------------------------*/
      dpt--; exp++; digits[strlen(digits)-1] = 0;
   }

   /*------------------------------------------------------------------------*/
   /* Copy the exponent part.                                                */
   /*------------------------------------------------------------------------*/
   _ecpy(exp, pfield->conv, a_it);

   /*------------------------------------------------------------------------*/
   /* Copy exactly precision digits from the fractional part.                */
   /*------------------------------------------------------------------------*/
   _fcpy(digits, dpt, pfield->precision, a_it);

   /*------------------------------------------------------------------------*/
   /* If the '#' flag was used, or there was a fractional part to the        */
   /* number, a decimal point will be placed.  "dpt" should always be 1      */
   /* here, and so it should only be equal to strlen(digits) if there were   */
   /* no fractional digits.                                                  */
   /*------------------------------------------------------------------------*/
   putdec = (dpt != (int)strlen(digits) || _STCHK(pfield, _PFPOUND)) ? 1 : 0;

   /*------------------------------------------------------------------------*/
   /* Copy the whole number part.                                            */
   /*------------------------------------------------------------------------*/
   _mcpy(digits, dpt, putdec, a_it);
}

/*****************************************************************************/
/* _PCONV_G -  Perform the %g conversion                                     */
/*                                                                           */
/*    Upon input, **a_it points to where the least significant digit will    */
/*    be written; this function writes backwards from this point.  *a_it is  */
/*    moved backward as characters are written; upon return, *a_it points    */
/*    to the character before the digit sequence.                            */
/*                                                                           */
/*    Upon input, cvt is in [0, LDBL_MAX]                                    */
/*                                                                           */
/*****************************************************************************/
static void _pconv_g(long double cvt, _PFIELD *pfield, char **a_it)
{
   /*------------------------------------------------------------------------*/
   /* Local variables                                                        */
   /*------------------------------------------------------------------------*/
   char  tmpbuf[FLOAT_VALUE_BUFSIZE];
   char  *digits;
   int   dpt,
         i,
         digit_seen,
	 putdec,
         exp         = 0;

   /*------------------------------------------------------------------------*/
   /* If the precision was given as 0, set it to one.                        */
   /*------------------------------------------------------------------------*/
   if (pfield->precision == 0) pfield->precision = 1;

   /*------------------------------------------------------------------------*/
   /* If no precision was specified, set it to 6.                            */
   /*------------------------------------------------------------------------*/
   if (pfield->precision < 0) pfield->precision = 6;

   /*------------------------------------------------------------------------*/
   /* Convert and round to the number of significant digits.  Unlike %e and  */
   /* %f, the precision for %g is the number of significant digits, which    */
   /* includes digits before the decimal point.                              */
   /*------------------------------------------------------------------------*/
   digits = ecvt(tmpbuf, cvt, pfield->precision, &dpt);

   /*------------------------------------------------------------------------*/
   /* If the exponent is less than -4, or greater than or equal to the       */
   /* precision, convert the number as a %e conversion.  Otherwise convert   */
   /* it as a %f conversion.  Note that 0.0 will always be formatted as %f.  */
   /*------------------------------------------------------------------------*/
   if (dpt < -3 || dpt > pfield->precision)
   {
      /*---------------------------------------------------------------------*/
      /* We'll format as %e; scale the number to [1, 10)                     */
      /*---------------------------------------------------------------------*/
      for (;dpt > 1; dpt--, exp++);
      for (;dpt < 1; dpt++, exp--);

      /*---------------------------------------------------------------------*/
      /* Copy the exponent part; subtract two to change g->e and G->E.       */
      /*---------------------------------------------------------------------*/
      _ecpy(exp, pfield->conv-2, a_it);
   }

   /*------------------------------------------------------------------------*/
   /* Copy the fractional part of the number.  Ecvt produces exactly the     */
   /* number of significant digits specified by the precision, but we may    */
   /* need to add leading '0' digits if formatting a small number as %f.     */
   /*------------------------------------------------------------------------*/
   for (digit_seen = 0, i = strlen(digits) - 1; i >= dpt; --i)
   {
      char digit = (i >= 0) ? digits[i] : '0';

      /*---------------------------------------------------------------------*/
      /* Unlike %e and %f, %g usually omits trailing '0' digits in the       */
      /* fractional part.  Do not omit them if the '#' flag is used.         */
      /*---------------------------------------------------------------------*/
      if (digit != '0' || digit_seen || _STCHK(pfield, _PFPOUND))
      {
         *((*a_it)--) = digit;
	 digit_seen = 1;
      }
   }

   /*------------------------------------------------------------------------*/
   /* If the '#' flag was used, or there was a fractional part to the number */
   /* a decimal point will be placed.                                        */
   /*------------------------------------------------------------------------*/
   putdec = (_STCHK(pfield, _PFPOUND) || digit_seen) ? 1 : 0;

   /*------------------------------------------------------------------------*/
   /* Copy the whole number part.                                            */
   /*------------------------------------------------------------------------*/
   _mcpy(digits, dpt, putdec, a_it);
}

/*****************************************************************************/
/* _PCONV_A -  Perform the %a conversion                                     */
/*                                                                           */
/*    Upon input, **a_it points to where the least significant digit will    */
/*    be written; this function writes backwards from this point.  *a_it is  */
/*    moved backward as characters are written; upon return, *a_it points    */
/*    to the character before the digit sequence.                            */
/*                                                                           */
/*    Upon input, cvt is in [0, LDBL_MAX]                                    */
/*                                                                           */
/*****************************************************************************/
static void _pconv_a(long double cvt, _PFIELD *pfield, char **a_it)
{
   /*------------------------------------------------------------------------*/
   /* Local variables                                                        */
   /*------------------------------------------------------------------------*/
   char  tmpbuf[FLOAT_VALUE_BUFSIZE];
   char *digits;
   int   dpt,
         putdec,
         exp      = 0;

   /*------------------------------------------------------------------------*/
   /* Check if CVT is within legal range -HUGE_VALL <= CVT <= HUGE_VALL      */
   /* Ideally, HUGE_VALL should be INFINITY, so this should have no effect.  */
   /*------------------------------------------------------------------------*/
   if (cvt >= HUGE_VALL) cvt = HUGE_VALL;
   else if (cvt <= -HUGE_VALL) cvt = -HUGE_VALL;

   /*------------------------------------------------------------------------*/
   /* Find the base 2 exponent.  frexp finds (f,e) where value==f*pow2(e)    */
   /* and f in [0.5, 1).  We want f in [1, 2), so subtract one from exp for  */
   /* normal values.  Infinite and NaN values don't reach this function.     */
   /*------------------------------------------------------------------------*/
   if (cvt) { frexpl(cvt, &exp); exp -= 1; }

   /*------------------------------------------------------------------------*/
   /* Convert and round to the number of significant digits.  The precision  */
   /* for %a is the number of digits after the radix point.  If precision    */
   /* is not specified, it has the value -1, which means exact precision.    */
   /*------------------------------------------------------------------------*/
   digits = acvt(tmpbuf, cvt, pfield->precision, &dpt, pfield->conv);

   /*------------------------------------------------------------------------*/
   /* Copy the exponent part, which is the decimal value of the base 2 exp.  */
   /*------------------------------------------------------------------------*/
   _ecpy(exp, pfield->conv == 'A' ? 'P' : 'p', a_it);

   /*------------------------------------------------------------------------*/
   /* Copy exactly precision digits from the fractional part.  If precision  */
   /* is -1, copy them all.                                                  */
   /*------------------------------------------------------------------------*/
   if (pfield->precision == -1)
   {
       pfield->precision = strlen(digits) - 1; /* ignore leading 1 in digits */
   }

   /*------------------------------------------------------------------------*/
   /* Copy exactly precision digits from the fractional part.                */
   /*------------------------------------------------------------------------*/
   _fcpy(digits, dpt, pfield->precision, a_it);

   /*------------------------------------------------------------------------*/
   /* If the '#' flag was used, or there was a fractional part to the        */
   /* number, a radix point will be placed.  "dpt" should always be 1 here,  */
   /* and so it should only be less than strlen(digits) if there were no    */
   /* fractional digits.                                                     */
   /*------------------------------------------------------------------------*/
   putdec = (pfield->precision || _STCHK(pfield, _PFPOUND));

   /*------------------------------------------------------------------------*/
   /* Copy the whole number part.                                            */
   /*------------------------------------------------------------------------*/
   _mcpy(digits, dpt, putdec, a_it);

   /*------------------------------------------------------------------------*/
   /* Add the hex prefix.                                                    */
   /*------------------------------------------------------------------------*/
   *((*a_it)--) = pfield->conv == 'A' ? 'X' : 'x';
   *((*a_it)--) = '0';
}

/*****************************************************************************/
/* _FCPY -  Copy the fraction part of a float to a string                    */
/*                                                                           */
/*    Upon input, **a_it points to where the least significant digit will    */
/*    be written; this function writes backwards from this point.  *a_it is  */
/*    moved backward as characters are written; upon return, *a_it points    */
/*    to the character before the digit sequence.                            */
/*                                                                           */
/*    Input "digits" contains the already-rounded digits of the output       */
/*    string, but without sign, decimal point, or exponent.  This function   */
/*    may add extra '0' digits that don't appear in "digits."                */
/*                                                                           */
/*    Used by %e, %f, %g, and %a                                             */
/*                                                                           */
/*****************************************************************************/
static void _fcpy(const char *digits, int dpt, int precision, char **a_it)
{
   /*------------------------------------------------------------------------*/
   /* Find the index of the least significant digit, as specified by the     */
   /* precision.                                                             */
   /*------------------------------------------------------------------------*/
   int   i   = dpt + precision - 1;
   int   max = strlen(digits);

   /*------------------------------------------------------------------------*/
   /* Copy exactly 'precision' digits.  This may add leading '0' digits that */
   /* don't appear in "digits."  In theory this could add trailing '0'       */
   /* digits, but I couldn't find a path where it does that.                 */
   /*------------------------------------------------------------------------*/
   while (precision-- > 0)
   {
      char digit = (i >= 0 && i < max) ? digits[i] : '0';
      *((*a_it)--) = digit;
      --i;
   }
}

/*****************************************************************************/
/* _ECPY -  Copy the "e+xx" part of a float to a string                      */
/*                                                                           */
/*    Upon input, **a_it points to where the least significant digit will    */
/*    be written; this function writes backwards from this point.  *a_it is  */
/*    moved backward as characters are written; upon return, *a_it points    */
/*    to the character before the digit sequence.                            */
/*                                                                           */
/*    Used by %a, %e and %g (when choosing the %e format)                    */
/*                                                                           */
/*****************************************************************************/
static void _ecpy(int exp, char letter, char **a_it)
{
   /*------------------------------------------------------------------------*/
   /* Always print the exponent as a decimal number.                         */
   /*------------------------------------------------------------------------*/
   _ltostr((exp > 0) ? exp : -exp, 10, 'd', a_it);

   /*------------------------------------------------------------------------*/
   /* e/g/E/G require the decimal exponent to have at least two digits.      */
   /*------------------------------------------------------------------------*/
   if (letter != 'p' && letter != 'P' && (exp < 10 && exp > -10))
       *((*a_it)--) = '0';

   /*------------------------------------------------------------------------*/
   /* Always print the exponent's sign.                                      */
   /*------------------------------------------------------------------------*/
   *((*a_it)--) = (exp < 0) ? '-' : '+';

   /*------------------------------------------------------------------------*/
   /* e/g use 'e' for the exponent prefix, E/G use 'E'.                      */
   /*------------------------------------------------------------------------*/
   *((*a_it)--) = letter;
}

/*****************************************************************************/
/* _MCPY -  Copy the whole number part of a float to a string                */
/*                                                                           */
/*    Upon input, **a_it points to where the least significant digit will    */
/*    be written; this function writes backwards from this point.  *a_it is  */
/*    moved backward as characters are written; upon return, *a_it points    */
/*    to the character before the digit sequence.                            */
/*                                                                           */
/*    Input "digits" contains the already-rounded digits of the output       */
/*    string, but without sign, decimal point, or exponent.  This function   */
/*    may add extra '0' digits that don't appear in "digits."                */
/*                                                                           */
/*    Input "dpt" indicates where the decimal point should fall in "digits," */
/*    in terms of digits from the beginning of the string.  The value of     */
/*    "dpt" may be higher than the length of the string, indicating          */
/*    trailing zeros should be added.  This mainly happens in fcvt when      */
/*    the whole number part has only a few leading non-zero digits.          */
/*                                                                           */
/*    Input "putdec" indicates whether the format requires a decimal point.  */
/*                                                                           */
/*    Used by %e, %f, %g, and %a                                             */
/*                                                                           */
/*****************************************************************************/
static void _mcpy(const char *digits, int dpt, int putdec, char **a_it)
{
   int i;
   const char *wholeend = NULL;
   size_t diglen = strlen(digits);

   /*------------------------------------------------------------------------*/
   /* If the number has both a whole number part and a fractional part,      */
   /* position WHOLEEND to the last character of the whole number.           */
   /*------------------------------------------------------------------------*/
   if (dpt > 0 && dpt <= (int)diglen) wholeend = digits + dpt -1;

   /*------------------------------------------------------------------------*/
   /* Place a decimal point if PUTDEC is set.                                */
   /*------------------------------------------------------------------------*/
   if (putdec) *((*a_it)--) = '.';

   /*------------------------------------------------------------------------*/
   /* Place any whole number trailing zeros that don't appear in "digits."   */
   /*------------------------------------------------------------------------*/
   for (i = dpt; i > (int)diglen; i--) *((*a_it)--) = '0';

   /*------------------------------------------------------------------------*/
   /* Copy the rest of the whole number.                                     */
   /*------------------------------------------------------------------------*/
   if (i > 0) for (; wholeend >= digits; wholeend--) *((*a_it)--) = *wholeend;
   else *((*a_it)--) = '0';
}
#endif /* !defined(NOFLOAT) && !defined(MINIMAL) */

/*****************************************************************************/
/* _PPROC_DIOUXP   -  Process the conversion for d, i, o, u, x, and p        */
/*                                                                           */
/*    This function takes the structure PFIELD, which contains all of the    */
/*    flags and parameters to process the conversion, and it does this       */
/*    conversion, and stores the result in the string pointed to by          */
/*    *A_IT.                                                                 */
/*                                                                           */
/*****************************************************************************/
static int _pproc_diouxp(_PFIELD *pfield, int *minus_flag, char **a_it,
                         va_list *_ap)
{
   /*------------------------------------------------------------------------*/
   /* Local variables                                                        */
   /*------------------------------------------------------------------------*/
   int digits =  0;
   int base   = 10;
   ubig_t cvt;

#if !defined(MINIMAL)
   /*------------------------------------------------------------------------*/
   /* If no precision was given, set it to 1.                                */
   /*------------------------------------------------------------------------*/
   if (pfield->precision < 0) pfield->precision = 1;
   else                      _UNSET(pfield, _PFZERO);
#endif

   /*------------------------------------------------------------------------*/
   /* Set the base of the number by the type of conversion specified.        */
   /*------------------------------------------------------------------------*/
   switch (pfield->conv)
   {
#if !defined(MINIMAL)
      case 'p' :
      case 'X' :
#endif
      case 'x' :
                  base = 16;
                  break;

      case 'o' :  base = 8;
                  break;

      default  :
#if !defined(MINIMAL)
      case 'u' :
      case 'i' :
#endif
      case 'd' :
                  base = 10;
  	          break;
   }

   /*------------------------------------------------------------------------*/
   /* Get the next argument.                                                 */
   /*------------------------------------------------------------------------*/
   cvt = _getarg_diouxp(pfield, _ap);

#if !defined(MINIMAL)
   /*------------------------------------------------------------------------*/
   /* If the precision is 0, and the number is 0, and the # flag is not      */
   /* used, print no characters and return 1.  However, given the same       */
   /* conditions except the # flag is used, write a single '0'.  We fall     */
   /* through to accomplish this in _ltostr.                                 */
   /*------------------------------------------------------------------------*/
   if (pfield->precision == 0 && cvt == 0 && !_STCHK(pfield, _PFPOUND))
       return 1;
#endif

   /*------------------------------------------------------------------------*/
   /* If the number is signed and negative, set the minus sign flag, and     */
   /* negate the number.                                                     */
   /*------------------------------------------------------------------------*/
   if ((pfield->conv == 'd'
#if !defined(MINIMAL)
|| pfield->conv == 'i'
#endif
      )  && ((big_t)cvt < 0))
   {
      *minus_flag = 1;
      cvt = -(big_t)cvt;
   }

   /*------------------------------------------------------------------------*/
   /* Call the function to convert the number to a string, and add the       */
   /* total number of digits assigned into DIGITS.                           */
   /*------------------------------------------------------------------------*/
   digits += _ltostr(cvt, base, pfield->conv, a_it);

#if !defined(MINIMAL)
   /*------------------------------------------------------------------------*/
   /* Fill in the remainder of the precision with zeros.                     */
   /*------------------------------------------------------------------------*/
   while (digits++ < pfield->precision) *((*a_it)--) = '0';

   /*------------------------------------------------------------------------*/
   /* If the "#" flag was used in the X, x, or p conversion, prefix a "0x"   */
   /* or "0X" to the hexadecimal number.  We treat the p conversion exactly  */
   /* the same as the x conversion; this means that when "%p" will print     */
   /* "3", "%#p" will print "0x3".  The standard explicitly says using #     */
   /* with p is undefined behavior, so we can handle this any way we like,   */
   /* don't need to document it, and can change it without notice.           */
   /*------------------------------------------------------------------------*/
   if (HEX_CONV(pfield->conv) && _STCHK(pfield, _PFPOUND)
                              && cvt !=0
                              && pfield->precision != 0)
   {
      *((*a_it)--) = (pfield->conv == 'p' ? 'x' : pfield->conv);
      *((*a_it)--) = '0';
   }

   /*------------------------------------------------------------------------*/
   /* If the "#' flag was used in the o conversion, prefix a "0" to the      */
   /* octal number, but only if the first digit is not already '0'.          */
   /*------------------------------------------------------------------------*/
   if (pfield->conv == 'o' && _STCHK(pfield, _PFPOUND) && (*a_it)[1] != '0')
       *((*a_it)--) = '0';
#endif

   return 0;
}

/*****************************************************************************/
/* _GETARG_DIOUXP -  Get the argument for a d, i, o, u, x, or p conversion   */
/*                                                                           */
/*    This function takes the next argument off the argument list, after     */
/*    determining what kind of argument it is.  It decides this by checking  */
/*    to see if the 'h' or the 'l' flag was used.  It returns the next       */
/*    argument.                                                              */
/*****************************************************************************/
static ubig_t _getarg_diouxp(_PFIELD *pfield, va_list *_ap)
{
   /*------------------------------------------------------------------------*/
   /* Local variables                                                        */
   /*------------------------------------------------------------------------*/
   ubig_t cvt = 0;

   if (pfield->conv == 'p') cvt = (ubig_t)(uintptr_t)va_arg(*_ap, void *);
   else
   /*------------------------------------------------------------------------*/
   /* Get the number from the next argument.  Determine what kind of         */
   /* argument by checking for the h or l flag in the format specification.  */
   /*------------------------------------------------------------------------*/
#if !defined(MINIMAL)
   switch (_STCHK(pfield, (_MFH | _MFHH | _MFL | _MFLL | _MFI40 | _MFJ | _MFZ | _MFT)))
   {
      case _MFH   :  switch (pfield->conv)
                     {
                        case 'd' :
                        case 'i' :
			    cvt = va_arg(*_ap, int);
			    break;

                        case 'o' :
                        case 'u' :
                        case 'x' :
                        case 'X' :
			    cvt = (unsigned short) va_arg(*_ap, uint);
                     }
                     break;

      case _MFHH  :  switch (pfield->conv)
                     {
                        case 'd' :
                        case 'i' :
			    cvt = va_arg(*_ap, int);
			    break;

                        case 'o' :
                        case 'u' :
                        case 'x' :
                        case 'X' :
			    cvt = (unsigned char) va_arg(*_ap, uint);
                     }
                     break;

      case _MFL   :  switch (pfield->conv)
                     {
                        case 'd' :
                        case 'i' :
			    cvt = va_arg(*_ap, long);
			    break;

                        case 'o' :
                        case 'u' :
                        case 'x' :
                        case 'X' :
			    cvt = va_arg(*_ap, unsigned long);
                     }
                     break;
#ifdef LLONG_MAX
      case _MFLL  :  switch (pfield->conv)
                     {
                        case 'd' :
                        case 'i' :
			    cvt = va_arg(*_ap, long);
			    break;

                        case 'o' :
                        case 'u' :
                        case 'x' :
                        case 'X' :
			    cvt = va_arg(*_ap, unsigned long);
                     }
                     break;
#endif
#ifdef __TI_INT40_T__
      case _MFI40 :  switch (pfield->conv)
                     {
                        case 'd' :
                        case 'i' :
                            cvt = va_arg(*_ap, __int40_t);
                            break;

                        case 'o' :
                        case 'u' :
                        case 'x' :
                        case 'X' :
                            cvt = va_arg(*_ap, unsigned __int40_t);
                     }
                     break;
#endif
      case _MFJ   :  switch (pfield->conv)
                     {
                        case 'd' :
                        case 'i' :
			    cvt = va_arg(*_ap, intmax_t);
			    break;

                        case 'o' :
                        case 'u' :
                        case 'x' :
                        case 'X' :
			    cvt = va_arg(*_ap, uintmax_t);
                     }
                     break;
      case _MFZ   :  switch (pfield->conv)
                     {
                        case 'd' :
                        case 'i' :
			    cvt = va_arg(*_ap, __ssize_t);
			    break;

                        case 'o' :
                        case 'u' :
                        case 'x' :
                        case 'X' :
			    cvt = va_arg(*_ap, size_t);
                     }
                     break;
      case _MFT   :  switch (pfield->conv)
                     {
                        case 'd' :
                        case 'i' :
			    cvt = va_arg(*_ap, ptrdiff_t);
			    break;

                        case 'o' :
                        case 'u' :
                        case 'x' :
                        case 'X' :
			    cvt = va_arg(*_ap, __uptrdiff_t);
                     }
                     break;

      default     :
#endif  /* MINIMAL */
	             switch (pfield->conv)
                     {
                        case 'd' :
#if !defined(MINIMAL)
                        case 'i' :
#endif
			    cvt = va_arg(*_ap, int);
			    break;

                        case 'o' :
#if !defined(MINIMAL)
                        case 'u' :
                        case 'X' :
#endif
                        case 'x' :
			    cvt = va_arg(*_ap, uint);
                     }
#if !defined(MINIMAL)
   }
#endif

   return cvt;
}

#if defined(VECTOR_PRINTF_SUPPORT)
/*****************************************************************************/
/* _PPROC_V_DIOUX_ELEM -  Process a vector element for                       */
/*                        d, i, o, u, x  conversion                          */
/*                                                                           */
/*    This function takes an element stored in a ulonglong container,        */
/*    a structure PFIELD, which contains all of the parameters to process    */
/*    the conversion. It does this conversion, without processing flags,     */
/*    sign or width for the element. The result is stored in the string      */
/*    pointed to by *A_IT                                                    */
/*                                                                           */
/*    It returns the number of characters added to *A_IT                     */
/*                                                                           */
/*****************************************************************************/
static int _pproc_v_dioux_elem(_PFIELD *pfield, ulonglong elem,
                               int *minus_flag, char **a_it)
{
    /*-----------------------------------------------------------------------*/
    /* Local variables                                                       */
    /*-----------------------------------------------------------------------*/
    int digits              =  0;
    int base                = 10;
    ubig_t cvt              = 0;
    char* a_it_initial_pos  = *a_it;

    /*-----------------------------------------------------------------------*/
    /* Set the base of the number by the type of conversion specified.       */
    /*-----------------------------------------------------------------------*/
    switch (pfield->conv)
    {
    case 'X' :
    case 'x' :
        base = 16;
        break;

    case 'o' :
        base = 8;
        break;

    default  :
    case 'u' :
    case 'i' :
    case 'd' :
        base = 10;
        break;
    }

    /*-----------------------------------------------------------------------*/
    /* Convert element into requested type                                   */
    /*-----------------------------------------------------------------------*/
    switch (_STCHK(pfield, (_MFH | _MFHH | _MFL | _MFHL)))
    {
    case _MFHH  : /* char or uchar */
        switch (pfield->conv)
        {
        case 'd' :
        case 'i' :
            cvt = (char) (as_char8(elem).s0); /* First 8 bits */
            break;

        case 'o' :
        case 'u' :
        case 'x' :
        case 'X' :
            cvt = (unsigned char) (as_uchar8(elem).s0); /* First 8 bits */
        }
        break;

    case _MFH  : /* short or ushort */
        switch (pfield->conv)
        {
        case 'd' :
        case 'i' :
            cvt = (short) (as_short4(elem).s0); /* First 16 bits */
            break;

        case 'o' :
        case 'u' :
        case 'x' :
        case 'X' :
            cvt = (unsigned short) (as_ushort4(elem).s0); /* First 16 bits */
        }
        break;

    case _MFHL   : /* int or uint */
        switch (pfield->conv)
        {
        case 'd' :
        case 'i' :
            cvt = (int) (as_int2(elem).s0); /* First 32 bits */
            break;

        case 'o' :
        case 'u' :
        case 'x' :
        case 'X' :
            cvt = (unsigned int) (as_uint2(elem).s0); /* First 32 bits */
        }
        break;
    case _MFL   : /* long or unsigned long */
        switch (pfield->conv)
        {
        case 'd' :
        case 'i' :
            cvt = (long int) as_longlong(elem); /* All 64 bits */
            break;

        case 'o' :
        case 'u' :
        case 'x' :
        case 'X' :
            cvt = (unsigned long int) elem; /* All 64 bits */
        }
        break;

    default     :
        /* Not handling other length sub-specifiers */
        break;
    }

    /*-----------------------------------------------------------------------*/
    /* If the precision is 0, and the number is 0, and the # flag is not     */
    /* used, print no characters.  However, given the same                   */
    /* conditions except the # flag is used, write a single '0'.  We fall    */
    /* through to accomplish this in _ltostr.                                */
    /*-----------------------------------------------------------------------*/
    if (pfield->precision == 0 && cvt == 0 && !_STCHK(pfield, _PFPOUND))
        return 0;

    /*-----------------------------------------------------------------------*/
    /* If the number is signed and negative, set the minus sign flag, and    */
    /* negate the number.                                                    */
    /*-----------------------------------------------------------------------*/
    if ((pfield->conv == 'd' || pfield->conv == 'i')  && ((big_t)cvt < 0))
    {
        *minus_flag = 1;
        cvt = -(big_t)cvt;
    }

    /*-----------------------------------------------------------------------*/
    /* Call the function to convert the number to a string, and add the      */
    /* total number of digits assigned into DIGITS.                          */
    /*-----------------------------------------------------------------------*/
    digits += _ltostr(cvt, base, pfield->conv, a_it);

    /*-----------------------------------------------------------------------*/
    /* Fill in the remainder of the precision with zeros.                    */
    /*-----------------------------------------------------------------------*/
    while (digits++ < pfield->precision) *((*a_it)--) = '0';

    /*-----------------------------------------------------------------------*/
    /* If the "#" flag was used in the X, x, or p conversion, prefix a "0x"  */
    /* or "0X" to the hexadecimal number.  We treat the p conversion exactly */
    /* the same as the x conversion; this means that when "%p" will print    */
    /* "3", "%#p" will print "0x3".  The standard explicitly says using #    */
    /* with p is undefined behavior, so we can handle this any way we like,  */
    /* don't need to document it, and can change it without notice.          */
    /*-----------------------------------------------------------------------*/
    if (HEX_CONV(pfield->conv) && _STCHK(pfield, _PFPOUND)
                               && cvt !=0
                               && pfield->precision != 0)
    {
        *((*a_it)--) = (pfield->conv == 'p' ? 'x' : pfield->conv);
        *((*a_it)--) = '0';
    }

    /*-----------------------------------------------------------------------*/
    /* If the "#' flag was used in the o conversion, prefix a "0" to the     */
    /* octal number, but only if the first digit is not already '0'.         */
    /*-----------------------------------------------------------------------*/
    if (pfield->conv == 'o' && _STCHK(pfield, _PFPOUND) && (*a_it)[1] != '0')
        *((*a_it)--) = '0';

    /*-----------------------------------------------------------------------*/
    /* Return the number of characters added to a_it                         */
    /*-----------------------------------------------------------------------*/
    return (a_it_initial_pos - *a_it);
}

/*****************************************************************************/
/* _PPROC_V_DIOUX   -  Process the vector conversion for d, i, o, u, x       */
/*                                                                           */
/*    This function takes the structure PFIELD, which contains all of the    */
/*    flags and parameters to process the conversion, and it does this       */
/*    conversion element by element, stores each result in the string        */
/*    pointed to by *A_IT, then writes out the element string to a stream    */
/*    using the provided _outs and _outc functions.                          */
/*                                                                           */
/*    The accepted vector lengths are 2, 3, 4, 8 and 16.                     */
/*    The accepted length sub-specifiers are hh (_MFHH), h (_MFH),           */
/*    hl (_MFHL) and l (_MFL).                                             */
/*                                                                           */
/*    The function returns the number of characters written to the stream.   */
/*****************************************************************************/
static  int _pproc_v_dioux(_PFIELD *pfield, int* flags, int *minus_flag,
                           char **a_it, va_list *_ap,
                           void * __restrict _op, int (*_outc)(char, void *),
			               int (*_outs)(char *, void *, int),
                           unsigned int _ap_orig)
{
    /*-----------------------------------------------------------------------*/
    /* If no precision was given, set it to 1.                               */
    /*-----------------------------------------------------------------------*/
    if (pfield->precision < 0) pfield->precision = 1;
    else                      _UNSET(pfield, _PFZERO);

    /*-----------------------------------------------------------------------*/
    /* Local variables                                                       */
    /*-----------------------------------------------------------------------*/
    int         i;
    int         _outs_result         = 0;
    int         _outc_result         = 0;
    int         elem_length          = 0;
    int         last_len_added       = 0;
    int         total_length_written = 0;
    ulonglong   cvt                  = 0;
    uchar2      uc2;

    union PTRTYPE
    {
        uchar       *ucptr;
        ushort      *usptr;
        uint        *uiptr;
        ulonglong   *ullptr;
    };

    union PTRTYPE vecptr = {NULL};

    union IVECTYPE
    {
        uchar4      uc4;
        uchar8      uc8;
        uchar16     uc16;
        ushort2     us2;
        ushort3     us3;
        ushort4     us4;
        ushort8     us8;
        ushort16    us16;
        uint2       ui2;
        uint3       ui3;
        uint4       ui4;
        uint8       ui8;
        uint16      ui16;
        ulonglong2  ull2;
        ulonglong3  ull3;
        ulonglong4  ull4;
        ulonglong8  ull8;
        ulonglong16 ull16;
    } ivec;
    /*-----------------------------------------------------------------------*/
    /* Helper macros                                                         */
    /*-----------------------------------------------------------------------*/
#define _PPROC_UCHAR(var) {\
    cvt = as_ulonglong((uchar8)(((uchar)var),0,0,0,0,0,0,0));\
    last_len_added = _pproc_v_dioux_elem(pfield, cvt, minus_flag, a_it);\
    elem_length = _pproc_v_elem_prelude(pfield, a_it, flags, minus_flag,\
                  last_len_added);\
    _outs_result = _write_v_elem(&total_length_written, a_it, elem_length,\
                                 _op, _outs);\
    if (_outs_result == EOF) return _outs_result;\
    }\

#define _PPROC_USHORT(var) {\
    cvt = as_ulonglong((ushort4)(((ushort)var),0,0,0));\
    last_len_added = _pproc_v_dioux_elem(pfield, cvt, minus_flag, a_it);\
    elem_length = _pproc_v_elem_prelude(pfield, a_it, flags, minus_flag,\
                  last_len_added);\
    _outs_result = _write_v_elem(&total_length_written, a_it, elem_length,\
                                 _op, _outs);\
    if (_outs_result == EOF) return _outs_result;\
    }

#define _PPROC_UINT(var) {\
    cvt = as_ulonglong((uint2)(((uint)var),0));\
    last_len_added = _pproc_v_dioux_elem(pfield, cvt, minus_flag, a_it);\
    elem_length = _pproc_v_elem_prelude(pfield, a_it, flags, minus_flag,\
                  last_len_added);\
    _outs_result = _write_v_elem(&total_length_written, a_it, elem_length,\
                                 _op, _outs);\
    if (_outs_result == EOF) return _outs_result;\
    }

#define _PPROC_ULONG(var) {\
    cvt = as_ulonglong(var);\
    last_len_added = _pproc_v_dioux_elem(pfield, cvt, minus_flag, a_it);\
    elem_length = _pproc_v_elem_prelude(pfield, a_it, flags, minus_flag,\
                  last_len_added);\
    _outs_result = _write_v_elem(&total_length_written, a_it, elem_length,\
                                 _op, _outs);\
    if (_outs_result == EOF) return _outs_result;\
    }

    /*-----------------------------------------------------------------------*/
    /* Get the number from the next argument.  Determine what kind of        */
    /* argument by checking for the h, hh, l, hl flag in the format          */
    /* specification.                                                        */
    /* Process in reverse order since a_it is being filled out backwards     */
    /*-----------------------------------------------------------------------*/
    switch (_STCHK(pfield, (_MFH | _MFHH | _MFL | _MFHL)))
    {

    case _MFHH  : /* charn or ucharn */
        switch(pfield->vn)
        {
        case 2 :
            /* uchar2 => 2x8-bit = 16-bit */
            /*****************************************************************/
            /* CODEGEN-4880: WORKAROUND                                      */
            /*****************************************************************/
            /* va_arg must fetch a minimum 32-bit element.                   */
            /* However, a 32-bit element fetch progresses the va_list to the */
            /* next 32-bit element. This leads to a problem if 2 consecutive */
            /* uchar2/char2 elements are specified as arguments.  In this    */
            /* situation, the 2nd uchar2 is consumed when the 1st one is     */
            /* fetched. Therefore, the hi 16-bits of the 32-bit fetch is     */
            /* stored in the global variable arguc2 and used if the next     */
            /* consecutive element is also a uchar2/char2. This is ensured by*/
            /* keeping track of the last_v_dioux_arg_size and confirming its */
            /* value to be 2 prior to using the leftover arguc2 value        */
            /*---------------------------------------------------------------*/
            if (pfield->last_v_dioux_arg_size == 2 && pfield->arguc2_leftover)
            {
                uc2 = (uchar2)(pfield->arguc2.s0, pfield->arguc2.s1);
                pfield->arguc2_leftover = 0;
            } else
            {
                ivec.uc4 = va_arg(*_ap, uchar4);
                uc2 = (uchar2)(ivec.uc4.s0, ivec.uc4.s1);
                pfield->arguc2 = (uchar2)(ivec.uc4.s2, ivec.uc4.s3);
                pfield->arguc2_leftover = 1;
            }
            /*---------------------------------------------------------------*/
            /* Note, for C7X, having a uchar2 element precede uchar3,4,8,16  */
            /* produce incorrect output since the charn will be aligned on   */
            /* 8-bit boundary (size of the element type). This is not the    */
            /* case for C6X where it is aligned on the size of the vector    */
            /*****************************************************************/
            /* CODEGEN-4880: WORKAROUND END                                  */
            /*****************************************************************/
            vecptr.ucptr = (uchar*)&uc2;
            break;
        case 3 :
            /* uchar3 => 3x8-bit = 24-bit */
            /*---------------------------------------------------------------*/
            /* va_arg must fetch a minimum 32-bit element.                   */
            /* However, in this case only 8-bits of extra data are fetched.  */
            /* Since a consecutive vector would be aligned to at least a     */
            /* 32 bit boundary, these extra 8-bits can be ignored.           */
            /*---------------------------------------------------------------*/
        case 4 :
            /* uchar4 => 4x8-bit = 32-bit */
            ivec.uc4     = va_arg(*_ap, uchar4);
            vecptr.ucptr = (uchar*)&ivec.uc4;
            break;
        case 8 :
            /* uchar8 => 8x8-bit = 64-bit */
            ivec.uc8     = va_arg(*_ap, uchar8);
            vecptr.ucptr = (uchar*)&ivec.uc8;
            break;
        case 16 :
            /* uchar16 => 16x8-bit = 128-bit */
            ivec.uc16    = va_arg(*_ap, uchar16);
            vecptr.ucptr = (uchar*)&ivec.uc16;
            break;
        }
        if (vecptr.ucptr != NULL)
        {
            for(i=0; i<pfield->vn; i++)
            {
                _PPROC_UCHAR(vecptr.ucptr[i]);
                if (i != pfield->vn - 1)
                {
                    _outc_result = _write_comma(&total_length_written,
                                                _op, _outc);
                    if (_outc_result == EOF) return _outc_result;
                }
            }
        }
        break;

    case _MFH   : /* shortn or ushortn */
        switch(pfield->vn)
        {
        case 2 :
            /* ushort2 => 2x16-bit = 32-bit */
            ivec.us2     = va_arg(*_ap, ushort2);
            vecptr.usptr = (ushort*)&ivec.us2;
            break;
        case 3 :
            /* ushort3 => 3x16-bit = 48-bit */
            ivec.us3     = va_arg(*_ap, ushort3);
            vecptr.usptr = (ushort*)&ivec.us3;
            break;
        case 4 :
            /* ushort4 => 4x16-bit = 64-bit */
            ivec.us4     = va_arg(*_ap, ushort4);
            vecptr.usptr = (ushort*)&ivec.us4;
            break;
        case 8 :
            /* ushort8 => 8x16-bit = 128-bit */
            ivec.us8     = va_arg(*_ap, ushort8);
            vecptr.usptr = (ushort*)&ivec.us8;
            break;
        case 16 :
            /* ushort16 => 16x16-bit = 256-bit */
            ivec.us16    = va_arg(*_ap, ushort16);
            vecptr.usptr = (ushort*)&ivec.us16;
            break;
        }
        if (vecptr.usptr != NULL)
        {
            for(i=0; i<pfield->vn; i++)
            {
                _PPROC_USHORT(vecptr.usptr[i]);
                if (i != pfield->vn - 1)
                {
                    _outc_result = _write_comma(&total_length_written,
                                                _op, _outc);
                    if (_outc_result == EOF) return _outc_result;
                }
            }
        }
        break;

    case _MFHL  : /* intn or uintn */
        switch(pfield->vn)
        {
        case 2 :
            /* uint2 => 2x32-bit = 64-bit */
            ivec.ui2     = va_arg(*_ap, uint2);
            vecptr.uiptr = (uint*)&ivec.ui2;
            break;
        case 3 :
            /* uint3 => 3x32-bit = 96-bit */
            ivec.ui3     = va_arg(*_ap, uint3);
            vecptr.uiptr = (uint*)&ivec.ui3;
            break;
        case 4 :
            /* uint4 => 4x32-bit = 128-bit */
            ivec.ui4     = va_arg(*_ap, uint4);
            vecptr.uiptr = (uint*)&ivec.ui4;
            break;
        case 8 :
            /* uint8 => 8x32-bit = 256-bit */
            ivec.ui8     = va_arg(*_ap, uint8);
            vecptr.uiptr = (uint*)&ivec.ui8;
            break;
        case 16 :
            /* uint16 => 16x32-bit = 512-bit */
            ivec.ui16    = va_arg(*_ap, uint16);
            vecptr.uiptr = (uint*)&ivec.ui16;
            break;
        }
        if (vecptr.uiptr != NULL)
        {
            for(i=0; i<pfield->vn; i++)
            {
                _PPROC_UINT(vecptr.uiptr[i]);
                if (i != pfield->vn - 1)
                {
                    _outc_result = _write_comma(&total_length_written,
                                                _op, _outc);
                    if (_outc_result == EOF) return _outc_result;
                }
            }
        }
        break;

    case _MFL   : /* longlongn or ulonglongn */
        switch(pfield->vn)
        {
        case 2 :
            /* ulonglong2 => 2x64-bit = 128-bit */
            ivec.ull2     = va_arg_128bit(_ap_orig, *_ap, ulonglong2);
            vecptr.ullptr = (ulonglong*)&ivec.ull2;
            break;
        case 3 :
            /* ulonglong3 => 3x64-bit = 192-bit */
            ivec.ull3     = va_arg(*_ap, ulonglong3);
            vecptr.ullptr = (ulonglong*)&ivec.ull3;
            break;
        case 4 :
            /* ulonglong4 => 4x64-bit = 256-bit */
            ivec.ull4     = va_arg(*_ap, ulonglong4);
            vecptr.ullptr = (ulonglong*)&ivec.ull4;
            break;
        case 8 :
            /* ulonglong8 => 8x64-bit = 512-bit */
            ivec.ull8     = va_arg(*_ap, ulonglong8);
            vecptr.ullptr = (ulonglong*)&ivec.ull8;
            break;
        case 16 :
            /* ulonglong16 => 16x64-bit = 1024-bit */
            ivec.ull16    = va_arg(*_ap, ulonglong16);
            vecptr.ullptr = (ulonglong*)&ivec.ull16;
            break;
        }
        if (vecptr.ullptr != NULL)
        {
            for(i=0; i<pfield->vn; i++)
            {
                _PPROC_ULONG(vecptr.ullptr[i]);
                if (i != pfield->vn - 1)
                {
                    _outc_result = _write_comma(&total_length_written,
                                                _op, _outc);
                    if (_outc_result == EOF) return _outc_result;
                }
            }
        }
        break;

    default     :
        /* No processing for any other vector lengths */
        break;
    }
    /*************************************************************************/
    /* CODEGEN-4880: WORKAROUND                                              */
    /*************************************************************************/
    pfield->last_v_dioux_arg_size = pfield->vn;
    /*************************************************************************/
    /* CODEGEN-4880: WORKAROUND END                                          */
    /*************************************************************************/
    return total_length_written;
}
#endif /* defined(VECTOR_PRINTF_SUPPORT) */

/*****************************************************************************/
/* _LTOSTR  -  Convert an integer to a string of up to base 16               */
/*                                                                           */
/*    Converts an integer to an octal, decimal, or hex digit string.         */
/*                                                                           */
/*    Upon input, **a_it points to where the least significant digit will    */
/*    be written; this function writes backwards from this point.  *a_it is  */
/*    moved backward as characters are written; upon return, *a_it points    */
/*    to the character before the digit sequence.                            */
/*                                                                           */
/*    Returns the number of digits written.                                  */
/*                                                                           */
/*****************************************************************************/
static int _ltostr(ubig_t cvt, int base, char conv, char **a_it)
{
    const char *bnum = "0123456789abcdef";
#if !defined(MINIMAL)
    /*-----------------------------------------------------------------------*/
    /* For a capital letter conversion, use capital letters in the output.   */
    /*-----------------------------------------------------------------------*/
    if (conv == 'X') bnum = "0123456789ABCDEF";
#endif

    /*-----------------------------------------------------------------------*/
    /* If the value is zero, we want to write at least one digit, 0.         */
    /*-----------------------------------------------------------------------*/
    if (!cvt) *((*a_it)--) = '0';

    /*-----------------------------------------------------------------------*/
    /* Convert cvt to a string by repeated modulo by base until cvt is 0.    */
    /*-----------------------------------------------------------------------*/
    while (cvt)
    {
        ubig_t quot = _div(cvt, base);
        ubig_t rem = cvt - (quot * base);

        *((*a_it)--) = bnum[rem];
        cvt = quot;
    }
    /*-----------------------------------------------------------------------*/
    /* Subtract 1 because *a_it is one character before the converted digits.*/
    /*-----------------------------------------------------------------------*/
    return strlen(*a_it) - 1;
}

/*****************************************************************************/
/* _DIV  -  Specialized integer division, used by _ltostr                    */
/*                                                                           */
/*    Returns cvt / base.                                                    */
/*                                                                           */
/*    Division must take place in unsigned arithmetic, because signed        */
/*    division can overflow.                                                 */
/*                                                                           */
/*****************************************************************************/
static ubig_t _div(ubig_t cvt, int base)
{
    /*-----------------------------------------------------------------------*/
    /* Use shifts to optimize power-of-two bases                             */
    /*-----------------------------------------------------------------------*/
    switch (base)
    {
	case  8: return cvt >> 3;
	case 16: return cvt >> 4;
    }

    /*-----------------------------------------------------------------------*/
    /* Perform divide in narrowest arithmetic possible, for speed.	     */
    /*-----------------------------------------------------------------------*/
         if (cvt <= UINT_MAX)  return (unsigned int)cvt / base;
    else if (cvt <= ULONG_MAX) return (unsigned long)cvt / base;
#ifdef __TI_INT40_T__
    else if (cvt <= UINT40_T_MAX) return (unsigned __int40_t)cvt / base;
#endif
    else return cvt / base;
}
