/******************************************************************************
 * Copyright (c) 2016, Texas Instruments Incorporated - http://www.ti.com/
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

#pragma once

#include "dynamic_loader_interface.h"
#include "u_lockable.h"
extern "C" {
    typedef void* DLOAD_HANDLE;
//#include "dload_api.h"
}

namespace Coal
{
    class DSPProgram;
}

namespace tiocl {


class DLOAD : public DynamicLoader, public Lockable
{
public:
    typedef int ProgramHandle;

    explicit DLOAD(Coal::DSPProgram *program);
    virtual ~DLOAD();
#ifndef _SYS_BIOS
    virtual bool LoadProgram(const std::string &fileName) override;
#else
    virtual bool LoadProgram(const std::string &binary_str) override;
#endif
    virtual bool UnloadProgram() override;
    virtual DSPDevicePtr QuerySymbol(const std::string &symName) const override;
    virtual DSPDevicePtr GetDataPagePointer() const override;
    virtual DSPDevicePtr GetProgramLoadAddress() const override;

    void SetProgramLoadAddress(DSPDevicePtr address);
    DLOAD_HANDLE GetDloadHandle() const { return dloadHandle; }

private:
    DLOAD_HANDLE dloadHandle;
    DSPDevicePtr programLoadAddress;
    ProgramHandle ph;
};

}
