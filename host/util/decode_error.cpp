/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
const char* ocl_decode_error(int code)
{
    switch(code)
    {
        case 0:   return "Success";
        case -1:  return "Device not found";
        case -2:  return "Device not available";
        case -3:  return "Compiler not available";
        case -4:  return "Mem object allocation failure";
        case -5:  return "Out of resources";
        case -6:  return "Out of host memory";
        case -7:  return "Profiling info not available";
        case -8:  return "Mem copy overlap";
        case -9:  return "Image format mismatch";
        case -10: return "Image format not supported";
        case -11: return "Build program failure";
        case -12: return "Map failure";
        case -13: return "Misaligned sub buffer offset";
        case -14: return "Exec status error for events in wait list";
        case -30: return "Invalid value";
        case -31: return "Invalid device type";
        case -32: return "Invalid platform";
        case -33: return "Invalid device";
        case -34: return "Invalid context";
        case -35: return "Invalid queue properties";
        case -36: return "Invalid command queue";
        case -37: return "Invalid host ptr";
        case -38: return "Invalid mem object";
        case -39: return "Invalid image format descriptor";
        case -40: return "Invalid image size";
        case -41: return "Invalid sampler";
        case -42: return "Invalid binary";
        case -43: return "Invalid build options";
        case -44: return "Invalid program";
        case -45: return "Invalid program executable";
        case -46: return "Invalid kernel name";
        case -47: return "Invalid kernel definition";
        case -48: return "Invalid kernel";
        case -49: return "Invalid arg index";
        case -50: return "Invalid arg value";
        case -51: return "Invalid arg size";
        case -52: return "Invalid kernel args";
        case -53: return "Invalid work dimension";
        case -54: return "Invalid work group size";
        case -55: return "Invalid work item size";
        case -56: return "Invalid global offset";
        case -57: return "Invalid event wait list";
        case -58: return "Invalid event";
        case -59: return "Invalid operation";
        case -60: return "Invalid gl object";
        case -61: return "Invalid buffer size";
        case -62: return "Invalid mip level";
        case -63: return "Invalid global work size";
        case -64: return "Invalid property";

        case -7101: return "Kernel aborted";
        case -7102: return "Kernel exited";
        case -7103: return "Kernel timed out";

        default:  return "Unknown";
    }
}
