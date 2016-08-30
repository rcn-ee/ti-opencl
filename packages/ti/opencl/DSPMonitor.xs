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
 * --/COPYRIGHT--*/

var DSPMonitor        = null;


/*
 *  ======== module$meta$init ========
 */
function module$meta$init()
{
    /* Only process during "cfg" phase */
    if (xdc.om.$name != "cfg") {
        return;
    }

    DSPMonitor = this;
}

/*
 *  ======== module$use ========
 */
function module$use()
{
    var BIOS = xdc.useModule("ti.sysbios.BIOS");     // for linking ordering

    if (DSPMonitor.OCL_ipc_customized == false)
        var ipc_cfg = xdc.loadCapsule("ti/opencl/ipc.cfg.xs");
    else
        var Ipc  = xdc.useModule("ti.sdo.ipc.Ipc");  // for linking ordering

    if (DSPMonitor.OCL_memory_customized == false)
    {
        var Program = xdc.useModule("xdc.cfg.Program");
        DSPMonitor.OCL_SR0_base = Program.cpu.memoryMap["SR_0"].base;
        DSPMonitor.OCL_SR0_len  = Program.cpu.memoryMap["SR_0"].len;
    }
}


/*
 *  ======== module$static$init ========
 */
function module$static$init(mod, params)
{
}



/*
 *  ======== module$validate ========
 */
function module$validate()
{
}
