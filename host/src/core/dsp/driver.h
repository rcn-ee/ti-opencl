#ifndef _DRIVER_H
#define _DRIVER_H
#include "u_lockable.h"
#include "device.h"

extern "C"
{
    #include "pciedrv.h"
    #include "dnldmgr.h"
    #include "cmem_drv.h"
    #include "bufmgr.h"
}

#define LOCK 0

#if LOCK
class Driver : public Lockable
#else
class Driver : public Lockable_off
#endif
{
  public:
   ~Driver() { close(); }
   int32_t close();
   int32_t write    (int32_t dsp_id, DSPDevicePtr addr, uint8_t *buf, 
                     uint32_t size);
   int32_t read     (int32_t dsp_id, DSPDevicePtr addr, uint8_t *buf,
                     uint32_t size);
   int32_t dma_write(int32_t dsp_id, DSPDevicePtr addr, uint8_t *buf,
                     uint32_t size);
   int32_t dma_read (int32_t dsp_id, DSPDevicePtr addr, uint8_t *buf,
                     uint32_t size);

#if LOCK
    void reserve () { this->mutex.Lock();   }
    void release () { this->mutex.Unlock(); }
#else
    void reserve () { }
    void release () { }
#endif

    static Driver* instance ();

  private:
    static Driver* pInstance;
    pciedrv_open_config_t config;

   int32_t open();
    Driver() { open(); }
    Driver(const Driver&);              // copy ctor disallowed
    Driver& operator=(const Driver&);   // assignment disallowed
};


#define MAX_CONTIGUOUS_XFER_BUFFERS 2
#define HOST_CMEM_BUFFER_SIZE       0x400000 // 4M
#define MAX_NUM_HOST_DSP_BUFFERS    128


#if LOCK
class Cmem : public Lockable
#else
class Cmem : public Lockable_off
#endif
{
  public:
    ~Cmem() { close(); }
    static Cmem* instance ();

    void open();
    void close();
    void dma_write(int32_t dsp_id, uint32_t addr, uint8_t *buf, uint32_t size);
    void dma_read (int32_t dsp_id, uint32_t addr, uint8_t *buf, uint32_t size);

#if LOCK
    void reserve () { this->mutex.Lock();   }
    void release () { this->mutex.Unlock(); }
#else
    void reserve () { }
    void release () { }
#endif

  private:
    static Cmem* pInstance;

    cmem_host_buf_desc_t buf_desc[MAX_NUM_HOST_DSP_BUFFERS];
    void *               DmaBufPool;

    Cmem() : DmaBufPool(NULL) { open(); }
    Cmem(const Cmem&);              // copy ctor disallowed
    Cmem& operator=(const Cmem&);   // assignment disallowed
};

#endif // _DRIVER_H
