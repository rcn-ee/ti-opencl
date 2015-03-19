;**************************************************************************
; Given a buffer with the following structure:
;
;     4      4     4    ArgSz1   4    ArgSz2       4
; +-------+-----+-----+-------+-----+-------+---+-----+
; |FcnAddr| DP  |ArgSz|ArgData|ArgSz|ArgData|...|  0  |
; +-------+-----+-----+-------+-----+-------+---+-----+
;
;  This routine will branch to the FcnAddr after setting up the argument
;  registers based on the Arg data in the buffer.
;
;**************************************************************************

	.asg B15, SP
	.asg B14, DP


ARG_HANDLER .macro SS1, SS2	
	 LDW 	*A0++, A1
	 NOP 	4
   [!A1] BNOP 	__MAKE_CALL, 5
         SUB 	A1, 4, A1
   [!A1] LDW 	*A0++, SS1
   [ A1] LDDW 	*A0++, SS2:SS1
	 .endm

ARG_HANDLER_LAST .macro SS1, SS2	
	 ARG_HANDLER SS1, SS2
   	 ADD	4, A0, A0	; in order to advance past null terminator
	 .endm

	.if __TI_EABI__
	.global dsp_rcp
dsp_rcp:
	.else
	.global _dsp_rcp
_dsp_rcp:
	.endif

    STW     A10,*SP--(48)
||  MV      A4, A0            ; Save input message pointer
    STW     A11,*+SP(4)
    STDW    B11:B10,*+SP(8)
||  LDW     *A0++, A2         ; Get function address
    STDW    A13:A12,*+SP(16)
||  LDW     *A0++, B1         ; Get Data Page Pointer
    STDW    B13:B12,*+SP(24)

    ARG_HANDLER 	A4, A5
    ARG_HANDLER 	B4, B5
    ARG_HANDLER 	A6, A7
    ARG_HANDLER 	B6, B7
    ARG_HANDLER 	A8, A9
    ARG_HANDLER 	B8, B9
    ARG_HANDLER 	A10, A11
    ARG_HANDLER 	B10, B11
    ARG_HANDLER 	A12, A13
    ARG_HANDLER_LAST 	B12, B13

__MAKE_CALL:
    CALL 	A2
    STW		A0, *+SP(32)
    STW 	B3, *+SP(36)
    STW 	DP, *+SP(40)
    MVKL 	__RETADDR, B3
||  MV          B1, DP
    MVKH 	__RETADDR, B3

__RETADDR:
    LDW		*+SP(36), B3
    LDW		*+SP(40), DP
    LDW		*+SP(32), A4
    NOP 	2
    RET 	B3
    LDDW    	*+SP(24), B13:B12
    LDDW    	*+SP(16), A13:A12
    LDDW    	*+SP(8),  B11:B10
    LDW     	*+SP(4),  A11
    LDW     	*++SP(48), A10
