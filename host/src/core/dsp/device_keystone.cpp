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

#include "mbox_impl_mpm.h"

#if defined(DEVICE_K2X)
extern "C" {
    extern int get_ocl_qmss_res(Msg_t *msg);
}
#endif

/******************************************************************************
* DSPDevice::DSPDevice(unsigned char dsp_id)
******************************************************************************/
DSPDevice::DSPDevice(unsigned char dsp_id)
    : DeviceInterface   (), 
      p_core_mail       (0), 
      p_cores           (0),
      p_num_events      (0), 
      p_dsp_mhz         (1000), // 1.00 GHz
      p_worker_dispatch  (0), 
      p_worker_completion(0), 
      p_stop            (false),
      p_exit_acked      (false),
      p_initialized     (false), 
      p_dsp_id          (dsp_id), 
      p_device_msmc_heap(),
      p_device_ddr_heap1(),
      p_device_ddr_heap2(),
      p_device_ddr_heap3(),
      p_complete_pending(),
      p_mpax_default_res(NULL)
{ 
    Driver *driver = Driver::instance();

    p_cores = driver->cores_per_dsp(dsp_id);

    driver->reset_and_load(dsp_id);

    void *hdl = driver->create_image_handle(dsp_id);

    p_addr_kernel_config = driver->get_symbol(hdl, "kernel_config_l2");
    p_addr_local_mem     = driver->get_symbol(hdl, "ocl_local_mem_start");
    p_size_local_mem     = driver->get_symbol(hdl, "ocl_local_mem_size");

    DSPDevicePtr64 global3 = 0;
    uint64_t       gsize3  = 0;
#ifndef DSPC868X
    /*-------------------------------------------------------------------------
    * On K2X, these 4 variables are determined by query of the CMEM system.
    *------------------------------------------------------------------------*/
    p_addr64_global_mem = 0;
    p_size64_global_mem = 0;
    p_addr_msmc_mem = 0;
    p_size_msmc_mem = 0;
    driver->cmem_init(&p_addr64_global_mem, &p_size64_global_mem,
                      &p_addr_msmc_mem,     &p_size_msmc_mem,
                      &global3,             &gsize3);
#else
    /*-------------------------------------------------------------------------
    * On DSPC868X, these 4 variables are retrieved from the monitor out file.
    *------------------------------------------------------------------------*/
    p_addr64_global_mem  = driver->get_symbol(hdl, "ocl_global_mem_start");
    p_size64_global_mem  = driver->get_symbol(hdl, "ocl_global_mem_size");
    p_addr_msmc_mem      = driver->get_symbol(hdl, "ocl_msmc_mem_start");
    p_size_msmc_mem      = driver->get_symbol(hdl, "ocl_msmc_mem_size");
#endif


    driver->free_image_handle(hdl);

    DSPDevicePtr64 global1 = p_addr64_global_mem;
    DSPDevicePtr64 global2 = 0;
    uint64_t       gsize1  = p_size64_global_mem;
    uint64_t       gsize2  = 0;

    setup_memory(global1, global2, global3, gsize1, gsize2, gsize3);

    init_ulm(gsize1, gsize2, gsize3);

    /*-------------------------------------------------------------------------
    * initialize the mailboxes on the cores, so they can receive an exit cmd
    *------------------------------------------------------------------------*/
    setup_mailbox();

#if defined(DEVICE_K2X)
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
    p_mb = new MBoxMPM(this);
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


