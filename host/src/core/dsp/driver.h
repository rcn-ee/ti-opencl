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
#include <vector>
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
#else
#include "shmem.h"
#endif

class Driver : public Lockable_off
{
  public:
   ~Driver() { close(); }
   int32_t num_dsps() const { return pNum_dsps; }
   int32_t close();

   int32_t write(int32_t dsp, DSPDevicePtr64 addr, uint8_t *buf, uint32_t sz);
   int32_t read (int32_t dsp, DSPDevicePtr64 addr, uint8_t *buf, uint32_t sz);

   void*        reset_and_load   (int chip);
   void         free_image_handle(void *handle);
   void         cmem_init(DSPDevicePtr64 *addr1, uint64_t *size1,
                          DSPDevicePtr   *addr2, uint32_t *size2,
                          DSPDevicePtr64 *addr3, uint64_t *size3);
   void         cmem_exit();
   DSPDevicePtr64 cmem_ondemand_malloc(uint64_t size);
   void           cmem_ondemand_free  (DSPDevicePtr64 addr);
   void         split_ddr_memory (DSPDevicePtr64 addr,   uint64_t  size,
                                  DSPDevicePtr64& addr1, uint64_t& size1, 
                                  DSPDevicePtr64& addr2, uint64_t& size2,
                                                         uint64_t& size3);
   void         shmem_configure  (DSPDevicePtr64 addr, uint64_t size,
                                  int cmem_block = -1);
   // enqueueMapBuffer in two steps: map and return in application thread,
   //                                run and invalidate in worker thread
   void*        map              (Coal::DSPDevice *device,
                                  DSPDevicePtr64 addr, uint32_t sz,
                                  bool is_read = false,
                                  bool allow_fail = false);
   int32_t      unmap            (Coal::DSPDevice *device,
                                  void *host_addr, DSPDevicePtr64 buf_addr,
                                  uint32_t sz, bool is_write = false);
   bool cacheInv(  DSPDevicePtr64 addr, void *host_addr, uint32_t sz);
   bool cacheWb(   DSPDevicePtr64 addr, void *host_addr, uint32_t sz);
   bool cacheWbInv(DSPDevicePtr64 addr, void *host_addr, uint32_t sz);
   bool cacheWbInvAll();
   DSPDevicePtr get_symbol(void* image_handle, const char *name);

   static Driver* instance ();

  private:
    static Driver*         pInstance;
    int32_t                pNum_dsps;

#ifdef DSPC868X
    pciedrv_open_config_t  config;
    pciedrv_device_info_t *pDevices_info;
#else
    std::vector<shmem*> pShmem_areas;
    shmem* get_memory_region(DSPDevicePtr64 addr);
#endif

    int32_t open ();
    bool    wait_for_ready(int chip);
    int32_t write_core(int32_t dsp, DSPDevicePtr64 addr, uint8_t *buf, 
                       uint32_t sz);

    Driver()  { open(); }
    Driver(const Driver&);              // copy ctor disallowed
    Driver& operator=(const Driver&);   // assignment disallowed
};

#endif // _DRIVER_H
