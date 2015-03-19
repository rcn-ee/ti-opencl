/*
* test_c60_reloc.cpp
*
* C6x Relocation Unit Tests.
*
* Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
*
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the
* distribution.
*
* Neither the name of Texas Instruments Incorporated nor the names of
* its contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include "test_c60_reloc.h"
#include <stdlib.h>
#include <stdio.h>

/*****************************************************************************/
/* C60_TestRelocDo                                                           */
/*                                                                           */
/* Tests the C60 version of reloc_do.  In cases where multiple relocation    */
/* types are implemented in the same way, only one type is tested.  For      */
/* instance, R_C6000_xxx, R_C6000_yyy, and R_C6000_zzz are implemented in    */
/* the exact same way and, therefore, only R_C6000_xxx is tested.            */
/*                                                                           */
/* Each test follows the same flow:                                          */
/* 1. A valid instruction is constructed for the relocation type being       */
/*    tested.                                                                */
/* 2. Addend, symbol value, and pc are then created.                         */
/*    (NOTE: static base is not needed, and so 0 is passed. Also, same       */
/*     endianness is assumed.)                                               */
/* 3. reloc_do() is called                                                   */
/* 4. The result is checked.                                                 */
/* 5. Repeat if variations should be considered.                             */
/*                                                                           */
/*****************************************************************************/
//void C60_TestRelocDo::test_R_C6000_NONE() { }

void C60_TestRelocDo::test_R_C6000_ABS32() 
{
    uint32_t address_space = 0x0;
    uint32_t addend = 0x4;
    uint32_t symval = 0x2001000;
    uint32_t pc = 0x0;
            
    unit_c60_reloc_do(R_C6000_ABS32,
                      (uint8_t*) &address_space,
		      addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(address_space, 0x2001004);
}

void C60_TestRelocDo::test_R_C6000_ABS16() 
{ 
    uint16_t address_space = 0x0;
    uint32_t addend = 0x4;
    uint32_t symval = 0xFFE;
    uint32_t pc = 0x0;

    unit_c60_reloc_do(R_C6000_ABS16,
                      (uint8_t*) &address_space,
		      addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(address_space, 0x1002);
}

void C60_TestRelocDo::test_R_C6000_ABS8() 
{ 
    uint8_t address_space = 0x0;
    uint32_t addend = 0x4;
    uint32_t symval = 0xE;
    uint32_t pc = 0x0;

    unit_c60_reloc_do(R_C6000_ABS8, 
                      &address_space,
		      addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(address_space, 0x12);
}

/*---------------------------------------------------------------------------*/
/* PC-Relative Relocation Tests                                              */
/*                                                                           */
/* Our relocation handler assumes that the address of 'opcode' is where the  */
/* relocation is.  Therefore, when creating a PCR test case, we will compute */
/* a value for symval and pc in terms of &opcode.                            */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void C60_TestRelocDo::test_R_C6000_PCR_S21() 
{ 
    uint32_t opcode = 0x00000010;
    uint32_t addend = 0x4;
    uint32_t symval = ((uint32_t)&opcode & 0xffffffe0) + 0x50000;
    uint32_t pc = 0x0;

    /* Test #1 -- destination is forward from PC */
    /*            PCR21 offset = 0x14001         */
    unit_c60_reloc_do(R_C6000_PCR_S21,
             (uint8_t*) &opcode,
             addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x00a00090);

    /* Test #2 -- symval definition implies offset is negative */
    /*            PCR21 offset = 0x1d4001  (signed - negative) */
    opcode = 0x00000010;
    symval = ((uint32_t)&opcode & 0xffffffe0) - 0xb0000;
    unit_c60_reloc_do(R_C6000_PCR_S21,
             (uint8_t*) &opcode,
             addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x0ea00090);
}

void C60_TestRelocDo::test_R_C6000_PCR_S12() 
{ 
    uint32_t opcode = 0x00002120; /* BNOP */
    uint32_t addend = 0x4;
    uint32_t symval = ((uint32_t)&opcode & 0xffffffe0) + 0x500;
    uint32_t pc = 0x0;

    /* Test #1 -- destination is forward from PC */
    /*            PCR12 offset = 0x141           */
    unit_c60_reloc_do(R_C6000_PCR_S12,
             (uint8_t*) &opcode,
             addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x01412120);

    /* Test #2 -- symval definition implies offset is negative */
    /*            PCR12 offset = 0xd41  (signed - negative) */
    opcode = 0x00002120;
    symval = ((uint32_t)&opcode & 0xffffffe0) - 0xb00;
    unit_c60_reloc_do(R_C6000_PCR_S12,
             (uint8_t*) &opcode,
             addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x0d412120);
}

void C60_TestRelocDo::test_R_C6000_PCR_S10() 
{ 
    uint32_t opcode = 0x01001020; /* BDEC */
    uint32_t addend = 0x4;
    uint32_t symval = ((uint32_t)&opcode & 0xffffffe0) + 0x50;
    uint32_t pc = 0x0;

    /* Test #1 -- destination is forward from PC */
    /*            PCR10 offset = 0x15            */
    unit_c60_reloc_do(R_C6000_PCR_S10,
             (uint8_t*) &opcode,
             addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x0102b020);

    /* Test #2 -- symval definition implies offset is negative */
    /*            PCR10 offset = 0x355  (signed - negative) */
    opcode = 0x01001020;
    symval = ((uint32_t)&opcode & 0xffffffe0) - 0xb0;
    unit_c60_reloc_do(R_C6000_PCR_S10,
             (uint8_t*) &opcode,
             addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x017ab020);
}

void C60_TestRelocDo::test_R_C6000_PCR_S7() 
{ 
    uint32_t opcode = 0x03006160; /* ADDKPC */
    uint32_t addend = 0x4;
    uint32_t symval = ((uint32_t)&opcode & 0xffffffe0) + 0x50;
    uint32_t pc = 0x0;

    /* Test #1 -- destination is forward from PC */
    /*            PCR7 offset = 0x15             */
    unit_c60_reloc_do(R_C6000_PCR_S7,
             (uint8_t*) &opcode,
             addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x03156160);

    /* Test #2 -- symval definition implies offset is negative */
    /*            PCR7 offset = 0x75  (signed - negative) */
    opcode = 0x03006160;
    symval = ((uint32_t)&opcode & 0xffffffe0) - 0x30;
    unit_c60_reloc_do(R_C6000_PCR_S7,
             (uint8_t*) &opcode,
             addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x03756160);
}

void C60_TestRelocDo::test_R_C6000_ABS_S16() 
{ 
    uint32_t opcode = 0x03000028; /* MVK */
    uint32_t addend = 0x4;
    uint32_t symval = 0xFFE;
    uint32_t pc = 0x0;

    unit_c60_reloc_do(R_C6000_ABS_S16,
                      (uint8_t*) &opcode,
		      addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x03080128);
}

void C60_TestRelocDo::test_R_C6000_ABS_L16() 
{ 
    uint32_t opcode = 0x03000028; /* MVKL */
    uint32_t addend = 0x4;
    uint32_t symval = 0x04560FFE;
    uint32_t pc = 0x0;

    unit_c60_reloc_do(R_C6000_ABS_L16,
                      (uint8_t*) &opcode,
		      addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x03080128);
}

void C60_TestRelocDo::test_R_C6000_ABS_H16() 
{ 
    uint32_t opcode = 0x03000068; /* MVKH */
    uint32_t addend = 0x4;
    uint32_t symval = 0x04560FFE;
    uint32_t pc = 0x0;

    unit_c60_reloc_do(R_C6000_ABS_H16,
                      (uint8_t*) &opcode,
		      addend, symval, pc, 0, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x03022b68);
}

void C60_TestRelocDo::test_R_C6000_SBR_U15_B() 
{ 
    uint32_t opcode = 0x0300002c; /* LDB */
    uint32_t addend = 0x0;
    uint32_t static_base = 0x04000000;
    uint32_t symval = (static_base + 0x1357);
    uint32_t pc = 0x0;

    /* unsigned 15-bit SBR offset = 0x1357 */
    /* encoded in bits 22 - 8 */
    unit_c60_reloc_do(R_C6000_SBR_U15_B,
                      (uint8_t*) &opcode,
		      addend, symval, pc, static_base, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x0313572c);
}

void C60_TestRelocDo::test_R_C6000_SBR_U15_H() 
{ 
    uint32_t opcode = 0x0300004c; /* LDH */
    uint32_t addend = 0x0;
    uint32_t static_base = 0x04000000;
    uint32_t symval = (static_base + 0x2246);
    uint32_t pc = 0x0;

    /* unsigned 16-bit SBR offset = 0x2246 */
    /* scaled 15-bit SBR offset = 0x1123 */
    /* encoded in bits 22 - 8 */
    unit_c60_reloc_do(R_C6000_SBR_U15_H,
                      (uint8_t*) &opcode,
		      addend, symval, pc, static_base, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x0311234c);
}

void C60_TestRelocDo::test_R_C6000_SBR_U15_W() 
{ 
    uint32_t opcode = 0x0300006c; /* LDW */
    uint32_t addend = 0x0;
    uint32_t static_base = 0x04000000;
    uint32_t symval = (static_base + 0x448c);
    uint32_t pc = 0x0;

    /* unsigned 17-bit SBR offset = 0x448c */
    /* scaled 15-bit SBR offset = 0x1123 */
    /* encoded in bits 22 - 8 */
    unit_c60_reloc_do(R_C6000_SBR_U15_W,
                      (uint8_t*) &opcode,
		      addend, symval, pc, static_base, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x0311236c);
}

void C60_TestRelocDo::test_R_C6000_SBR_S16() 
{ 
    uint32_t opcode = 0x03000028; /* MVK */
    uint32_t addend = 0x0;
    uint32_t static_base = 0x04000000;
    uint32_t symval = (static_base + 0x1357);
    uint32_t pc = 0x0;

    /* Test #1 positive signed 16-bit offset */
    /* 16-bit SBR offset = 0x1357 */
    /* encoded in bits 22-7 of opcode */
    unit_c60_reloc_do(R_C6000_SBR_S16,
                      (uint8_t*) &opcode,
		      addend, symval, pc, static_base, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x0309aba8);

    /* Test #2 negative signed 16-bit offset */
    /* 16-bit SBR offset = 0xeca9  (-0x1357) */
    /* encoded in bits 22-7 of opcode */
    symval = (static_base - 0x1357);
    unit_c60_reloc_do(R_C6000_SBR_S16,
                      (uint8_t*) &opcode,
		      addend, symval, pc, static_base, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x037654a8);
}

void C60_TestRelocDo::test_R_C6000_SBR_L16_B() 
{ 
    uint32_t opcode = 0x03000028; /* MVKL */
    uint32_t addend = 0x0;
    uint32_t static_base = 0x04000000;
    uint32_t symval = (static_base + 0x11123);
    uint32_t pc = 0x0;

    /* 16-bit SBR offset = 0x1123 */
    /* encoded in bits 22-7 of opcode */
    unit_c60_reloc_do(R_C6000_SBR_L16_B,
                      (uint8_t*) &opcode,
		      addend, symval, pc, static_base, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x030891a8);
}

void C60_TestRelocDo::test_R_C6000_SBR_L16_H() 
{ 
    uint32_t opcode = 0x03000028; /* MVKL */
    uint32_t addend = 0x0;
    uint32_t static_base = 0x04000000;
    uint32_t symval = (static_base + 0x12246);
    uint32_t pc = 0x0;

    /* 17-bit SBR offset = 0x12246 */
    /* scaled SBR offset = 0x9123 */
    /* encoded in bits 22-7 of opcode */
    unit_c60_reloc_do(R_C6000_SBR_L16_H,
                      (uint8_t*) &opcode,
		      addend, symval, pc, static_base, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x034891a8);
}

void C60_TestRelocDo::test_R_C6000_SBR_L16_W() 
{ 
    uint32_t opcode = 0x03000028; /* MVKL */
    uint32_t addend = 0x0;
    uint32_t static_base = 0x04000000;
    uint32_t symval = (static_base + 0x1448c);
    uint32_t pc = 0x0;

    /* 18-bit SBR offset = 0x1448c */
    /* scaled SBR offset = 0x5123 */
    /* encoded in bits 22-7 of opcode */
    unit_c60_reloc_do(R_C6000_SBR_L16_W,
                      (uint8_t*) &opcode,
		      addend, symval, pc, static_base, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x032891a8);
}

void C60_TestRelocDo::test_R_C6000_SBR_H16_B() 
{ 
    uint32_t opcode = 0x03000068; /* MVKH */
    uint32_t addend = 0x0;
    uint32_t static_base = 0x04000000;
    uint32_t symval = (static_base + 0x357448c);
    uint32_t pc = 0x0;

    /* total SBR offset = 0x357448c */
    /* upper 16-bits of SBR offset = 0x357 */
    /* encoded in bits 22-7 of opcode */
    unit_c60_reloc_do(R_C6000_SBR_H16_B,
                      (uint8_t*) &opcode,
		      addend, symval, pc, static_base, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x0301abe8);
}

void C60_TestRelocDo::test_R_C6000_SBR_H16_H() 
{ 
    uint32_t opcode = 0x03000068; /* MVKH */
    uint32_t addend = 0x0;
    uint32_t static_base = 0x04000000;
    uint32_t symval = (static_base + 0x357448c);
    uint32_t pc = 0x0;

    /* total SBR offset = 0x357448c */
    /* scaled SBR offset = 0x1aba246 */
    /* upper 16-bits of scaled SBR offset = 0x1ab */
    /* encoded in bits 22-7 of opcode */
    unit_c60_reloc_do(R_C6000_SBR_H16_H,
                      (uint8_t*) &opcode,
		      addend, symval, pc, static_base, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x0300d5e8);
}

void C60_TestRelocDo::test_R_C6000_SBR_H16_W()
{ 
    uint32_t opcode = 0x03000068; /* MVKH */
    uint32_t addend = 0x0;
    uint32_t static_base = 0x04000000;
    uint32_t symval = (static_base + 0x357448c);
    uint32_t pc = 0x0;

    /* total SBR offset = 0x357448c */
    /* scaled SBR offset = 0x0d5d123 */
    /* upper 16-bits of scaled SBR offset = 0x0d5 */
    /* encoded in bits 22-7 of opcode */
    unit_c60_reloc_do(R_C6000_SBR_H16_W,
                      (uint8_t*) &opcode,
		      addend, symval, pc, static_base, 0, 0);

    TS_ASSERT_EQUALS(opcode, 0x03006ae8);
}

/* The DSBT table is accessed via DP-relative addressing with   */
/* an LDW instruction, but the DSBT_INDEX is really an index    */
/* into the DSBT table, the index is scaled to a 4-word offset. */
void C60_TestRelocDo::test_R_C6000_DSBT_INDEX()
{ 
    uint32_t opcode = 0x0300006c; /* LDW */
    uint32_t addend = 0x0;
    uint32_t static_base = 0x04000000;
    uint32_t symval = static_base;
    uint32_t pc = 0x0;

    unit_c60_reloc_do(R_C6000_DSBT_INDEX,
                      (uint8_t*) &opcode,
		      addend, symval, pc, static_base, 0, 3);

    TS_ASSERT_EQUALS(opcode, 0x0300036c);
}

/*****************************************************************************/
/* C60_TestRelUnpackAddend                                                   */
/*                                                                           */
/* Tests the C60 rel_unpack_addend function.                                 */
/*                                                                           */
/* In cases where the addends are unpacked in the same way, only one is      */ 
/* tested.                                                                   */
/*                                                                           */
/* All tests follow the same flow:                                           */
/*                                                                           */
/* 1. Create a valid instruction for the relocation type, where the addend   */
/*    is packed in the instruction.                                          */
/* 2. Call rel_unpack_addend().                                              */
/* 3. Check that the addend is correct.                                      */
/*                                                                           */
/* Relocations may be tested multiple times to handle variations, such as    */
/* positive/negative addends, extra bits depending on the encoding, etc.     */
/*                                                                           */
/* NOTE!! C60 ONLY SUPPORTS RELA TYPE RELOCATIONS, SO ADDEND FIELD IS STORED */
/* IN RELOCATION ENTRY ITSELF.                                               */
/*****************************************************************************/
#if 0
void C60_TestRelUnpackAddend::test_R_C6000_ABS32()
{
    uint32_t address_space=0xFEDCBA9;
    uint32_t addend;

    unit_c60_rel_unpack_addend(R_C6000_ABS32, 
                               (uint8_t*)&address_space, 
                               &addend);

    TS_ASSERT_EQUALS(addend, address_space);
}

void C60_TestRelUnpackAddend::test_R_C6000_ABS16()
{
    uint16_t address_space=0x7FFF;
    uint32_t addend;

    unit_c60_rel_unpack_addend(R_C6000_ABS16, 
                               (uint8_t*)&address_space, 
                               &addend);
            
    TS_ASSERT_EQUALS(addend, 0x7FFF);

    address_space = 0x8000;

    unit_c60_rel_unpack_addend(R_C6000_ABS16, 
                               (uint8_t*)&address_space, 
                               &addend);
            
    TS_ASSERT_EQUALS(addend, 0xFFFF8000);
}
#endif


/*****************************************************************************/
/* C60_TestRelOverflow                                                       */
/*                                                                           */
/* Test the C60 rel_overflow function.                                       */
/*                                                                           */
/* In each case, we test the upper and lower bounds of each relocation type. */
/* Only relocation types where the overflow is checked in rel_overflow are   */
/* considered.  In most cases four tests are performed to test the upper and */
/* lower bounds (1 pass and 1 fail for each).                                */
/*                                                                           */
/* NOTE!! HAVEN'T REFACTORED OVERFLOW CHECK OUT OF RELOCATION HANDLERS FOR   */
/* C60, SO OVERFLOW SHOULD BE TESTED AS PART OF THE RELOC DO(???)            */
/*                                                                           */
/*****************************************************************************/
void C60_TestRelOverflow::test_R_C6000_ABS16()
{
    int32_t reloc_val = 0xFFFF;
    int rval;

    rval = unit_c60_rel_overflow(R_C6000_ABS16, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = 0x10000;

    rval = unit_c60_rel_overflow(R_C6000_ABS16, reloc_val);

    TS_ASSERT_EQUALS(rval, 1);

    reloc_val = -0x8000;

    rval = unit_c60_rel_overflow(R_C6000_ABS16, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = -0x8001;

    rval = unit_c60_rel_overflow(R_C6000_ABS16, reloc_val);
    
    TS_ASSERT_EQUALS(rval, 1);
}

void C60_TestRelOverflow::test_R_C6000_ABS8()
{
    int32_t reloc_val = 0xFF;
    int rval;

    rval = unit_c60_rel_overflow(R_C6000_ABS8, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = 0x100;

    rval = unit_c60_rel_overflow(R_C6000_ABS8, reloc_val);

    TS_ASSERT_EQUALS(rval, 1);

    reloc_val = -0x80;

    rval = unit_c60_rel_overflow(R_C6000_ABS8, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = -0x81;

    rval = unit_c60_rel_overflow(R_C6000_ABS8, reloc_val);
    
    TS_ASSERT_EQUALS(rval, 1);
}

void C60_TestRelOverflow::test_R_C6000_PCR_S21()
{
    int32_t reloc_val = 0x3FFFFC;
    int rval;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S21, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = 0x400000;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S21, reloc_val);

    TS_ASSERT_EQUALS(rval, 1);

    reloc_val = -0x400000;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S21, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = -0x400001;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S21, reloc_val);
    
    TS_ASSERT_EQUALS(rval, 1);
}

void C60_TestRelOverflow::test_R_C6000_PCR_S12()
{
    int32_t reloc_val = 0x1FFC;
    int rval;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S12, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = 0x2000;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S12, reloc_val);

    TS_ASSERT_EQUALS(rval, 1);

    reloc_val = -0x2000;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S12, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = -0x2001;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S12, reloc_val);
    
    TS_ASSERT_EQUALS(rval, 1);
}

void C60_TestRelOverflow::test_R_C6000_PCR_S10()
{
    int32_t reloc_val = 0x7FC;
    int rval;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S10, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = 0x800;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S10, reloc_val);

    TS_ASSERT_EQUALS(rval, 1);

    reloc_val = -0x800;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S10, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = -0x801;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S10, reloc_val);
    
    TS_ASSERT_EQUALS(rval, 1);
}

void C60_TestRelOverflow::test_R_C6000_PCR_S7()
{
    int32_t reloc_val = 0xFC;
    int rval;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S7, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = 0x100;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S7, reloc_val);

    TS_ASSERT_EQUALS(rval, 1);

    reloc_val = -0x100;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S7, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = -0x101;

    rval = unit_c60_rel_overflow(R_C6000_PCR_S7, reloc_val);
    
    TS_ASSERT_EQUALS(rval, 1);
}

void C60_TestRelOverflow::test_R_C6000_SBR_S16()
{
    int32_t reloc_val = 0x7FFF;
    int rval;

    rval = unit_c60_rel_overflow(R_C6000_SBR_S16, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = 0x8000;

    rval = unit_c60_rel_overflow(R_C6000_SBR_S16, reloc_val);

    TS_ASSERT_EQUALS(rval, 1);

    reloc_val = -0x8000;

    rval = unit_c60_rel_overflow(R_C6000_SBR_S16, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = -0x8001;

    rval = unit_c60_rel_overflow(R_C6000_SBR_S16, reloc_val);
    
    TS_ASSERT_EQUALS(rval, 1);
}

void C60_TestRelOverflow::test_R_C6000_ABS_S16()
{
    int32_t reloc_val = 0x7FFF;
    int rval;

    rval = unit_c60_rel_overflow(R_C6000_ABS_S16, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = 0x8000;

    rval = unit_c60_rel_overflow(R_C6000_ABS_S16, reloc_val);

    TS_ASSERT_EQUALS(rval, 1);

    reloc_val = -0x8000;

    rval = unit_c60_rel_overflow(R_C6000_ABS_S16, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = -0x8001;

    rval = unit_c60_rel_overflow(R_C6000_ABS_S16, reloc_val);
    
    TS_ASSERT_EQUALS(rval, 1);
}

void C60_TestRelOverflow::test_R_C6000_SBR_U15_B()
{
    uint32_t reloc_val = 0x7FFF;
    int rval;

    rval = unit_c60_rel_overflow(R_C6000_SBR_U15_B, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = 0x8000;

    rval = unit_c60_rel_overflow(R_C6000_SBR_U15_B, reloc_val);

    TS_ASSERT_EQUALS(rval, 1);
}

void C60_TestRelOverflow::test_R_C6000_SBR_U15_H()
{
    uint32_t reloc_val = 0xFFFE;
    int rval;

    rval = unit_c60_rel_overflow(R_C6000_SBR_U15_H, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = 0xFFFF;

    rval = unit_c60_rel_overflow(R_C6000_SBR_U15_H, reloc_val);

    TS_ASSERT_EQUALS(rval, 1);
}

void C60_TestRelOverflow::test_R_C6000_SBR_U15_W()
{
    uint32_t reloc_val = 0x1FFFC;
    int rval;

    rval = unit_c60_rel_overflow(R_C6000_SBR_U15_W, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = 0x1FFFD;

    rval = unit_c60_rel_overflow(R_C6000_SBR_U15_W, reloc_val);

    TS_ASSERT_EQUALS(rval, 1);
}

void C60_TestRelOverflow::test_R_C6000_DSBT_INDEX()
{
    uint32_t reloc_val = 0x1FFFC;
    int rval;

    rval = unit_c60_rel_overflow(R_C6000_DSBT_INDEX, reloc_val);

    TS_ASSERT_EQUALS(rval, 0);

    reloc_val = 0x1FFFD;

    rval = unit_c60_rel_overflow(R_C6000_DSBT_INDEX, reloc_val);

    TS_ASSERT_EQUALS(rval, 1);
}

