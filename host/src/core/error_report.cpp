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
#include <cstdio>
#include <cstdlib>
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
     "OpenCL needs at least one CMEM block in off-chip DDR to operate,"
     " plus optional CMEM block in on-chip (MSMC SRAM or OCMC RAM) memory."},

    {ErrorKind::CMEMInvalidBlockId,
     "Invalid CMEM block id, %d, is specified."},

    {ErrorKind::CMEMMapFailed,
     "Cannot map CMEM physical memory (0x%llx, %lld MB) into the Host virtual address space.\n"
     "       This is typically due to Linux system memory being near capacity."},

    {ErrorKind::CMEMAllocFailed,
     "Unable to allocate %s CMEM from 0x%llx"},

    {ErrorKind::CMEMAllocFromBlockFailed,
     "Failed to allocate 0x%llx from CMEM block %d, allocated 0x%llx"},

    {ErrorKind::InvalidPointerToClFree,
     "Invalid pointer %p to clFree"},

    {ErrorKind::InvalidPointerToFree,
     "Invalid pointer %p to %s"},

    {ErrorKind::ELFLibraryInitFailed,
     "Failed to initialize DSP binary symbol reader"},

    {ErrorKind::ELFSymbolAddressNotCached,
     "Internal Error: Symbol %s not found in symbol cache."},

    {ErrorKind::DeviceResetFailed,
     "Internal Error: Failed to reset device, DSP core %d, error code %d"},

    {ErrorKind::DeviceLoadFailed,
     "Internal Error: Failed to load monitor %s on DSP core %d, error code %d"},

    {ErrorKind::DeviceRunFailed,
     "Internal Error: Failed to run monitor on DSP core %d, error code %d"},

    {ErrorKind::NumComputeUnitDetectionFailed,
     "Internal Error: Failed to determine number of compute units."},

    {ErrorKind::DSPMonitorPathNonExistent,
     "Internal Error: Path to DSP monitor binary (%s) does not exist"},

    {ErrorKind::TiOclInstallNotSpecified,
     "Set TI_OCL_INSTALL environment variable to location of OpenCL"
     " installation. Refer User Guide for details"},

    {ErrorKind::MailboxCreationFailed,
     "Internal Error: Failed to initialize mailbox"},

    {ErrorKind::ShouldNotGetHere,
     "Internal Error: %s, %d"},

    {ErrorKind::PCIeDriverError,
     "Internal Error: Error in PCIe driver"},

    {ErrorKind::MessageQueueCountMismatch,
     "Internal Error: Number of message queues (%d) does not match number of compute units (%d)"},

    {ErrorKind::LostDSP,
     "Communication to a DSP has been lost (likely due to an MMU fault).%s"},

    {ErrorKind::DaemonNotRunning,
     "The TI Multicore Tools daemon (/usr/bin/ti-mctd) is not running. To start daemon, rm /dev/shm/HeapManager (if exists); ti-mctd. Re-run application. Refer User Guide for details."},

    {ErrorKind::DaemonAlreadyRunning,
     "The TI Multicore Tools daemon (/usr/bin/ti-mctd) is already running. If a restart is needed, pkill ti-mctd; rm /dev/shm/HeapManager (if exists); ti-mctd. Refer User Guide for details."},

    {ErrorKind::DaemonConfigOpenError,
     "Cannot parse mctd config file /etc/ti-mctd/ti_mctd_config.json. It is either missing or incorrect. Please check."},

    {ErrorKind::InfoMessage2,
     "%s%s."},
};

void tiocl::ReportError(const ErrorType et, const ErrorKind ek, ...)
{
    std::string error_format;

    switch (et)
    {
        case ErrorType::Abort:
        case ErrorType::Fatal:
        case ErrorType::FatalNoExit:
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
    vfprintf  (stderr, error_format.c_str(), args);
    va_end   (args);

    if (et == ErrorType::Fatal)
        exit(EXIT_FAILURE);
    else if (et == ErrorType::Abort)
        abort();
}

#if defined(TRACE_ENABLED)
#include "../tiocl_thread.h"
void tiocl::ReportTrace(const char *fmt, ...)
{
    std::string trace_fmt = "TIOCL Trace: (";
    trace_fmt += std::to_string(pthread_self());
    trace_fmt += ") ";
    trace_fmt += fmt;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, trace_fmt.c_str(), ap);
    va_end(ap);
    std::fflush(stdout);
}
#endif
