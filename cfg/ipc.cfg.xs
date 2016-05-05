/*
 * Copyright (c) 2012-2013, Texas Instruments Incorporated
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
//var procNameAry = MultiProc.getDeviceProcNames();
var procNameAry = [ "DSP1","HOST" ]; //[ "DSP", "VIDEO-M3", "HOST" ];
//var procNameAry = cfgArgs.procList.toUpperCase().split(/\s+/);

var MultiProc = xdc.useModule('ti.sdo.utils.MultiProc');
MultiProc.numProcessors = 2;
MultiProc.setConfig(Program.global.procName, procNameAry);

/* ipc configuration */
var Ipc = xdc.useModule('ti.sdo.ipc.Ipc');
Ipc.procSync = Ipc.ProcSync_ALL; /* synchronize all processors in Ipc_start */
Ipc.sr0MemorySetup = true;

/* shared region configuration */
var SharedRegion = xdc.useModule('ti.sdo.ipc.SharedRegion');

/* configure SharedRegion #0 (IPC) */
var SR0Mem = Program.cpu.memoryMap["SR_0"];

SharedRegion.setEntryMeta(0,
    new SharedRegion.Entry({
        name:           "SR_0",
        base:           SR0Mem.base,
        len:            SR0Mem.len,
        ownerProcId:    0,
        isValid:        true,
        cacheEnable:    true
    })
);

/* reduce data memory usage */
var GateMP = xdc.useModule('ti.sdo.ipc.GateMP');
GateMP.maxRuntimeEntries = 2;
GateMP.RemoteCustom1Proxy = xdc.useModule('ti.sdo.ipc.gates.GateMPSupportNull');

/* reduce data memory usage */
var Notify = xdc.useModule('ti.sdo.ipc.Notify');
Notify.numEvents = 8;


var NotifySetup = xdc.useModule('ti.sdo.ipc.family.vayu.NotifySetup');
if(procName != "DSP1")
{
NotifySetup.connections.$add(
      new NotifySetup.Connection({
          driver: NotifySetup.Driver_SHAREDMEMORY,
          procName: "DSP1"
      })
);
}
if(procName != "HOST")
{
NotifySetup.connections.$add(
      new NotifySetup.Connection({
          driver: NotifySetup.Driver_SHAREDMEMORY,
          procName: "HOST"
      })
);
}
var NotifyDriverMbx = xdc.useModule('ti.sdo.ipc.family.vayu.NotifyDriverMbx');
