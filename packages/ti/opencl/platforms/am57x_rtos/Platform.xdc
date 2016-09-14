/*
 *  Copyright (c) 2016 by Texas Instruments and others.
 *  All rights reserved. This program and the accompanying materials
 *  are made available under the terms of the Eclipse Public License v1.0
 *  which accompanies this distribution, and is available at
 *  http://www.eclipse.org/legal/epl-v10.html
 *
 *  Contributors:
 *      Texas Instruments - initial implementation
 *
 * */

/*
 *  ======== Platform.xdc ========
 *  Platform support for OpenCL RTOS on DRA7XX
 *
 */

/*!
 *  ======== Platform ========
 *  Platform support for OpenCL RTOS on DRA7XX
 *
 *  This module implements xdc.platform.IPlatform and defines configuration
 *  parameters that correspond to this platform's Cpu's, Board's, etc.
 *
 *  The configuration parameters are initialized in this package's
 *  configuration script (package.cfg) and "bound" to the TCOM object
 *  model.  Once they are part of the model, these parameters are
 *  queried by a program's configuration script.
 *
 *  This particular platform has 1 Cortex-A15, 4 EVE's, 2 Benelli/IPU 
 *  Sub-system (Dual M4's) and 2 C66x DSP's.
 */
metaonly module Platform inherits xdc.platform.IPlatform
{
    readonly config xdc.platform.IPlatform.Board BOARD = {
        id:             "0",
        boardName:      "evmDRA7XX",
        boardFamily:    "evmDRA7XX",
        boardRevision:  null,
    };

    readonly config xdc.platform.IExeContext.Cpu DSP = {
        id:             "0",
        clockRate:      600,
        catalogName:    "ti.catalog.c6000",
        deviceName:     "DRA7XX",
        revision:       "1.0",
    };

    /* Benelli M4 Subsystem */
    readonly config xdc.platform.IExeContext.Cpu M4 = {
        id:             "1",
        clockRate:      212.8,
        catalogName:    "ti.catalog.arm.cortexm4",
        deviceName:     "DRA7XX",
        revision:       "1.0",
    };

    /* EVE */
    readonly config xdc.platform.IExeContext.Cpu EVE = {
        id:             "2",
        clockRate:      535.0,
        catalogName:    "ti.catalog.arp32",
        deviceName:     "DRA7XX",
        revision:       "1.0"
    };

    /* GPP */
    readonly config xdc.platform.IExeContext.Cpu GPP = {
        id:             "3",
        clockRate:      1000.0,  /* Typically set by the HLOS */
        catalogName:    "ti.catalog.arm.cortexa15",
        deviceName:     "DRA7XX",
        revision:       "1.0"
    };

/*  DSP Internal Memory
 */
    readonly config Any L1DSRAM = {
        name: "L1DSRAM", space: "data", access: "RW",
        base: 0x00F00000, len: 0x00008000,
        comment: "32KB RAM/CACHE L1 data memory"
    };

    readonly config Any L2SRAM = {
        name: "L2SRAM", space: "code/data", access: "RWX",
        base: 0x00800000, len: 0x00008000,
        comment: "32KB L2 SRAM for OpenCL runtime"
    };

    readonly config Any OCL_LOCAL = {
        name: "OCL_LOCAL", space: "data", access: "RW",
        base: 0x00808000, len: 0x00020000,
        comment: "128KB L2 SRAM for OpenCL local memory"
    };

/*  Memory Map for OpenCL RTOS on ti.platforms.evmDRA7XX
 *  
 *  Virtual     Physical        Size            Comment
 *  ------------------------------------------------------------------------
 *              8000_0000  1000_0000  ( 256 MB) External Memory
 *
 *  0000_0000 0 8000_0000        100  ( 256  B) EVE1_VECS (vector table)
 *              8000_0100       FF00  ( ~64 KB) --------
 *  0000_0000   8001_0000        100  ( 256  B) EVE2_VECS (vector table)
 *              8001_0100       FF00  ( ~64 KB) --------
 *  0000_0000   8002_0000        100  ( 256  B) EVE3_VECS (vector table)
 *              8002_0100       FF00  ( ~64 KB) --------
 *  0000_0000   8003_0000        100  ( 256  B) EVE4_VECS (vector table)
 *              8003_0100    FE_FF00  ( ~16 MB) --------
 *            1 8100_0000    40_0000  (   4 MB) EVE1_PROG (code, data)
 *              8140_0000    C0_0000  (  12 MB) --------
 *            2 8200_0000    40_0000  (   4 MB) EVE2_PROG (code, data)
 *              8240_0000    C0_0000  (  12 MB) --------
 *            3 8300_0000    40_0000  (   4 MB) EVE3_PROG (code, data)
 *              8340_0000    C0_0000  (  12 MB) --------
 *            4 8400_0000    40_0000  (   4 MB) EVE4_PROG (code, data)
 *              8440_0000    C0_0000  (  12 MB) --------
 *            5 8500_0000   100_0000  (  16 MB) --------
 *            6 8600_0000   100_0000  (  16 MB) --------
 *            7 8700_0000   100_0000  (  16 MB) --------
 *            8 8800_0000   100_0000  (  16 MB) --------
 *            9 8900_0000   100_0000  (  16 MB) --------
 *            A 8A00_0000    80_0000  (   8 MB) IPU1 (code, data), benelli
 *              8A80_0000    80_0000  (   8 MB) IPU2 (code, data), benelli
 *            B 8B00_0000   100_0000  (  16 MB) HOST (code, data)
 *            C 8C00_0000   100_0000  (  16 MB) DSP1 (code, data)
 *            D 8D00_0000   100_0000  (  16 MB) DSP2 (code, data)
 *            E 8E00_0000   100_0000  (  16 MB) SR_0 (ipc)
 *            F 8F00_0000   100_0000  (  16 MB) --------
 *           10 9000_0000  1000_0000  ( 256 MB) --------
 *           11 A000_0000  0100_0000  (  16 MB) OCL_OMP_NOCACHE (data)
 *           12 A100_0000     2_0000  ( 128 KB) OCL_OMP_STACK (stack)
 *              A102_0000    FE_0000  (  15 MB) OCL_OMP_HEAP (heap)
 *           13 A200_0000  5E00_0000  (1504 MB) OCL_GLOBAL (kernel code, data)
 */

    readonly config Any HOST_PROG = {
        name: "HOST_PROG", space: "code/data", access: "RWX",
        base: 0x8B000000, len: 0x1000000,
        comment: "HOST Program Memory (16 MB)"
    };

    readonly config Any DSP1_PROG = {
        name: "DSP1_PROG", space: "code/data", access: "RWX",
        base: 0x8C000000, len: 0x1000000,
        comment: "DSP1 Program Memory (16 MB)"
    };

    readonly config Any DSP2_PROG = {
        name: "DSP2_PROG", space: "code/data", access: "RWX",
        base: 0x8D000000, len: 0x1000000,
        comment: "DSP2 Program Memory (16 MB)"
    };

    readonly config Any SR_0 = {
        name: "SR_0", space: "data", access: "RWX",
        base: 0x8E000000, len: 0x1000000,
        comment: "SR#0 Memory (16 MB)"
    };

    config Any OCL_OMP_NOCACHE = {
        name: "OCL_OMP_NOCACHE", space: "data", access: "RWX",
        base: 0xa0000000, len:  0x01000000,
        comment: "Shared non-cached memory for OpenMP runtime internal structures (16MB)"
    };

    config Any OCL_OMP_STACK = {
        name: "OCL_OMP_STACK", space: "data", access: "RWX",
        base: 0xa1000000, len:  0x00020000,
        comment: "Stacks for tasks executing OpenMP regions in the DSP monitor (64KB per DSP)"
    };

    config Any OCL_OMP_HEAP = {
        name: "OCL_OMP_HEAP", space: "data", access: "RWX",
        base: 0xa1020000, len:  0x00FE0000,
        comment: "Region for .sysmem - shared heap across DSPs (15MB)"
    };

    config Any OCL_GLOBAL = {
        name: "OCL_GLOBAL", space: "data", access: "RWX",
        base: 0xA2000000, len: 0x5E000000,
        comment: "DDR Memory (1504 MB) for OpenCL global memory"
    };

    readonly config Any DSP1 = {
        customMemoryMap: [
            [ "L1DSRAM", L1DSRAM ],
            [ "L2SRAM", L2SRAM ],
            [ "OCL_LOCAL", OCL_LOCAL ],
            [ "DSP1_PROG", DSP1_PROG ],
            [ "SR_0", SR_0 ],
            [ "OCL_OMP_NOCACHE", OCL_OMP_NOCACHE ],
            [ "OCL_OMP_STACK", OCL_OMP_STACK],
            [ "OCL_OMP_HEAP", OCL_OMP_HEAP ]
        ],
        codeMemory:  "DSP1_PROG",
        dataMemory:  "DSP1_PROG",
        stackMemory: "L2SRAM",
        l1DMode: "32k",
        l1PMode: "32k",
        l2Mode: "128k"
    };

    readonly config Any DSP2 = {
        customMemoryMap: [
            [ "L1DSRAM", L1DSRAM ],
            [ "L2SRAM", L2SRAM ],
            [ "OCL_LOCAL", OCL_LOCAL ],
            [ "DSP2_PROG", DSP2_PROG ],
            [ "SR_0", SR_0 ],
            [ "OCL_OMP_NOCACHE", OCL_OMP_NOCACHE ],
            [ "OCL_OMP_STACK", OCL_OMP_STACK],
            [ "OCL_OMP_HEAP", OCL_OMP_HEAP ]
        ],
        codeMemory:  "DSP2_PROG",
        dataMemory:  "DSP2_PROG",
        stackMemory: "L2SRAM",
        l1DMode: "32k",
        l1PMode: "32k",
        l2Mode: "128k"
    };

    readonly config Any EVE1 = {
        externalMemoryMap: [
            [ "EVEVECS", { /* name used by SYS/BIOS */
                name: "EVEVECS", space: "code/data", access: "RWX",
                base: 0x80000000, len: 0x100, page: 0,
                comment: "EVE1 Vector Table (256 B)"
            }],
            [ "EVE1_PROG", {
                name: "EVE1_PROG", space: "code/data", access: "RWX",
                base: 0x81000000, len: 0x400000, page: 1,
                comment: "EVE1 Program Memory (4 MB)"
            }],
            [ "SR_0", SR_0 ]
        ],
        codeMemory:  "EVE1_PROG",
        dataMemory:  "EVE1_PROG",
        stackMemory: "EVE1_PROG"
    };

    readonly config Any EVE2 = {
        externalMemoryMap: [
            [ "EVEVECS", { /* name used by SYS/BIOS */
                name: "EVEVECS", space: "code/data", access: "RWX",
                base: 0x80010000, len: 0x100, page: 0,
                comment: "EVE2 Vector Table (256 B)"
            }],
            [ "EVE2_PROG", {
                name: "EVE2_PROG", space: "code/data", access: "RWX",
                base: 0x82000000, len: 0x400000, page: 1,
                comment: "EVE2 Program Memory (4 MB)"
            }],
            [ "SR_0", SR_0 ]
        ],
        codeMemory:  "EVE2_PROG",
        dataMemory:  "EVE2_PROG",
        stackMemory: "EVE2_PROG"
    };

    readonly config Any EVE3 = {
        externalMemoryMap: [
            [ "EVEVECS", { /* name used by SYS/BIOS */
                name: "EVEVECS", space: "code/data", access: "RWX",
                base: 0x80020000, len: 0x100, page: 0,
                comment: "EVE3 Vector Table (256 B)"
            }],
            [ "EVE3_PROG", {
                name: "EVE3_PROG", space: "code/data", access: "RWX",
                base: 0x83000000, len: 0x400000, page: 1,
                comment: "EVE3 Program Memory (4 MB)"
            }],
            [ "SR_0", SR_0 ]
        ],
        codeMemory:  "EVE3_PROG",
        dataMemory:  "EVE3_PROG",
        stackMemory: "EVE3_PROG"
    };

    readonly config Any EVE4 = {
        externalMemoryMap: [
            [ "EVEVECS", { /* name used by SYS/BIOS */
                name: "EVEVECS", space: "code/data", access: "RWX",
                base: 0x80030000, len: 0x100, page: 0,
                comment: "EVE4 Vector Table (256 B)"
            }],
            [ "EVE4_PROG", {
                name: "EVE4_PROG", space: "code/data", access: "RWX",
                base: 0x84000000, len: 0x400000, page: 1,
                comment: "EVE4 Program Memory (4 MB)"
            }],
            [ "SR_0", SR_0 ]
        ],
        codeMemory:  "EVE4_PROG",
        dataMemory:  "EVE4_PROG",
        stackMemory: "EVE4_PROG"
    };

    readonly config Any IPU1 = {
        externalMemoryMap: [
            [ "IPU1_PROG", {
                name: "IPU1_PROG", space: "code/data", access: "RWX",
                base: 0x8A000000, len: 0x800000,
                comment: "IPU1 Program Memory (8 MB)"
            }],
            [ "SR_0", SR_0 ]
        ],
        codeMemory:  "IPU1_PROG",
        dataMemory:  "IPU1_PROG",
        stackMemory: "IPU1_PROG"
    };

    readonly config Any IPU2 = {
        externalMemoryMap: [
            [ "IPU2_PROG", {
                name: "IPU2_PROG", space: "code/data", access: "RWX",
                base: 0x8A800000, len: 0x800000,
                comment: "IPU2 Program Memory (8 MB)"
            }],
            [ "SR_0", SR_0 ]
        ],
        codeMemory:  "IPU2_PROG",
        dataMemory:  "IPU2_PROG",
        stackMemory: "IPU2_PROG"
    };

    readonly config Any HOST = {
        customMemoryMap: [
            [ "HOST_PROG", HOST_PROG ],
            [ "SR_0", SR_0 ],
            [ "OCL_GLOBAL", OCL_GLOBAL ]
        ],
        codeMemory:  "HOST_PROG",
        dataMemory:  "HOST_PROG",
        stackMemory: "HOST_PROG"
    };

instance:

    /*!
     *  ======== externalMemoryMap ========
     *  Memory regions as defined in the DRA7XX Specification
     */
    override readonly config xdc.platform.IPlatform.Memory
        externalMemoryMap[string] = [
            ["EXT_RAM", {
                comment: "2 GB External RAM Memory",
                name: "EXT_RAM",
                base: 0x80000000,
                len:  0x80000000
            }]
        ];

    /*
     *  ======== l1PMode ========
     *  Define the amount of L1P RAM used for L1 Program Cache.
     *
     *  Check the device documentation for valid values.
     */
    config String l1PMode = "32k";

    /*
     *  ======== l1DMode ========
     *  Define the amount of L1D RAM used for L1 Data Cache.
     *
     *  Check the device documentation for valid values.
     */
    config String l1DMode = "32k";

    /*
     *  ======== l2Mode ========
     *  Define the amount of L2 RAM used for L2 Cache.
     *
     *  Check the device documentation for valid values.
     */
    config String l2Mode = "128k";
};
/*
 *  @(#) ti.platforms.evmDRA7XX; 1, 0, 0, 0,; 1-29-2016 10:02:10; /db/ztree/library/trees/platform/platform-q17/src/
 */

