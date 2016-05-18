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

#include <stdint.h>
#include <stdlib.h>

#include "dspmem.h"
#include "shmem_init_policy_rtos.h"
#include "core/error_report.h"

using namespace tiocl;

void
InitializationPolicyRTOS::DiscoverMemoryRanges(std::vector<MemoryRange>& ranges)
{
    // YUAN TODO: hard code for now, will get from platform/package config vars
    ranges.emplace_back(0xA0000000, 0x60000000,
                        MemoryRange::Kind::RTOS_SHMEM,
                        MemoryRange::Location::OFFCHIP);

    ranges.emplace_back(0x8B000000, 0x01000000,
                        MemoryRange::Kind::RTOS_HOSTMEM,
                        MemoryRange::Location::OFFCHIP);
#if 0
    ranges.emplace_back(addr1, size1,
                        MemoryRange::Kind::CMEM_PERSISTENT,
                        MemoryRange::Location::OFFCHIP);

    ranges.emplace_back(addr2, size2,
                        MemoryRange::Kind::CMEM_ONDEMAND,
                        MemoryRange::Location::OFFCHIP);

    ranges.emplace_back(onchip_shared_addr, onchip_shared_size,
                        MemoryRange::Kind::CMEM_PERSISTENT,
                        MemoryRange::Location::ONCHIP);
#endif
}

void
InitializationPolicyRTOS::Destroy()
{
}

