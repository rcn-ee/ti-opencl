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
#ifndef _DRIVER_H
#define _DRIVER_H
#include "u_lockable.h"
#include "device.h"

#ifdef DSPC868X
extern "C"
{
    #include "pciedrv.h"
    #include "dnldmgr.h"
    #include "cmem_drv.h"
    #include "bufmgr.h"
}
#endif

class Driver : public Lockable_off
{
  public:
   // dtor is not called directly. DSPDevice calls close in its dtor
   ~Driver() { close(); }

   int32_t num_dsps() const { return pNum_dsps; }
   std::string dsp_monitor(int dsp);
   int cores_per_dsp(int dsp);
   int32_t close();

   void         reset_and_load   (int chip);
   void*        create_image_handle(int chip);
   void         free_image_handle(void *handle);


   DSPDevicePtr get_symbol(void* image_handle, const char *name);

   static Driver* instance ();

  private:
    static Driver*         pInstance;
    int32_t                pNum_dsps;

#ifdef DSPC868X
    pciedrv_open_config_t  config;
    pciedrv_device_info_t *pDevices_info;
#endif

    int32_t open ();
    Driver()  { open(); }
    Driver(const Driver&);              // copy ctor disallowed
    Driver& operator=(const Driver&);   // assignment disallowed
};

#endif // _DRIVER_H
