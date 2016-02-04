/******************************************************************************
 * Copyright (c) 2016, Texas Instruments Incorporated - http://www.ti.com/
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Texas Instruments Incorporated nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "error_report.h"
#include <cstdarg>
#include <string>
#include <map>

using namespace tiocl;

static const std::map<const ErrorKind, const std::string> ErrorStrings =
{
    {ErrorKind::PageSizeNotAvailable,
     "Failed to get _SC_PAGE_SIZE" },

    {ErrorKind::RegionAddressNotMultipleOfPageSize,
     "Region address %08llx is not a multiple of page size (%d)"},

    {ErrorKind::RegionSizeNotMultipleOfPageSize,
     "Region size is not a multiple of page size"},

    {ErrorKind::FailedToOpenFileName,
     "Failed to open file %s"},

    {ErrorKind::TranslateAddressOutsideMappedAddressRange,
     "Attempt to translate an address 0x%llx outside the mapped range"},

    {ErrorKind::UnableToMapDSPAddress,
     "Unable to map DSP address %s"},

    {ErrorKind::IllegalMemoryRegion,
     "No memory region in OpenCL runtime corresponding to 0x%llx"},

    {ErrorKind::CMEMInitFailed,
     "The cmemk kernel module is not installed. Consult the OpenCL User"
     "Guide at http://software-dl.ti.com/mctools/esd/docs/opencl/index.html"},

    {ErrorKind::CMEMMinBlocks,
     "OpenCL needs %d CMEM blocks to operate - one each for off-chip (DDR)"
     " and on-chip (MSMC SRAM or OCMC RAM) memory."},

    {ErrorKind::CMEMMapFailed,
     "Cannot map CMEM physical memory (0x%llx, %lld MB) into the Host virtual address space.\n"
     "       This is typically due to Linux system memory being near capacity."},

    {ErrorKind::CMEMAllocFailed,
     "Unable to allocate %s CMEM from 0x%llx"},

    {ErrorKind::CMEMAllocFromBlockFailed,
     "Failed to allocate 0x%llx from CMEM block %d, allocated 0x%llx"},

    {ErrorKind::InvalidPointerToClFree,
     "Invalid pointer %p to clFree"}
};

void tiocl::ReportError(const ErrorType et, const ErrorKind ek, ...)
{
    std::string error_format;

    switch (et)
    {
        case ErrorType::Fatal:
            error_format = "TIOCL FATAL: "; break;
        case ErrorType::Warning:
            error_format = "TIOCL WARNING: "; break;
        default:
            error_format = "TIOCL: "; break;
    }

    error_format += ErrorStrings.at(ek);
    error_format += '\n';

    va_list args;
    va_start (args, ek);
    vprintf  (error_format.c_str(), args);
    va_end   (args);

    if (et == ErrorType::Fatal)
        exit(EXIT_FAILURE);
}

#if defined(TRACE_ENABLED)
void tiocl::ReportTrace(const char *fmt, ...)
{
    std::string trace_fmt = "TIOCL Trace: ";
    trace_fmt += fmt;
    va_list ap;
    va_start(ap, fmt);
    std::vprintf(trace_fmt.c_str(), ap);
    va_end(ap);
    std::fflush(stdout);
}
#endif
