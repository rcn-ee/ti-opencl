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

#include "device_manager_mpm.h"
#include "../error_report.h"

extern "C"
{
   #include "mpmclient.h"
};

using namespace tiocl;

bool DeviceManagerMPM::Reset() const
{
    for (uint8_t core=0; core < num_cores_; core++)
    {
        char curr_core[10];
        snprintf(curr_core, 5, "dsp%d", core);

        int error_code = 0;
        int ret = mpm_reset(curr_core, &error_code);
        if (ret < 0)
            ReportError(ErrorType::Fatal, ErrorKind::DeviceResetFailed, core, error_code);
    }

    return true;
}


bool DeviceManagerMPM::Load() const
{
    for (uint8_t core=0; core < num_cores_; core++)
    {
        char curr_core[10];
        snprintf(curr_core, 5,"dsp%d", core);

        char curr_binary[10];
        snprintf(curr_binary, 9, "dsp%d.out", core);

        std::string dspbin = monitor_;
        size_t pos = dspbin.find("dsp.out");
        dspbin.replace(pos, 7, const_cast<char*>(curr_binary));

        int error_code = 0;
        int ret = mpm_load(curr_core, const_cast<char*>(dspbin.c_str()),
                       &error_code);
        if (ret < 0)
            ReportError(ErrorType::Fatal, ErrorKind::DeviceLoadFailed,
                        dspbin.c_str(), core, error_code);
    }


}

bool DeviceManagerMPM::Run() const
{
    for (uint8_t core=0; core < num_cores_; core++)
    {
        char curr_core[10];
        snprintf(curr_core, 5,"dsp%d", core);

        int error_code = 0;
        int ret = mpm_run(curr_core, &error_code);
        if (ret < 0)
            ReportError(ErrorType::Fatal, ErrorKind::DeviceRunFailed,
                        core, error_code);
    }


}

const DeviceManager*
tiocl::DeviceManagerFactory::CreateDeviceManager(uint8_t device_id, uint8_t num_cores,
                                                 const std::string &binary)
{
    return new DeviceManagerMPM(num_cores, binary);
}
