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
#pragma once

#include "u_locks_pthread.h"
#include "u_lockable.h"
#include "message.h"
#include "device_info.h"

#include "mbox_interface.h"
#include "core/memory_range.h"
#include "memory_provider_factory.h"
#include "core/error_report.h"

extern "C"
{
#ifndef DSPC868X
    #include "mpm_mailbox.h"
#else
    #include "mailBox.h"
    #define MPM_MAILBOX_ERR_FAIL                MAILBOX_ERR_FAIL
    #define MPM_MAILBOX_ERR_MAIL_BOX_FULL       MAILBOX_ERR_MAIL_BOX_FULL
    #define MPM_MAILB0X_ERR_EMPTY               MAILB0X_ERR_EMPTY
    #define MPM_MAILBOX_READ_ERROR              MAILBOX_READ_ERROR
    #define MPM_MAILBOX_MEMORY_LOCATION_REMOTE  MAILBOX_MEMORY_LOCATION_REMOTE
    #define MPM_MAILBOX_DIRECTION_RECEIVE       MAILBOX_DIRECTION_RECEIVE
    #define MPM_MAILBOX_DIRECTION_SEND          MAILBOX_DIRECTION_SEND
    #define mpm_mailbox_config_t                mailBox_config_t
    #define mpm_mailbox_create                  mailBox_create
    #define mpm_mailbox_open                    mailBox_open
    #define mpm_mailbox_write                   mailBox_write
    #define mpm_mailbox_read                    mailBox_read
    #define mpm_mailbox_query                   mailBox_query
    #define mpm_mailbox_get_alloc_size          mailBox_get_alloc_size
#endif

#ifdef DSPC868X
#define MAILBOX_LOCATION MPM_MAILBOX_MEMORY_LOCATION_REMOTE
#else
#define MAILBOX_LOCATION MPM_MAILBOX_MEMORY_LOCATION_LOCAL
#endif

}

class MBoxMPM : public MBox, public Lockable
{
    public:
        MBoxMPM(Coal::DSPDevice *device);
        ~MBoxMPM();
        void     to   (uint8_t *msg, uint32_t  size, uint8_t id=0);
        int32_t  from (uint8_t *msg, uint32_t *size, uint8_t id=0);
        bool     query(uint8_t id=0);

  private:

#ifdef DSPC868X
    int32_t create(void* mbox_handle, uint32_t remote_node_id,
                   uint32_t mem_location, uint32_t direction,
                   mpm_mailbox_config_t *mbox_config)
    {
       int32_t result = mpm_mailbox_create(mbox_handle, remote_node_id,
                                        mem_location, direction, mbox_config);
        return result;
    }
#else
    int32_t create(void* mbox_handle, char *slave_node_name,
                   uint32_t mem_location, uint32_t direction,
                   mpm_mailbox_config_t *mbox_config)
    {
       int32_t result = mpm_mailbox_create(mbox_handle, slave_node_name,
                                        mem_location, direction, mbox_config);
        return result;
    }
#endif

    int32_t open(void* mbox_handle)
    {
        int32_t result = mpm_mailbox_open(mbox_handle);
        return result;
    }

    int32_t write (void* mbox_handle, uint8_t *buf, uint32_t size,
                   uint32_t trans_id)
    {
        int result;

        do result = mpm_mailbox_write (mbox_handle, buf, size, trans_id);
        while (result == MPM_MAILBOX_ERR_MAIL_BOX_FULL);

        return true;
    }

    int32_t read (void* mbox_handle, uint8_t *buf, uint32_t *size,
                  uint32_t *trans_id)
    {
        int32_t result = mpm_mailbox_read (mbox_handle, buf, size, trans_id);
        return result;
    }

    int32_t query (void* mbox_handle)
    {
        int32_t result = mpm_mailbox_query (mbox_handle);
        return result;
    }

  private:
        void*              p_rx_mbox;
        void*              p_tx_mbox;
        #ifndef DSPC868X
        tiocl::MemoryProviderFactory *p_mpf;
        #endif
};
