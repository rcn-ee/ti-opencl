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
#include "message.h"
#include "device_info.h"
#include "device.h"

#include "mbox_interface.h"
#include "core/memory_range.h"
#include "core/error_report.h"

#include "memory_provider_factory.h"
#include "mbox_impl_mpm.h"

MBoxMPM::MBoxMPM(Coal::DSPDevice *device)
{
    const tiocl::DeviceInfo& device_info = tiocl::DeviceInfo::Instance();

    // Storage for MPM mailbox (K2x, DSPC868X)
    DSPDevicePtr addr_mbox_d2h_phys = device_info.GetSymbolAddress("mbox_d2h_phys");
    DSPDevicePtr addr_mbox_h2d_phys = device_info.GetSymbolAddress("mbox_h2d_phys");
    uint32_t     size_mbox_d2h      = device_info.GetSymbolAddress("mbox_d2h_size");
    uint32_t     size_mbox_h2d      = device_info.GetSymbolAddress("mbox_h2d_size");

    uint32_t mailboxallocsize     = mpm_mailbox_get_alloc_size();

    p_tx_mbox = (void*)malloc(mailboxallocsize);
    p_rx_mbox = (void*)malloc(mailboxallocsize);

    mpm_mailbox_config_t mbConfig;
    mbConfig.mem_size         = size_mbox_h2d;
    mbConfig.max_payload_size = mbox_payload;

#ifdef DSPC868X    // mailbox location is remote
    mbConfig.mem_start_addr   = (uint32_t) addr_mbox_h2d_phys;
    int tx_status = create(p_tx_mbox,
                     MAILBOX_MAKE_DSP_NODE_ID(device->dspID(), 0),
                     MAILBOX_LOCATION,
                     MPM_MAILBOX_DIRECTION_SEND, &mbConfig);
#else              // mailbox location is local
    p_mpf = new tiocl::MemoryProviderFactory();
    p_mpf->CreateMemoryProvider(MemoryRange(addr_mbox_d2h_phys, size_mbox_d2h,
                                        MemoryRange::Kind::DEVMEM,
                                        MemoryRange::Location::ONCHIP));
    p_mpf->CreateMemoryProvider(MemoryRange(addr_mbox_h2d_phys, size_mbox_h2d,
                                         MemoryRange::Kind::DEVMEM,
                                         MemoryRange::Location::ONCHIP));
    const tiocl::MemoryProvider *mp =
                                p_mpf->GetMemoryProvider (addr_mbox_h2d_phys);
    mbConfig.mem_start_addr   = (uint32_t)mp->MapToHostAddressSpace(
                                        addr_mbox_h2d_phys, size_mbox_h2d,
                                        false);
    int tx_status = create(p_tx_mbox,
       		     NULL,
                     MAILBOX_LOCATION,
                     MPM_MAILBOX_DIRECTION_SEND, &mbConfig);
#endif

    mbConfig.mem_size         = size_mbox_d2h;

#ifdef DSPC868X    // mailbox location is remote
    mbConfig.mem_start_addr   = (uint32_t)addr_mbox_d2h_phys;
    int rx_status = create(p_rx_mbox,
                     MAILBOX_MAKE_DSP_NODE_ID(device->dspID(), 0),
                     MAILBOX_LOCATION,
                     MPM_MAILBOX_DIRECTION_RECEIVE, &mbConfig);
#else              // mailbox location is local
    mp = p_mpf->GetMemoryProvider(addr_mbox_d2h_phys);
    mbConfig.mem_start_addr   = (uint32_t)mp->MapToHostAddressSpace(
                                        addr_mbox_d2h_phys, size_mbox_d2h,
                                        false);
    int rx_status = create(p_rx_mbox,
		     NULL,
                     MAILBOX_LOCATION,
                     MPM_MAILBOX_DIRECTION_RECEIVE, &mbConfig);
#endif

    tx_status |= open(p_tx_mbox);
    rx_status |= open(p_rx_mbox);

    if (tx_status != 0 || rx_status != 0)
        tiocl::ReportError(tiocl::ErrorType::Fatal, tiocl::ErrorKind::MailboxCreationFailed);

}


void MBoxMPM::to(uint8_t *msg, uint32_t  size, uint8_t id)
{
    static unsigned trans_id = TX_ID_START;

    Lock lock(this);
    write(p_tx_mbox, msg, size, trans_id++);
}

int32_t MBoxMPM::from (uint8_t *msg, uint32_t *size, uint8_t *id)
{
    uint32_t trans_id_rx;

    read(p_rx_mbox, msg, size, &trans_id_rx);

    /*-------------------------------------------------------------------------
    * We do not currently need a return core from the MPM mailbox, 
    *------------------------------------------------------------------------*/
    if (id != 0) *id = -1;

    return trans_id_rx;
}

bool MBoxMPM::query(uint8_t id)
{
    return query(p_rx_mbox);
}

MBoxMPM::~MBoxMPM()
{
    free(p_tx_mbox);
    free(p_rx_mbox);

    p_tx_mbox = p_rx_mbox = NULL;

    #ifndef DSPC868X
    p_mpf->DestroyMemoryProviders();
    delete p_mpf;
    #endif
}

