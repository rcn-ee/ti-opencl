;*****************************************************************************
; Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
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

STACK_ARG_HANDLER_4 .macro OFFSET
        CMP       0, R3                 ; check size of args on stack
        BLE       __COPY_ARGS_IN_REG    ;
        SUB       0x4, R3, R3           ; prepare for next iteration
        ; BRANCHCC OCCURS {__COPY_ARGS_IN_REG}
        .align  4
        LDW       *+R4(OFFSET), R1
        STW       R1, *+SP(OFFSET + 4)
        .endm


        .if __TI_EABI__
	.global	eve_rpc
eve_rpc:
	.global	_eve_rpc
_eve_rpc:
        .endif
;*****************************************************************************
;* FUNCTION NAME: eve_rpc                                                    *
;*    eve_rpc(tiocl_eve_builtin_func_table[ocl_msg->builtin_kernel_index],   *
;*            ocl_msg->args_on_stack_size, ocl_msg->args_in_reg);            *
;*    R2: builtin function pointer                                           *
;*    R3: size of args on stack                                              *
;*    R4: address of args in reg array                                       *
;*                                                                           *
;*   Regs Modified     : R0,R1,R2,R3,R4                                      *
;*   Regs Used         : R0,R1,R2,R3,R4                                      *
;*   Local Frame Size  : 128 Args + 0 Auto + 0 Save = 128 byte               *
;*****************************************************************************
        .align  4
        SUB       0x80, SP              ; reserve stack
                                        ; ARP32 won't allow non-constant
                                        ; offset on SP, we have to assume
                                        ; a worst case scenario (128 bytes)

__COPY_ARGS_ON_STACK:
        ADD       0xc, R4, R4           ; move R4 to copy args on stack
        .asg 0, OFFSET                  ; copy up to 128 bytes
        .loop 32
        STACK_ARG_HANDLER_4 OFFSET
        .eval OFFSET + 4, OFFSET
        .endloop

__COPY_ARGS_IN_REG:
        .align  4
        SUB       0xc, R4, R1           ; move R4 back to copy args in reg
        MV        R2, R0                ; builtin function address
        LDW       *+R1(0), R2
        LDW       *+R1(4), R3
        LDW       *+R1(8), R4

__MAKE_CALL:
        CALL      R0                    ; call builtin function
        NOP
        ; CALL OCCURS {indirect}

        .align  4
        ADD       0x80, SP              ; restore stack
        RET
        NOP
        ; RETURN OCCURS
