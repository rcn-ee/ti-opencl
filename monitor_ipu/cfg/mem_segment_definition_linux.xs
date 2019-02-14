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

/* AM57xx IPU1 Memory Map: 0x9D00_0000 - 0x9F00_0000 (reserved-memory in DTS)
 *   (Memory are allocated by remoteproc using carveout entries in the resource
 *    table.  Unused memory will be reused by Linux kernel, because of
 *    the "reusable" property on IPU1's reserved memory block in DTS.)
 *
 *   System convention:
 *     0x9D00_0000 - 0x9D10_0000: VRING (carveout/devmem, always at the front)
 *   Carveout entries:
 *     0x9D10_0000 - 0x9D20_0000: TEXT, virtual addr 0x0
 *     0x9D20_0000 - 0x9D30_0000: IPC data (trace_buf, exc_data, pm_data)
 *     0x9D30_0000 - 0x9D40_0000: CODE
 *     0x9D40_0000 - 0x9D80_0000: DATA
 *     0x9D80_0000 - 0x9E00_0000: EVE (4 x 2MB)
 *     0x9E00_0000 - 0x9E10_0000: SR0
 *   Unused:
 *     0x9E10_0000 - 0x9F00_0000: unused, reusable by Linux
 *
 *   Detailed EVE memory map:
 *     0x9D80_0000 - 0x9D80_1000: EVE1 VECS
 *     0x9D80_1000 - 0x9D90_0000: EVE1 CODE
 *     0x9D90_0000 - 0x9DA0_0000: EVE1 DATA
 *     0x9DA0_0000 - 0x9DA0_1000: EVE2 VECS
 *     0x9DA0_1000 - 0x9DB0_0000: EVE2 CODE
 *     0x9DB0_0000 - 0x9DC0_0000: EVE2 DATA
 *     0x9DC0_0000 - 0x9DC0_1000: EVE3 VECS
 *     0x9DC0_1000 - 0x9DD0_0000: EVE3 CODE
 *     0x9DD0_0000 - 0x9DE0_0000: EVE3 DATA
 *     0x9DE0_0000 - 0x9DE0_1000: EVE4 VECS
 *     0x9DE0_1000 - 0x9DF0_0000: EVE4 CODE
 *     0x9DF0_0000 - 0x9E00_0000: EVE4 DATA
 */

/* Allocating virtual addresses for carveouts/devmems in the resource table */
/* We use the identical virtual addresses as the physical addresses */
/* Carveout's physical addresses are allocated by remoteproc */

/* IPC Data: carveout, not used on EVE */
var TRACE_BUF_LEN               = 384*KB;
var EXC_DATA_LEN                = 64*KB;
var PM_DATA_LEN                 = 128*KB;

var TRACE_BUF_BASE              = 0x9D200000;
var EXC_DATA_BASE               = TRACE_BUF_BASE + TRACE_BUF_LEN;
var PM_DATA_BASE                = EXC_DATA_BASE + EXC_DATA_LEN;

/* IPU1 code, data: carveout, (EVE firmware is placed in IPU1 data) */
var IPU1_CODE_SIZE              = 1*MB;
var IPU1_DATA_SIZE              = 4*MB;

var IPU1_CODE_ADDR              = 0x9D300000;
var IPU1_DATA_ADDR              = IPU1_CODE_ADDR + IPU1_CODE_SIZE;

/* EVE vecs, code, data: carveout, (using the IPU1 memory reserved in DTS) */

/* Old SBL: The start address of EVE memory must be 16MB aligned. */
/* With updated SBL, EVE vecs space could be align with 1MB boundary,
 * and if possible try to fit the entire vecs+code+data in the same 16MB
 * section. In this case a single TLB map would be enough to map
 * vecs+code+data of an EVE.
 * tlb_config_eve.c needs to map these EVE memory sections accordingly.
 */
var EVE_START_ADDR              = 0x9D800000;

var EVE_VECS_SIZE               = 0x001000;
var EVE_CODE_SIZE               = 0x0FF000;
var EVE_DATA_SIZE               = 0x100000;

var EVE1_VECS_ADDR              = EVE_START_ADDR;
var EVE1_CODE_ADDR              = EVE1_VECS_ADDR        + EVE_VECS_SIZE;
var EVE1_DATA_ADDR              = EVE1_CODE_ADDR        + EVE_CODE_SIZE;
var EVE2_VECS_ADDR              = EVE1_DATA_ADDR        + EVE_DATA_SIZE;
var EVE2_CODE_ADDR              = EVE2_VECS_ADDR        + EVE_VECS_SIZE;
var EVE2_DATA_ADDR              = EVE2_CODE_ADDR        + EVE_CODE_SIZE;
var EVE3_VECS_ADDR              = EVE2_DATA_ADDR        + EVE_DATA_SIZE;
var EVE3_CODE_ADDR              = EVE3_VECS_ADDR        + EVE_VECS_SIZE;
var EVE3_DATA_ADDR              = EVE3_CODE_ADDR        + EVE_CODE_SIZE;
var EVE4_VECS_ADDR              = EVE3_DATA_ADDR        + EVE_DATA_SIZE;
var EVE4_CODE_ADDR              = EVE4_VECS_ADDR        + EVE_VECS_SIZE;
var EVE4_DATA_ADDR              = EVE4_CODE_ADDR        + EVE_CODE_SIZE;

/* SR0 for IPU/EVE IPC: carveout, (using the IPU1 memory reserved in DTS) */
var SR0_SIZE                    = 1*MB;
var SR0_ADDR                    = 0x9E000000;

/* Memory segements listed here are in the order of increasing physical
 * addresses.  They don't have to be.  But the increasing order helps
 * visualize the memory map. */
function getMemSegmentDefinition_external(core)
{
    var memory = new Array();
    var index = 0;
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

    memory[index++] = ["IPU1_CODE_MEM", {
            comment : "IPU1_CODE_MEM",
            name    : "IPU1_CODE_MEM",
            base    : IPU1_CODE_ADDR,
            len     : IPU1_CODE_SIZE
        }];
    memory[index++] = ["IPU1_DATA_MEM", {
            comment : "IPU1_DATA_MEM",
            name    : "IPU1_DATA_MEM",
            base    : IPU1_DATA_ADDR,
            len     : IPU1_DATA_SIZE
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

    return (memory);
}

