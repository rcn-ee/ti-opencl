/*
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== mem_segment_definition.xs ========
 */

var KB=1024;
var MB=KB*KB;

var DDR3_ADDR;
var DDR3_SIZE;

var DDR3_BASE_ADDR_0;
var DDR3_BASE_SIZE_0;
var DDR3_BASE_ADDR_1;
var DDR3_BASE_SIZE_1;

var IPU1_CODE_ADDR;
var IPU1_CODE_SIZE;

var IPU1_DATA_ADDR;
var IPU1_DATA_SIZE;

var IPU1_HEAP_ADDR;
var IPU1_HEAP_SIZE;

var EVE1_SRAM_ADDR;
var EVE1_SRAM_SIZE;

var EVE2_SRAM_ADDR;
var EVE2_SRAM_SIZE;

var EVE3_SRAM_ADDR;
var EVE3_SRAM_SIZE;

var EVE4_SRAM_ADDR;
var EVE4_SRAM_SIZE;

var SR0_ADDR;
var SR0_SIZE;

var EVE1_CODE_ADDR;
var EVE1_DATA_ADDR;
var EVE1_VECS_ADDR;

var EVE2_CODE_ADDR;
var EVE2_DATA_ADDR;
var EVE2_VECS_ADDR;

var EVE3_CODE_ADDR;
var EVE3_DATA_ADDR;
var EVE3_VECS_ADDR;

var EVE4_CODE_ADDR;
var EVE4_DATA_ADDR;
var EVE4_VECS_ADDR;

var EVE_CODE_SIZE;
var EVE_DATA_SIZE;
var EVE_VECS_SIZE;

var TRACE_BUF_BASE;
var TRACE_BUF_LEN;
var EXC_DATA_BASE;
var EXC_DATA_LEN;
var PM_DATA_BASE;
var PM_DATA_LEN;

/* EVE configuration spaces */
EVE1_SRAM_ADDR              = 0x42000000;
EVE1_SRAM_SIZE              = 1*MB;

EVE2_SRAM_ADDR              = 0x42100000;
EVE2_SRAM_SIZE              = 1*MB;

EVE3_SRAM_ADDR              = 0x42200000;
EVE3_SRAM_SIZE              = 1*MB;

EVE4_SRAM_ADDR              = 0x42300000;
EVE4_SRAM_SIZE              = 1*MB;


/* Allocating virtual addresses for carveouts */
/* IPU1 code, data, heap,  IPC data: trace buf, exc data, pm data */
IPU1_CODE_SIZE              = 1*MB;
IPU1_DATA_SIZE              = 2*MB;
IPU1_HEAP_SIZE              = 10*MB;

IPU1_CODE_ADDR              = 0x9D300000;
IPU1_DATA_ADDR              = IPU1_CODE_ADDR + IPU1_CODE_SIZE;
IPU1_HEAP_ADDR              = IPU1_DATA_ADDR + IPU1_DATA_SIZE;

TRACE_BUF_LEN               = 384*KB;
EXC_DATA_LEN                = 64*KB;
PM_DATA_LEN                 = 128*KB;

/* IPC Data is a carveout on M4/IPU, not used on EVE */
TRACE_BUF_BASE              = IPU1_HEAP_ADDR + IPU1_HEAP_SIZE;
EXC_DATA_BASE               = TRACE_BUF_BASE + TRACE_BUF_LEN;
PM_DATA_BASE                = EXC_DATA_BASE + EXC_DATA_LEN;


/* Allocating device memory */
/* NOTE: EVE memory + SR0,REMOTE_LOG,... memory must be memreserved in linux
 * dts file
 */

//SR0_SIZE                    = 0xE0000;
SR0_SIZE                    = 2*MB;
//SR0_ADDR                    = 0xA0020000;
SR0_ADDR                    = 0xA1500000;


/* Old SBL: The start address of EVE memory must be 16MB aligned. */
/* With updated SBL, EVE vecs space could be align with 1MB boundary,
 * and if possible try to fit the entire vecs+code+data in the same 16MB
 * section. In this case a single TLB map would be enough to map
 * vecs+code+data of an EVE.
 * tlb_config_eve.c needs to map these EVE memory sections accordingly.
 */
EVE_START_ADDR              = 0xA0100000;

EVE_VECS_SIZE              = 0x001000;
EVE_CODE_SIZE              = 0x1FF000;
EVE_DATA_SIZE              = 0x300000;

EVE1_VECS_ADDR              = EVE_START_ADDR
EVE1_CODE_ADDR              = EVE1_VECS_ADDR        + EVE_VECS_SIZE;
EVE1_DATA_ADDR              = EVE1_CODE_ADDR        + EVE_CODE_SIZE;
EVE2_VECS_ADDR              = EVE1_DATA_ADDR        + EVE_DATA_SIZE;
EVE2_CODE_ADDR              = EVE2_VECS_ADDR        + EVE_VECS_SIZE;
EVE2_DATA_ADDR              = EVE2_CODE_ADDR        + EVE_CODE_SIZE;
EVE3_VECS_ADDR              = EVE2_DATA_ADDR        + EVE_DATA_SIZE;
EVE3_CODE_ADDR              = EVE3_VECS_ADDR        + EVE_VECS_SIZE;
EVE3_DATA_ADDR              = EVE3_CODE_ADDR        + EVE_CODE_SIZE;
EVE4_VECS_ADDR              = EVE3_DATA_ADDR        + EVE_DATA_SIZE;
EVE4_CODE_ADDR              = EVE4_VECS_ADDR        + EVE_VECS_SIZE;
EVE4_DATA_ADDR              = EVE4_CODE_ADDR        + EVE_CODE_SIZE;

function getMemSegmentDefinition_external(core)
{
    var memory = new Array();
    var index = 0;
    memory[index++] = ["EXT_CODE", {
            comment : "IPU1_CODE_MEM",
            name    : "EXT_CODE",
            base    : IPU1_CODE_ADDR,
            len     : IPU1_CODE_SIZE
        }];
    memory[index++] = ["IPU1_DATA_MEM", {
            comment : "IPU1_DATA_MEM",
            name    : "IPU1_DATA_MEM",
            base    : IPU1_DATA_ADDR,
            len     : IPU1_DATA_SIZE
        }];
    memory[index++] = ["IPU1_HEAP_MEM", {
            comment : "IPU1_HEAP_MEM",
            name    : "IPU1_HEAP_MEM",
            base    : IPU1_HEAP_ADDR,
            len     : IPU1_HEAP_SIZE
        }];
    memory[index++] = ["EVE1_VECS_MEM", {
            comment : "EVE1_VECS_MEM",
            name    : "EVE1_VECS_MEM",
            base    : EVE1_VECS_ADDR,
            len     : EVE_VECS_SIZE
        }];
    memory[index++] = ["EVE1_CODE_MEM", {
            comment : "EVE1_CODE_MEM",
            name    : "EVE1_CODE_MEM",
            base    : EVE1_CODE_ADDR,
            len     : EVE_CODE_SIZE
        }];
    memory[index++] = ["EVE1_DATA_MEM", {
            comment : "EVE1_DATA_MEM",
            name    : "EVE1_DATA_MEM",
            base    : EVE1_DATA_ADDR,
            len     : EVE_DATA_SIZE
        }];
    memory[index++] = ["EVE2_VECS_MEM", {
            comment : "EVE2_VECS_MEM",
            name    : "EVE2_VECS_MEM",
            base    : EVE2_VECS_ADDR,
            len     : EVE_VECS_SIZE
        }];
    memory[index++] = ["EVE2_CODE_MEM", {
            comment : "EVE2_CODE_MEM",
            name    : "EVE2_CODE_MEM",
            base    : EVE2_CODE_ADDR,
            len     : EVE_CODE_SIZE
        }];
    memory[index++] = ["EVE2_DATA_MEM", {
            comment : "EVE2_DATA_MEM",
            name    : "EVE2_DATA_MEM",
            base    : EVE2_DATA_ADDR,
            len     : EVE_DATA_SIZE
        }];
    memory[index++] = ["EVE3_VECS_MEM", {
            comment : "EVE3_VECS_MEM",
            name    : "EVE3_VECS_MEM",
            base    : EVE3_VECS_ADDR,
            len     : EVE_VECS_SIZE
        }];
    memory[index++] = ["EVE3_CODE_MEM", {
            comment : "EVE3_CODE_MEM",
            name    : "EVE3_CODE_MEM",
            base    : EVE3_CODE_ADDR,
            len     : EVE_CODE_SIZE
        }];
    memory[index++] = ["EVE3_DATA_MEM", {
            comment : "EVE3_DATA_MEM",
            name    : "EVE3_DATA_MEM",
            base    : EVE3_DATA_ADDR,
            len     : EVE_DATA_SIZE
        }];
    memory[index++] = ["EVE4_VECS_MEM", {
            comment : "EVE4_VECS_MEM",
            name    : "EVE4_VECS_MEM",
            base    : EVE4_VECS_ADDR,
            len     : EVE_VECS_SIZE
        }];
    memory[index++] = ["EVE4_CODE_MEM", {
            comment : "EVE4_CODE_MEM",
            name    : "EVE4_CODE_MEM",
            base    : EVE4_CODE_ADDR,
            len     : EVE_CODE_SIZE
        }];
    memory[index++] = ["EVE4_DATA_MEM", {
            comment : "EVE4_DATA_MEM",
            name    : "EVE4_DATA_MEM",
            base    : EVE4_DATA_ADDR,
            len     : EVE_DATA_SIZE
        }];
    memory[index++] = ["SR0", {
            comment : "SR0",
            name    : "SR0",
            base    : SR0_ADDR,
            len     : SR0_SIZE
        }];

   memory[index++] = ["TRACE_BUF", {
            comment: "TRACE_BUF",
            name: "TRACE_BUF",
            base: TRACE_BUF_BASE,
            len:  TRACE_BUF_LEN,
        }];
    memory[index++] = ["EXC_DATA", {
            comment: "EXC_DATA",
            name: "EXC_DATA",
            base: EXC_DATA_BASE,
            len:  EXC_DATA_LEN,
        }];
    memory[index++] = ["PM_DATA", {
            comment: "PM_DATA",
            name: "PM_DATA",
            base: PM_DATA_BASE,
            len:  PM_DATA_LEN,
        }];

    return (memory);
}

