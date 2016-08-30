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
 *  ======== getSects ========
 */
function getSects()
{
    if (String(xdc.global.oclProcName).search("DSP") != -1)
        return "ti/opencl/linkcmd_dsp.xdt";
}

/*
 *  ======== Package.getLibs ========
 *  This function is called when a program's configuration files are
 *  being generated and it returns the name of a library appropriate
 *  for the program's configuration.
 */
function getLibs(prog)
{
    var pkg = this;
    var suffix = prog.build.target.findSuffix(pkg);

    var lib = "";

    if (String(Program.cpu.deviceName).search("DRA7XX") != -1 &&
        String(xdc.global.oclProcName).search("HOST") != -1)
    {
        var ocl_lib = "usr/lib/libOpenCL.a";

        /* If library exists, return it. If not, throw exception */
        if (java.io.File(this.packageBase + ocl_lib).exists())
            lib += ocl_lib;
        else
            throw new Error("Library not found: " + ocl_lib);

        var ocl_util_lib = "usr/lib/libocl_util.a";
        if (java.io.File(this.packageBase + ocl_util_lib).exists())
            lib += ";" + ocl_util_lib;
        else
            throw new Error("Library not found: " + ocl_lib);
    }
    else if (String(Program.cpu.deviceName).search("DRA7XX") != -1 &&
             String(xdc.global.oclProcName).search("DSP") != -1)
    {
        var dsp_monitor_lib = "usr/share/ti/opencl/libDSPMonitor.ae66";
        var dsp_monitor_lib_build = "../../../monitor_vayu/libDSPMonitor.ae66";
        if (java.io.File(this.packageBase + dsp_monitor_lib).exists())
            lib += dsp_monitor_lib;
        else if (java.io.File(this.packageBase+dsp_monitor_lib_build).exists())
            lib += dsp_monitor_lib_build;
        else
            throw new Error("Library not found: " + dsp_monitor_lib);

        var builtins_lib = "usr/share/ti/opencl/dsp.lib";
        var builtins_lib_build = "../../../builtins/dsp.lib";
        if (java.io.File(this.packageBase + builtins_lib).exists())
            lib += ";" + builtins_lib;
        else if (java.io.File(this.packageBase + builtins_lib_build).exists())
            lib += ";" + builtins_lib_build;
        else
            throw new Error("Library not found: " + builtins_lib);

        var libm_lib = "usr/share/ti/opencl/libm.lib";
        var libm_lib_build = "../../../libm/libm.lib";
        if (java.io.File(this.packageBase + libm_lib).exists())
            lib += ";" + libm_lib;
        else if (java.io.File(this.packageBase + libm_lib_build).exists())
            lib += ";" + libm_lib_build;
        else
            throw new Error("Library not found: " + libm_lib);

        DSPMonitor = xdc.module("ti.opencl.DSPMonitor");
        if (DSPMonitor.OCL_link_extra_sym_def == true)
        {
            var sym_lib = "usr/share/ti/opencl/sym.def";
            if (java.io.File(this.packageBase + sym_lib).exists())
                lib += ";" + sym_lib;
            else
                throw new Error("Library not found: " + sym_lib);
        }
    }
    else
    {
        Program.$logError("Device:" + Program.cpu.deviceName +
                          " oclProcName:" + xdc.global.oclProcName +
                          " not supported", this);
    }

    configuration_lib = "lib/" + pkg.$name + ".a" + suffix;
    if (java.io.File(this.packageBase + configuration_lib).exists())
        lib += ";" + configuration_lib;
    else
        throw new Error("Library not found: " + configuration_lib);

    return lib;
}

/* Set up flags in preparation for calls to getLibs() in OEM/OMP */
function close()
{
}

