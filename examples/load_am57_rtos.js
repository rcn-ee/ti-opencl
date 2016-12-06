/******************************************************************************
 * Copyright (c) 2013-2016, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

/** Arguments to this DSS script (Debug Server Scripting):
 * 0: CCS_TARGET_CONFIG
 * 1: ARM  exectuable
 * 2: DSP1 exectuable
 * 3: DSP2 exectuable
 * 4: Verbose output: 0 for off, 1 for on
**/
if (arguments.length < 5) {
    print("Usage: dss.sh load_am57_rtos.js CCS_target_config arm_exe dsp1_exe dsp2_exe verbose_output");
    print("       set verbose_output to 1 or 0");
    java.lang.System.exit(0);
}

var CCS_TARGET_CONFIG = arguments[0];
var ARM_BINARY        = arguments[1];
var DSP1_OUT          = arguments[2];
var DSP2_OUT          = arguments[3];
var VERBOSE_OUTPUT    = arguments[4];


// Check to see if running from within CCSv4 Scripting Console
var ds;
var withinCCS = (ds !== undefined);
 
// Create scripting environment and get debug server if running standalone
if (!withinCCS)
{
    // Import the DSS packages into our namespace to save on typing
    importPackage(Packages.com.ti.debug.engine.scripting);
    importPackage(Packages.com.ti.ccstudio.scripting.environment);
    importPackage(Packages.java.lang);
 
    // Create our scripting environment object - which is the main entry point
    // into any script and the factory for creating other Scriptable servers
    // and Sessions
    var script = ScriptingEnvironment.instance();
 
    // Get the Debug Server and start a Debug Session
    debugServer = script.getServer("DebugServer.1");
}
else // otherwise leverage existing scripting environment and debug server
{
    var debugServer = ds;
    var script = env;
}

debugServer.setConfig(CCS_TARGET_CONFIG);
script.traceBegin("autoRunLog.xml", "DefaultStylesheet.xsl");
script.setCurrentDirectory("/");
if (VERBOSE_OUTPUT !== "1")  script.traceSetConsoleLevel(TraceLevel.OFF);
else                         script.traceSetConsoleLevel(TraceLevel.INFO);
//script.setScriptTimeout(15000);
debugSession = [];


openSessions();
connect();
waitInMs(1000);
loadPrograms();
waitInMs(500);
runPrograms();
haltPrograms();
disconnect();
terminateSessions();
debugServer.stop();
script.traceEnd();
java.lang.System.exit(0);


/*********************************************************************/
/* Script functions                                                  */
/*********************************************************************/
function printTrace(string)
{
    script.traceWrite(string);  // goes to both log and console
    //print(string);            // goes only to console
}

function openSessions()
{
    printTrace("Opening sessions ...");
    try
    {
        debugSession[0] = debugServer.openSession("Spectrum Digital XDS560V2 STM LAN Emulator_0/CortexA15_0");
        debugSession[1] = debugServer.openSession("Spectrum Digital XDS560V2 STM LAN Emulator_0/C66xx_DSP1");
        debugSession[2] = debugServer.openSession("Spectrum Digital XDS560V2 STM LAN Emulator_0/C66xx_DSP2");
    }
    catch (e) 
    {
        print("ERROR: Couldn't open session on cpu: " + e);
        java.lang.System.exit(-1);
    }
}

function terminateSessions()
{
    debugSession[2].terminate();
    debugSession[1].terminate();
    debugSession[0].terminate();
}

function waitInMs(ms)
{
    var exitTime = (new Date()).getTime() + ms;
    while((new Date()).getTime() < exitTime){};
}

function reset()
{
    printTrace("Resetting target...");

    try
    {
        for (var core = 0; core < 3; ++core)
        {
            debugSession[core].target.reset();
        }
    }
    catch (ex)
    {
        print("Could reset target!\nAborting!" + ex);
    }
}

function connect()
{
    printTrace("TARGET: " + debugSession[0].getBoardName());
    printTrace("Performing a System Reset...");

    // Connect to Cortex15_0
    if (!debugSession[0].target.isConnected())
    {
        debugSession[0].target.connect();
    }
 
    // Array for supported resets
    var reset = new Array();

    // Query the number of supported resets
    var numReset = debugSession[0].target.getNumResetTypes();

    // Look for system reset and issue it
    for (i=0;i<numReset;i++)
    {
         reset[i]=debugSession[0].target.getResetType(i);  

         // Issue a system reset
         if (reset[i].getName().indexOf("System Reset") != -1)
         {
            printTrace(reset[i].getName());
            reset[i].issueReset();    
         }
    }
    debugSession[0].target.disconnect();

    try
    {
        if (!debugSession[0].target.isConnected())
        {
            printTrace("Connect to A15_0...");
            debugSession[0].target.connect();
        }
        
        if (!debugSession[1].target.isConnected())
        {
            printTrace("Enabling DSP1...");
            debugSession[1].target.connect();
        }
        if (!debugSession[2].target.isConnected())
        {
            printTrace("Enabling DSP2...");
            debugSession[2].target.connect();
        }
    }
    catch (e)
    {
        print("Could not connect to target!" + e);
    }

    // set clock to be consistent with BIOS.cpuFreq.lo in Host.cfg
    try
    {
        debugSession[0].expression.evaluate("AM572x_PRCM_Clock_Config_OPPHIGH()");
    }
    catch (e)
    {
        print("Could not set target clock!" + e);
    }
}

function disconnect()
{
    debugSession[2].target.disconnect();
    debugSession[1].target.disconnect();
    debugSession[0].target.disconnect();
}

function loadPrograms()
{
    printTrace("Loading programs: ARM ...");
    debugSession[0].memory.loadProgram(ARM_BINARY);
    waitInMs(500);
    printTrace("Loading programs: DSP1 ...");
    debugSession[1].memory.loadProgram(DSP1_OUT);
    waitInMs(500);
    printTrace("Loading programs: DSP2 ...");
    debugSession[2].memory.loadProgram(DSP2_OUT);
}

function runPrograms()
{
    printTrace("Target running...");
    script.traceSetConsoleLevel(TraceLevel.INFO);
    debugServer.simultaneous.run(debugSession);
    //for (var core = 0; core < 3; ++core)
    //{
    //    debugSession[core].target.run();
    //}
    if (VERBOSE_OUTPUT !== "1")  script.traceSetConsoleLevel(TraceLevel.OFF);
}

function haltPrograms()
{
    printTrace("Halting DSPs...");
    debugSession[0].target.halt();
    debugSession[1].target.halt();
    debugSession[2].target.halt();
}


/**
 * Get error code from the given exception.
 * @param {exception} The exception from which to get the error code.
 */
function getErrorCode(exception)
{
        var ex2 = exception.javaException;
        if (ex2 instanceof Packages.com.ti.ccstudio.scripting.environment.ScriptingException) {
                return ex2.getErrorID();
        }
        return 0;
}

