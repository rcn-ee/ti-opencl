;* Copyright (C) 2011-2014 Texas Instruments Incorporated - http://www.ti.com/
;*
;* Redistribution and use in source and binary forms, with or without
;* modification, are permitted provided that the following conditions
;* are met:
;*
;* Redistributions of source code must retain the above copyright
;* notice, this list of conditions and the following disclaimer.
;*
;* Redistributions in binary form must reproduce the above copyright
;* notice, this list of conditions and the following disclaimer in the
;* documentation and/or other materials provided with the
;* distribution.
;*
;* Neither the name of Texas Instruments Incorporated nor the names of
;* its contributors may be used to endorse or promote products derived
;* from this software without specific prior written permission.
;*
;* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
;* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
;* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
;* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
;* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
;* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
;* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
;* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
;* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
;* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
;* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*

;* ========================================================================= *
;* TEXAS INSTRUMENTS, INC. *
;* NAME *
;* touch *
;* PLATFORM *
;* C64x *
;* USAGE *
;* This routine is C callable, and has the following C prototype: *
;* void touch *
;* ( *
;* const void *array, /* Pointer to array to touch */ *
;* int length /* Length array in bytes */ *
;* ); *
;* This routine returns no value and discards the loaded data. *
;* DESCRIPTION *
;* The touch() routine brings an array into the cache by reading *
;* elements spaced one cache line apart in a tight loop. This *
;* causes the array to be read into the cache, despite the fact *
;* that the data being read is discarded. If the data is already *
;* present in the cache, the code has no visible effect. *
;* When touching the array, the pointer is first aligned to a cache- *
;* line boundary, and the size of the array is rounded up to the *
;* next multiple of two cache lines. The array is touched with two *
;* parallel accesses that are spaced one cache-line and one bank *
;* apart. A multiple of two cache lines is always touched. *
;* MEMORY NOTE *
;* The code is ENDIAN NEUTRAL. *
;* No bank conflicts occur in this code. *
;* CODESIZE *
;* 84 bytes *
;* CYCLES *
;* cycles = MIN(22, 16 + ((length + 124) / 128)) *
;* For length = 1280, cycles = 27. *
;* The cycle count includes 6 cycles of function-call overhead, but *
;* does NOT include any cycles due to cache misses. *

        .sect   ".text"
	        .clink
		.protected __touch
__touch:
		B .S2 loop ; Pipe up the loop
|| 		MVK .S1 128, A2 ; Step by two cache lines
|| 		ADDAW .D2 B4, 31, B4 ; Round up # of iters
		B .S2 loop ; Pipe up the loop
|| 		CLR .S1 A4, 0, 6, A4 ; Align to cache line
|| 		MV .L2X A4, B0 ; Twin the pointer
		B .S1 loop ; Pipe up the loop
|| 		CLR .S2 B0, 0, 6, B0 ; Align to cache line
|| 		MV .L2X A2, B2 ; Twin the stepping constant
		B .S2 loop ; Pipe up the loop
|| 		SHR .S1X B4, 7, A1 ; Divide by 128 bytes
|| 		ADDAW .D2 B0, 17, B0 ; Offset by one line + one word
		[A1] BDEC .S1 loop, A1 ; Step by 128s through array
|| 		[A1] LDBU .D1T1 *A4++[A2], A3 ; Load from [128*i + 0]
|| 		[A1] LDBU .D2T2 *B0++[B2], B4 ; Load from [128*i + 68]
|| 		SUB .L1 A1, 7, A0
loop:
		[A0] BDEC .S1 loop, A0 ; Step by 128s through array
|| 		[A1] LDBU .D1T1 *A4++[A2], A3 ; Load from [128*i + 0]
|| 		[A1] LDBU .D2T2 *B0++[B2], B4 ; Load from [128*i + 68]
|| 		[A1] SUB .L1 A1, 1, A1
		BNOP .S2 B3, 5 ; Return

;* ========================================================================= *
;* End of file: touch.asm *
;* ------------------------------------------------------------------------- *
;* Copyright © 2011-2014 Texas Instruments, Incorporated. *
;* All Rights Reserved. *
;* ========================================================================= *
