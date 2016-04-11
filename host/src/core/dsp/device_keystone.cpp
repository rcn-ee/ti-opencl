/******************************************************************************
 * Copyright (c) 2013-2015, Texas Instruments Incorporated - http://www.ti.com/
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


// Included in device.cpp

#include <vector>

#if defined(DEVICE_K2X)
extern "C" {
    extern int get_ocl_qmss_res(Msg_t *msg);
}
#include "tal/mbox_impl_mpm.h"
#elif defined (DEVICE_K2G)
#include "tal/mbox_impl_msgq.h"
#endif

/******************************************************************************
* DSPDevice::DSPDevice(unsigned char dsp_id)
******************************************************************************/
DSPDevice::DSPDevice(unsigned char dsp_id, SharedMemory* shm)
    : DeviceInterface   (),
      p_cores           (0),
      p_num_events      (0),
      p_dsp_mhz         (1000), // 1.00 GHz
      p_worker_dispatch  (0),
      p_worker_completion(0),
      p_stop            (false),
      p_exit_acked      (false),
      p_initialized     (false),
      p_dsp_id          (dsp_id),
      p_complete_pending(),
      p_mpax_default_res(NULL),
      p_shmHandler(shm)
{
    Driver *driver = Driver::instance();

    p_cores = driver->cores_per_dsp(dsp_id);

    driver->reset_and_load(dsp_id);

    void *hdl = driver->create_image_handle(dsp_id);

    p_addr_kernel_config = driver->get_symbol(hdl, "kernel_config_l2");
    p_addr_local_mem     = driver->get_symbol(hdl, "ocl_local_mem_start");
    p_size_local_mem     = driver->get_symbol(hdl, "ocl_local_mem_size");


#ifdef DSPC868X
    /*-------------------------------------------------------------------------
    * On DSPC868X, these 4 variables are retrieved from the monitor out file.
    *------------------------------------------------------------------------*/
    p_addr64_global_mem  = driver->get_symbol(hdl, "ocl_global_mem_start");
    p_size64_global_mem  = driver->get_symbol(hdl, "ocl_global_mem_size");
    p_addr_msmc_mem      = driver->get_symbol(hdl, "ocl_msmc_mem_start");
    p_size_msmc_mem      = driver->get_symbol(hdl, "ocl_msmc_mem_size");
#endif


    driver->free_image_handle(hdl);

    init_ulm();

    /*-------------------------------------------------------------------------
    * initialize the mailboxes on the cores, so they can receive an exit cmd
    *------------------------------------------------------------------------*/
    setup_mailbox();

#if defined(DEVICE_K2X) && !defined(DEVICE_K2G)
    /*-------------------------------------------------------------------------
    * Send monitor configuration
    *------------------------------------------------------------------------*/
    Msg_t msg = {CONFIGURE_MONITOR};
    msg.u.configure_monitor.n_cores = dspCores();

    // Keystone2: get QMSS resources from RM, mail to DSP monitor
    if (get_ocl_qmss_res(&msg) == 0)
    {
        printf("Unable to allocate resource from RM server!\n");
        exit(-1);
    }

    mail_to(msg);
#endif

    /*-------------------------------------------------------------------------
    * Query DSP frequency; monitor is in message loop task after this point.
    *------------------------------------------------------------------------*/
    setup_dsp_mhz();
}

void DSPDevice::setup_mailbox(void)
{
#ifdef DEVICE_K2G
    p_mb = new MBoxMsgQ(this);
#else
    p_mb = new MBoxMPM(this);
#endif
}


void DSPDevice::setup_dsp_mhz(void)
{
#ifdef DSPC868X
    char *ghz1 = getenv("TI_OCL_DSP_1_25GHZ");
    if (ghz1) p_dsp_mhz = 1250;  // 1.25 GHz
#else
    mail_to(frequencyMsg);

    int ret = 0;
    do
    {
        while (!mail_query())  ;
        ret = mail_from();
    } while (ret == -1);

    p_dsp_mhz = ret;
#endif
}

bool DSPDevice::hostSchedule() const
{
#ifdef DEVICE_K2G
    return true;
#else
    return false;
#endif
}
