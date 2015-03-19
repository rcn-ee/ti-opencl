#! /bin/bash

# Generating all permutations of conversion functions
# convert_round.cl: functions involving rounding
# convert.cl:       all other functions
# The reason to split into two functions is prevent inlining of base convert functions 
#     in rounding modes, otherwise, it will lead to wrong C66 scheduling.

declare -a ASAT=("" "_sat");
declare -a ARND=("" "_rtz" "_rte" "_rtp" "_rtn");
declare -a AT=("char" "uchar" "short" "ushort" "int" "uint" "long" "ulong" \
               "float" "double");
declare -a ASZ=("" "2" "4" "8" "16" "3");
declare -a AMIN=("CHAR_MIN" "0" "SHRT_MIN" "0" \
                 "INT_MIN" "0" "LONG_MIN" "0");
declare -a AMAX=("CHAR_MAX" "UCHAR_MAX" "SHRT_MAX" "USHRT_MAX" \
                 "INT_MAX" "UINT_MAX" "LONG_MAX" "ULONG_MAX");
declare -a AC66ROUND=("" "FE_TOWARDZERO" "FE_TONEAREST" "FE_UPWARD" "FE_DOWNWARD");
FILE_ROUND="convert_round.cl"
FILE_NOROUND="convert.cl"

DUMMY=0

scalar_convert() {
  FSIT=$1
  FSOT=$2
  FISAT=$3
  FIRND=$4
  FIIT=$5
  FIOT=$6

  ## 1. Process input for rounding mode
  ## rounding mode only applies to floating point (from floating, to floating)
  if [ $FIIT -gt 7 -o $FIOT -gt 7 ] && [ $FIRND -gt 0 ]; then

    ## to integral, _rtz is the default
    if [ $FIOT -le 7 ]; then
      ## Could Flush to zero here (now in src/{s_ceilf, s_rintf, s_floorf}.c)
      #if [ $FIIT -eq 8 ]; then
      #  echo "  if (((as_uint(x) >> 23) & 0xFF) == 0)  return ($FSOT)0;"
      #fi
      if [ $FIRND -eq 2 ]; then
        echo "  x = rint(x);"
      elif [ $FIRND -eq 3 ]; then
        echo "  x = ceil(x);"
      elif [ $FIRND -eq 4 ]; then
        echo "  x = floor(x);"
      else
        DUMMY=0
      fi

    ## to floating, _rte is the default, and there is no saturation mode
    ## - if OT has enough fraction bits for IT, no need to round
    else
      if [ $FIOT -eq 8 -a $FIIT -le 3 ] || [ $FIOT -eq 9 -a $FIIT -le 5 ] \
                                        || [ $FIOT -eq 9 -a $FIIT -eq 8 ]; then
        echo "  return ($FSOT)x;"
        return 0
      fi

      INTFSOT="int"
      if [ $FSOT == "double" ]; then
        INTFSOT="long"
      fi
      if [ $FIIT -ne 6 -a $FIIT -ne 7 ]; then
        echo "#ifdef USE_C66_HARDWARE_ROUNDING"
        if [ $FIRND -ne 2 ]; then
          echo "  c66_set_round(${AC66ROUND[$FIRND]});"
        fi
        echo "  $FSOT r66 = convert_$FSOT(x); // inlining will cause wrong scheduling"
        if [ $FIRND -ne 2 ]; then
          echo "  c66_set_round(${AC66ROUND[2]});"
        fi
        echo "  return r66;"
        echo "#else"
      else
        ## float/double_rt*((u)long): call special routines
        echo "  return fpconv_${FSOT}_${FSIT}(x, ${AC66ROUND[$FIRND]});"
        return 0
      fi

      if [ $FIRND -ne 2 ]; then
        echo "  $FSOT r = convert_$FSOT(x);"
        echo "  $FSIT y = convert_$FSIT(r);"
      fi
      if [ $FIRND -eq 1 ]; then
        if [ $FIIT -gt 7 ]; then
          echo "  $FSIT abs_x = fabs(x);"
          echo "  $FSIT abs_y = fabs(y);"
        elif [ $(( FIIT % 2 )) -eq 0 ]; then
          echo "  u$FSIT abs_x = abs(x);"
          echo "  u$FSIT abs_y = abs(y);"
        else
          echo "  $FSIT abs_x = abs(x);"
          echo "  $FSIT abs_y = abs(y);"
        fi
        echo "  return select(r, nextafter(r, sign(r) * ($FSOT)-INFINITY), convert_$INTFSOT(abs_y > abs_x));"
      elif [ $FIRND -eq 3 ]; then
        echo "  return select(r, nextafter(r, ($FSOT)INFINITY), convert_$INTFSOT(y < x));"
      elif [ $FIRND -eq 4 ]; then
        echo "  return select(r, nextafter(r, ($FSOT)-INFINITY), convert_$INTFSOT(y > x));"
      else
        echo "  return ($FSOT)x;"
      fi
      if [ $FIIT -ne 6 -a $FIIT -ne 7 ]; then
        echo "#endif"
      fi
      return 0
    fi
  fi

  ## 2. Process input for saturation
  ## - if OT has enough bits for IT and has same sign as IT, do nothing
  ## - if OT has more bits for IT and OT is signed, do nothing
  ## - if OT has enough bits for IT but different sign, reduce from one end
  ## - signed to signed or unsigned: clamp from both ends
  ## - unsigned to signed or unsigned: reduce from one end
  ## - floating to integral: convert to integral, adjust according to converted value
  if [ $FISAT -eq 1 ]; then
    if [ $(( FIIT / 2 )) -le $(( FIOT / 2)) ] && [ $(( FIIT % 2 )) -eq $(( FIOT % 2)) ]; then
      DUMMY=0
    elif [ $(( FIIT / 2 )) -lt $(( FIOT / 2)) ] && [ $(( FIOT % 2 )) -eq 0 ]; then
      DUMMY=0
    elif [ $FIIT -le 7 -a $(( FIIT / 2)) -le $(( FIOT / 2)) ]; then
      if [ $((FIOT % 2)) -eq 0 ]; then
        echo "  x = min(x, ($FSIT)${AMAX[$FIOT]});"
      else
        echo "  x = max(x, ($FSIT)${AMIN[$FIOT]});"
      fi
    elif [ $FIIT -le 7 -a $(( FIIT % 2)) -eq 0 ]; then
      echo "  x = clamp(x, ($FSIT)${AMIN[$FIOT]}, ($FSIT)${AMAX[$FIOT]});"
    elif [ $FIIT -le 7 -a $(( FIIT % 2)) -eq 1 ]; then
      echo "  x = min(x, ($FSIT)${AMAX[$FIOT]});"
    else
      echo "  $FSOT y = convert_$FSOT(x);"
      echo "  y = select(y, ($FSOT)${AMIN[$FIOT]}, convert_$FSOT(x < ($FSIT)${AMIN[$FIOT]}));"
      echo "  y = select(y, ($FSOT)${AMAX[$FIOT]}, convert_$FSOT(x > ($FSIT)${AMAX[$FIOT]}));"
      echo "  return y;"
      return 0
    fi
  fi

  #### 3. (u)long  to float/double, call _rte implementation
  if [ $FIIT -eq 6 -o $FIIT -eq 7 ] && [ $FIOT -eq 8 -o $FIOT -eq 9 ]; then
    echo "  return convert_${FSOT}_rte(x);"
    return 0
  fi

  #### 4. double to float, Flush To Zero input, because hardware won't FTZ output
  if [ $FIIT -eq 9 -a $FIOT -eq 8 ]; then
    echo "  if (((as_ulong(x) >> 52) & 0x7FF) <= 0x380) // biased - 0x3FF <= -0x7F"
    echo "  {"
    echo "    if ((as_ulong(x) & 0x8000000000000000) == 0) return  0.0f;"
    echo "    else                                         return -0.0f;"
    echo "  }"
  fi

  ## 5. Final type conversion
  echo "  return ($FSOT)x;"
}

gen_header() {
  echo "/* !!!! AUTOGENERATED FILE generated by convert_gen.sh !!!!!
   Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
   Inspired by original convert.cl in POCL distribution, see Copyright below.
*/
"

  echo "/* !!!! AUTOGENERATED FILE generated by convert_type.py !!!!!

   DON'T CHANGE THIS FILE. MAKE YOUR CHANGES TO convert_type.py AND RUN:
   $ ./generate-conversion-type-cl.sh

   OpenCL type conversion functions

   Copyright (c) 2013 Victor Oliveira <victormatheus@gmail.com>
   Copyright (c) 2013 Jesse Towner <jessetowner@lavabit.com>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/
"

  echo "#include \"clc.h\"
#define cles_khr_int64
#define cl_khr_fp64

#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif
"

  echo "/* In consistence with defs in open_libm/c66/fenv.h */
#define FE_TONEAREST    0x0000
#define FE_TOWARDZERO   0x0001
#define FE_UPWARD       0x0002
#define FE_DOWNWARD     0x0003
#define USE_C66_HARDWARE_ROUNDING
#ifdef USE_C66_HARDWARE_ROUNDING
extern void c66_set_round(int __round);
#endif
extern float  fpconv_float_ulong(ulong x,  int round);
extern float  fpconv_float_long(long x,    int round);
extern double fpconv_double_ulong(ulong x, int round);
extern double fpconv_double_long(long x,   int round);
"
}


gen_header > $FILE_NOROUND
gen_header > $FILE_ROUND
### For all input type
for IIT in 0 1 2 3 4 5 6 7 8 9; do
if [ $IIT -eq 6 -o $IIT -eq 7 ]; then
echo "#ifdef cles_khr_int64" >> $FILE_NOROUND
echo "#ifdef cles_khr_int64" >> $FILE_ROUND
fi
if [ $IIT -eq 9 ]; then
echo "#ifdef cl_khr_fp64" >> $FILE_NOROUND
echo "#ifdef cl_khr_fp64" >> $FILE_ROUND
fi

### For all output type
for IOT in 0 1 2 3 4 5 6 7 8 9; do
if [ $IOT -eq 6 -o $IOT -eq 7 ] && [ $IIT -ne 6 -a $IIT -ne 7 ]; then
echo "#ifdef cles_khr_int64" >> $FILE_NOROUND
echo "#ifdef cles_khr_int64" >> $FILE_ROUND
fi
if [ $IOT -eq 9 ] && [ $IIT -ne 9 ]; then
echo "#ifdef cl_khr_fp64" >> $FILE_NOROUND
echo "#ifdef cl_khr_fp64" >> $FILE_ROUND
fi

### For all saturation modes
ISATS="0 1"
if [ $IOT -gt 7 ]; then
  ISATS="0"
fi
for ISAT in $ISATS; do

### For all rounding modes
IRNDS="0 1 2 3 4"
for IRND in $IRNDS; do

### For all vector sizes
for ISZ in 0 1 2 3 4 5; do

  SSAT=${ASAT[$ISAT]}
  SRND=${ARND[$IRND]}
  SIT=${AT[$IIT]}
  SOT=${AT[$IOT]}
  SSZ=${ASZ[$ISZ]}
  SHSZ=$SSZ
  if [ $ISZ -gt 0 ]; then
    IHSZ=$(( ISZ - 1))
    SHSZ=${ASZ[$IHSZ]}
  fi
  FUNC=convert_$SOT$SSZ$SSAT$SRND
  FUNC1=convert_$SOT$SSAT$SRND
  FUNC2=convert_${SOT}2$SSAT$SRND
  FUNCH=convert_$SOT$SHSZ$SSAT$SRND
  OUTFILE=$FILE_NOROUND
  if [ $IRND -gt 0 ]; then
    OUTFILE=$FILE_ROUND
  fi

  echo "" >> $OUTFILE
  echo "_CLC_DEF _CLC_OVERLOAD" >> $OUTFILE
  echo "$SOT$SSZ $FUNC($SIT$SSZ x)" >> $OUTFILE
  echo "{" >> $OUTFILE
  if [ "$SIT" == "$SOT" ]; then
    echo "  return x;" >> $OUTFILE
  elif [ a"$SSZ" == "a" ]; then
    ## to call base case with SIT, SOT, SSAT, SRND
    scalar_convert $SIT $SOT $ISAT $IRND $IIT $IOT >> $OUTFILE
  elif [ "$SSZ" == "3" ]; then
    echo "  return ($SOT$SSZ)($FUNC2(x.s01), $FUNC1(x.s2));" >> $OUTFILE
  else
    echo "  return ($SOT$SSZ)($FUNCH(x.lo), $FUNCH(x.hi));" >> $OUTFILE
  fi
  echo "}" >> $OUTFILE

done 
done 
done 

if [ $IOT -eq 6 -o $IOT -eq 7 ] && [ $IIT -ne 6 -a $IIT -ne 7 ]; then
echo "#endif  // ifdef cles_khr_int64" >> $FILE_NOROUND
echo "#endif  // ifdef cles_khr_int64" >> $FILE_ROUND
fi
if [ $IOT -eq 9 ] && [ $IIT -ne 9 ]; then
echo "#endif  // ifdef cl_khr_fp64" >> $FILE_NOROUND
echo "#endif  // ifdef cl_khr_fp64" >> $FILE_ROUND
fi
done

if [ $IIT -eq 6 -o $IIT -eq 7 ]; then
echo "#endif  // ifdef cles_khr_int64" >> $FILE_NOROUND
echo "#endif  // ifdef cles_khr_int64" >> $FILE_ROUND
fi
if [ $IIT -eq 9 ]; then
echo "#endif  // ifdef cl_khr_fp64" >> $FILE_NOROUND
echo "#endif  // ifdef cl_khr_fp64" >> $FILE_ROUND
fi
done

echo "Finished. Outputs: convert.cl convert_round.cl"

