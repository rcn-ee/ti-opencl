;*****************************************************************************
; Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
;   All rights reserved.
;
;   Redistribution and use in source and binary forms, with or without
;   modification, are permitted provided that the following conditions are met:
;       * Redistributions of source code must retain the above copyright
;         notice, this list of conditions and the following disclaimer.
;       * Redistributions in binary form must reproduce the above copyright
;         notice, this list of conditions and the following disclaimer in the
;         documentation and/or other materials provided with the distribution.
;       * Neither the name of Texas Instruments Incorporated nor the
;         names of its contributors may be used to endorse or promote products
;         derived from this software without specific prior written permission.
;
;   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
;   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
;   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
;   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
;   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
;   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
;   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
;   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
;   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
;   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
;   THE POSSIBILITY OF SUCH DAMAGE.
;****************************************************************************/

;**************************************************************************
; Given a buffer with the following structure:
;
;     4      4     4      A5:A4   B5:B4   A7:A6   B7:B6   ...
; +-------+-----+-------+-------+-------+-------+-------+
; |FcnAddr| DP  |NumRegs|ArgData|ArgData|ArgData|ArgData| ...
; +-------+-----+-------+-------+-------+-------+-------+
; Ensure NumRegs % 4 === 0, to issue A-side and B-side LDNDWs (cannot parallel)
; 
; After first 20 registers (A4-A13, B4-B13) are exhausted, additional arguments
; are laid out in memory in the order of calling convention.  Copy more args
; from memory onto stack frame before calling child, roll back more args on
; stack frame after call.
; For example, starting from first additional argument, memory layout:
;   4   4      4        4        8
; +---+---+--------+---------+--------+-----+-------+---+-----+
; |pad|int|char,pad|short,pad| double | ...
; +---+---+--------+---------+--------+-----+-------+---+-----+
; more args block size are rounded up to multiple of 8 bytes.
;
;**************************************************************************

	.asg B15, SP
	.asg B14, DP

NEW_ARG_HANDLER .macro SSA1, SSA2, SSB1, SSB2
   [ A1] LDNDW  *A0++(16), SSA2:SSA1
   [ A1] LDNDW  *B3++(16), SSB2:SSB1
|| [ A1] SUB    A1, 4, A1
	 .endm

	.if __TI_EABI__
	.global dsp_rpc
dsp_rpc:
	.else
	.global _dsp_rpc
_dsp_rpc:
	.endif

         STW    A10,    *SP--(48) ; allocate 48 bytes on stack
||       MV     A4, A0            ; Save input message pointer

         ADD    B4, A6, B2        ; save more args (after first 10) end address
||       STW    A14, *+SP(44)     ; preserve parent's A14 on stack
         MV     A6, A14           ; save more args size, A14 persists across child
||       MV     A6, B0            ; copy more args size to loop counter

         STW    A11,     *+SP( 4)
         STDW   B11:B10, *+SP( 8)
||       LDW    *A0++, A2         ; Get function address
         STDW   A13:A12, *+SP(16)
||       LDW    *A0++, B1         ; Get Data Page Pointer
         STDW   B13:B12, *+SP(24)
||       LDW    *A0++(4), A1      ; Get NumRegs, update A0 for loading A-regs
         STW    A4,      *+SP(32)
         STW    B3,      *+SP(36)
         STW    DP,      *+SP(40)
||       ADD    A0, 8, B3         ; B3 for loading B-regs: A4:A5,B4:B5,...
         NOP                      ; wait for A1 load to finish

         ; copy argument into registers (10 cycles total)
         NEW_ARG_HANDLER A4,  A5,  B4,  B5
         NEW_ARG_HANDLER A6,  A7,  B6,  B7
         NEW_ARG_HANDLER A8,  A9,  B8,  B9
         NEW_ARG_HANDLER A10, A11, B10, B11
         NEW_ARG_HANDLER A12, A13, B12, B13

         ; Align SP to 128-byte boundary so that double16 local var is aligned.
         ; Given that A14 is the size of additional args, compute the padding
         ; such that (SP - padding - A14) is aligned on 128-byte boundary.
         ; padding = (SP - A14) & 0x7F
         ; In what follows,
         ; 1) reduce SP by padding
         ; 2) add the padding into A14, to be rolled back
         ; 3) copy additional args onto stack
         ; 4) call kernel function
         ; 5) roll back additional args and the padding
         SUB    SP, A14, A0
         EXTU   A0, 25, 25, A0
         SUB    SP, A0, SP
||       ADD    A14, A0, A14

         ; copy additional argument onto stack
   [!B0] BNOP 	__MAKE_CALL, 5
__COPY_MORE_ARGS:                 ; copy from back to front
         LDNDW  *--B2, A1:A0      ; copy from more args in memory
||       SUB    B0, 8, B0
   [ B0] BNOP   __COPY_MORE_ARGS, 4
         STNDW  A1:A0, *--SP      ; copy onto stack frame for child call

__MAKE_CALL:
         CALL   A2
         NOP    3
         MVKL   __TI_OCL_WG_RETURN, B3
||       MV     B1, DP

__TI_OCL_WG_LAUNCH:
         MVKH   __TI_OCL_WG_RETURN, B3

__TI_OCL_WG_RETURN:

         ADD    SP, A14, SP       ; roll back more args on stack frame
         LDW    *+SP(44), A14     ; restore parent's A14

         LDW    *+SP(40), DP
         LDW    *+SP(36), B3
         LDW    *+SP(32), A4
         LDDW   *+SP(24), B13:B12
         LDDW   *+SP(16), A13:A12
         LDDW   *+SP( 8), B11:B10
         LDW    *+SP( 4), A11
         RET    B3
         LDW    *++SP(48), A10
         NOP    4

	.global __TI_OCL_WG_RETURN
	.global __TI_OCL_WG_LAUNCH
