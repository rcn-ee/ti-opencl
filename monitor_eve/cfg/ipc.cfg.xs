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
 *  ======== ipc.cfg.xs ========
 */
var Program = xdc.useModule('xdc.cfg.Program');
var cfgArgs = Program.build.cfgArgs;

/* configure processor names */
var procNameAry = ["HOST", "IPU2", "IPU1", "DSP2", "DSP1",
                   "EVE1", "EVE2", "EVE3", "EVE4"];
var ownerSr0 = "IPU1";

var MultiProc = xdc.useModule('ti.sdo.utils.MultiProc');
MultiProc.setConfig(Program.global.procName, procNameAry);

/* shared region configuration */
var SharedRegion = xdc.useModule('ti.sdo.ipc.SharedRegion');
SharedRegion.translate = false;  // if no translation needed, faster access

/* configure SharedRegion #0 (IPC) */
var SR0Mem = Program.cpu.memoryMap["SR0"];

SharedRegion.setEntryMeta(0,
    new SharedRegion.Entry({
        base:           SR0Mem.base,
        len:            SR0Mem.len,
        ownerProcId:    MultiProc.getIdMeta(ownerSr0),
        isValid:        true,
        cacheEnable:    true,
        createHeap:     true,
        name:           "SR0"
    })
);

var Ipc = xdc.useModule('ti.sdo.ipc.Ipc');
Ipc.procSync = Ipc.ProcSync_PAIR;
Ipc.hostProcId = MultiProc.getIdMeta("IPU1");

/* IPU1 */
Ipc.setEntryMeta({
    remoteProcId: MultiProc.getIdMeta("IPU1"),
    setupNotify: true,
    setupMessageQ: true
});
/* EVE1 */
Ipc.setEntryMeta({
    remoteProcId: MultiProc.getIdMeta("EVE1"),
    setupNotify: true,
    setupMessageQ: true
});
/* EVE2 */
Ipc.setEntryMeta({
    remoteProcId: MultiProc.getIdMeta("EVE2"),
    setupNotify: true,
    setupMessageQ: true
});
/* EVE3 */
Ipc.setEntryMeta({
    remoteProcId: MultiProc.getIdMeta("EVE3"),
    setupNotify: true,
    setupMessageQ: true
});
/* EVE4 */
Ipc.setEntryMeta({
    remoteProcId: MultiProc.getIdMeta("EVE4"),
    setupNotify: true,
    setupMessageQ: true
});

var Notify      = xdc.useModule('ti.sdo.ipc.Notify');
var NotifySetup = xdc.useModule('ti.sdo.ipc.family.vayu.NotifySetup');
var core = environment["CORE"];
if (core == "IPU1")
{
    NotifySetup.connections.$add(
        new NotifySetup.Connection({
        driver: NotifySetup.Driver_MAILBOX,
        procName: "EVE1"
        })
    );
    NotifySetup.connections.$add(
        new NotifySetup.Connection({
        driver: NotifySetup.Driver_MAILBOX,
        procName: "EVE2"
        })
    );
    NotifySetup.connections.$add(
        new NotifySetup.Connection({
        driver: NotifySetup.Driver_MAILBOX,
        procName: "EVE3"
        })
    );
    NotifySetup.connections.$add(
        new NotifySetup.Connection({
        driver: NotifySetup.Driver_MAILBOX,
        procName: "EVE4"
        })
    );
}
if (core != "IPU1")
{
    NotifySetup.connections.$add(
        new NotifySetup.Connection({
        driver: NotifySetup.Driver_MAILBOX,
        procName: "IPU1"
        })
    );
}
