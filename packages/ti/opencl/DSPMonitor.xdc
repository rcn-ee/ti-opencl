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

package ti.opencl;

import xdc.runtime.Types;

/*!
 *  ======== DSPMonitor ========
 *  DSPMonitor RTSC Configuration Module
 *
 *  This module must be used (via xdc.useModule) by all DSPMonitor RTSC
 *  applications. This module is used to setup a configuration for the
 *  runtime.
 *
 */

module DSPMonitor
{
    /*!
     *  ======== OCL_monitor_priority ========
     *
     *  Priority of OpenCL DSP monitor task.
     */
    config UInt32   OCL_monitor_priority   = 3;

    /*!
     *  ======== OCL_ipc_customized ========
     *
     *  Flag for ipc customization.  If set to true, then user must provide
     *  its own ipc configuration.
     */
    config Bool     OCL_ipc_customized     = false;

    /*!
     *  ======== OCL_link_extra_sym_def ========
     *
     *  Flag for linking in extra sym def.  See package.xs:getLibs().
     */
    config Bool     OCL_link_extra_sym_def = false;

    /*!
     *  ======== OCL_memory_customized ========
     *
     *  Flag for memory customization.  If set to true, then the memory
     *  sections below must be given valid values, otherwise, default
     *  platform values are used.  memory sections: TBD
     *  If customizing memory, user must set cachability explicitly in the
     *  xdc/xs config file, ddr memory used for OpenCL global buffers must be
     *  set to write through. (see ti.sysbios.family.c66/Cache.xdc:setMarMeta)
     */
    config Bool     OCL_memory_customized  = false;

    /*!
     *  ======== OCL_SR0_base ========
     *  ======== OCL_SR0_len ========
     *
     *  IPC Shared Region 0.
     */
    config UInt32   OCL_SR0_base           = 0;
    config UInt32   OCL_SR0_len            = 0;
}

