/*
 * Copyright (c) 2016, Texas Instruments Incorporated
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
 *
 */
var Program = xdc.useModule('xdc.cfg.Program');
var cfgArgs = Program.build.cfgArgs;

/* configure processor names */
var procNameAry = ["HOST",
                   "DSP1","DSP2","EVE1","EVE2","EVE3","EVE4","IPU1","IPU2"];

var MultiProc = xdc.useModule('ti.sdo.utils.MultiProc');
MultiProc.setConfig(xdc.global.oclProcName, procNameAry);

/* ipc configuration */
var Ipc = xdc.useModule('ti.sdo.ipc.Ipc');
Ipc.procSync = Ipc.ProcSync_PAIR; /* need to call Ipc_attach to synchronize */
Ipc.sr0MemorySetup = true;

/* shared region configuration */
var SharedRegion = xdc.useModule('ti.sdo.ipc.SharedRegion');
SharedRegion.translate = false;

/* configure SharedRegion #0 (IPC) */
var SR0Mem = Program.cpu.memoryMap["SR_0"];

SharedRegion.setEntryMeta(0,
    new SharedRegion.Entry({
        name:           "SR_0",
        base:           SR0Mem.base,
        len:            SR0Mem.len,
        cacheLineSize:  128,
        ownerProcId:    MultiProc.getIdMeta("HOST"),
        isValid:        true,
        createHeap:     true,
        cacheEnable:    xdc.global.SR0_cacheEnable
    })
);

var GateMP = xdc.useModule('ti.sdo.ipc.GateMP');
GateMP.maxRuntimeEntries = 9;
var Notify = xdc.useModule('ti.sdo.ipc.Notify');
Notify.numEvents = 8;

var NotifySetup = xdc.useModule('ti.sdo.ipc.family.vayu.NotifySetup');
if (xdc.global.oclProcName == "HOST")
{
    NotifySetup.connections.$add(
          new NotifySetup.Connection({
              driver: NotifySetup.Driver_SHAREDMEMORY,
              procName: "DSP1"
          })
    );
    NotifySetup.connections.$add(
          new NotifySetup.Connection({
              driver: NotifySetup.Driver_SHAREDMEMORY,
              procName: "DSP2"
          })
    );
}
if (xdc.global.oclProcName == "DSP1" || xdc.global.oclProcName == "DSP2")
{
    NotifySetup.connections.$add(
          new NotifySetup.Connection({
              driver: NotifySetup.Driver_SHAREDMEMORY,
              procName: "HOST"
          })
    );
}

var NotifyDriverMbx = xdc.useModule('ti.sdo.ipc.family.vayu.NotifyDriverMbx');
