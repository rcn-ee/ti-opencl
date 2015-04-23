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
#include <stdint.h>
#ifndef _SHMEM_H
#define _SHMEM_H

#if !defined (DEVICE_AM57)
extern "C"
{
    #include <mpm_transport.h>
}
#endif
#include "dspmem.h"

/*=============================================================================
* Abstract class for Shared memory
*============================================================================*/
class shmem
{
  public:
    shmem                          ();
    virtual         ~shmem         ();
    virtual void     configure_base(DSPDevicePtr64 dsp_addr,  uint64_t size);
    virtual void     configure     (DSPDevicePtr64 dsp_addr,  uint64_t size) = 0;
    virtual void    *map           (DSPDevicePtr64 dsp_addr,  uint32_t size,
                                    bool is_read=false) = 0;
    virtual void     unmap         (void* host_addr, uint32_t size,
                                    bool is_write=false) = 0;
    virtual bool     cacheInv      (void *host_addr, uint32_t size) { return true; }
    virtual bool     cacheWb       (void *host_addr, uint32_t size) { return true; }
    virtual bool     cacheWbInv    (void *host_addr, uint32_t size) { return true; }
    uint32_t         page_size     ();
    DSPDevicePtr64   start         () { return p_dsp_addr; }
    int64_t          size          () { return p_size; }

  protected:
    DSPDevicePtr64 p_dsp_addr;
    int64_t  p_size;
    uint32_t p_page_size;
    int32_t  p_mmap_fd;
    int32_t  p_threshold;
#if !defined (DEVICE_AM57)
    mpm_transport_h p_mpm_transport_handle;
#endif
    
};

/*=============================================================================
* Peristent implementation of shmem
*============================================================================*/
class shmem_persistent : public shmem 
{
  public:
    shmem_persistent ();
   ~shmem_persistent ();
    void          configure(DSPDevicePtr64 dsp_addr, uint64_t size);
    virtual void *map      (DSPDevicePtr64 dsp_addr, uint32_t size, bool is_read=false);
    virtual void  unmap    (void*    host_addr, uint32_t size, bool is_write=false);

  private:
      void *  p_host_addr;
      void *  p_xlate_dsp_to_host_offset;
};

/*=============================================================================
* On Demand implementation of shmem
*============================================================================*/
class shmem_ondemand : public shmem
{
  public:
    shmem_ondemand ();
   ~shmem_ondemand ();
    void          configure(DSPDevicePtr64 dsp_addr, uint64_t size);
    virtual void *map      (DSPDevicePtr64 dsp_addr, uint32_t size, bool is_read=false);
    virtual void  unmap    (void*    host_addr, uint32_t size, bool is_write=false);
};

/*=============================================================================
* shmem using CMem (commoning class)
*============================================================================*/
class shmem_cmem : public shmem
{
  public:
    shmem_cmem() {}
   ~shmem_cmem() {}

    virtual void  configure (DSPDevicePtr64 dsp_addr,  uint64_t size) = 0;
    virtual void *map       (DSPDevicePtr64 dsp_addr,  uint32_t size,
                                                 bool is_read=false) = 0;
    virtual void  unmap     (void* host_addr, uint32_t size,
                                                bool is_write=false) = 0;

           bool   cacheInv  (void *host_addr, uint32_t size);
           bool   cacheWb   (void *host_addr, uint32_t size);
           bool   cacheWbInv(void *host_addr, uint32_t size);
    static bool   cacheWbInvAll();

    static void   cmem_init (DSPDevicePtr64* addr1, uint64_t* size1,
                             DSPDevicePtr*   addr2, uint32_t* size2,
                             DSPDevicePtr64* addr3, uint64_t* size3);
    static void   cmem_exit ();
};

/*=============================================================================
* Peristent implementation of shmem using CMem
*============================================================================*/
class shmem_cmem_persistent : public shmem_cmem
{
  public:
    shmem_cmem_persistent (int cmem_block);
   ~shmem_cmem_persistent ();
    void          configure(DSPDevicePtr64 dsp_addr, uint64_t size);
    virtual void *map      (DSPDevicePtr64 dsp_addr, uint32_t size, bool is_read=false);
    virtual void  unmap    (void*    host_addr, uint32_t size, bool is_write=false);

  private:
      void *  p_host_addr;
      int64_t p_xlate_dsp_to_host_offset;
      int     p_cmem_block;
};

/*=============================================================================
* Ondemand implementation of shmem using CMem
*============================================================================*/
class shmem_cmem_ondemand : public shmem_cmem
{
  public:
    shmem_cmem_ondemand () {}
   ~shmem_cmem_ondemand () {}
    void          configure(DSPDevicePtr64 dsp_addr, uint64_t size);
    virtual void *map      (DSPDevicePtr64 dsp_addr, uint32_t size, bool is_read=false);
    virtual void  unmap    (void*    host_addr, uint32_t size, bool is_write=false);

    static DSPDevicePtr64 cmem_malloc(uint64_t size);
    static void           cmem_free  (DSPDevicePtr64 addr);
};

#endif // _SHMEM_H
