#ifndef __DSP_DEVICE_H__
#define __DSP_DEVICE_H__

extern "C" {
#include "dload_api.h"
}

typedef uint32_t DSPDevicePtr;

#include "../deviceinterface.h"
#include "dspheap.h"
#include "message.h"
#include <pthread.h>
#include <string>
#include <list>
#include "core_scheduler.h"

namespace Coal
{

class MemObject;
class Event;
class Program;
class Kernel;

class DSPDevice : public DeviceInterface
{
    public:
        DSPDevice(unsigned char dsp_id);
        ~DSPDevice();

        void init();

        cl_int info(cl_device_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const;

        DeviceBuffer *createDeviceBuffer(MemObject *buffer, cl_int *rs);
        DeviceProgram *createDeviceProgram(Program *program);
        DeviceKernel *createDeviceKernel(Kernel *kernel,
                                         llvm::Function *function);

        cl_int initEventDeviceData(Event *event);
        void freeEventDeviceData(Event *event);

        void pushEvent(Event *event);
        Event *getEvent(bool &stop);

        unsigned int numDSPs() const;
        float dspMhz() const;
        unsigned char dspID() const;
        DLOAD_HANDLE dload_handle() const;

        int load(const char *filename);

        /*---------------------------------------------------------------------
        * These malloc routines return a uint32_t instead of a pointer
        * Because the target memory space is 32 bit and is independent of the 
        * size of a host pointer (ie. 32bit vs 64 bit)
        *--------------------------------------------------------------------*/
        uint32_t malloc_l2(size_t size);
        void free_l2(uint32_t add);
        uint32_t malloc_code(size_t size);
        void free_code(uint32_t add);
        uint32_t malloc_ddr(size_t size);
        void free_ddr(uint32_t add);

        void mail_to   (Msg_t& msg);
        bool mail_query();
        int  mail_from ();

        int dma_write(DSPDevicePtr addr, void* host_addr, size_t size);

        std::string sourceTranslation(std::string& source) ;

    private:
        unsigned int       p_cores;
        unsigned int       p_num_events;
        float              p_dsp_mhz;
        pthread_t          p_worker;
        int                p_rx_mbox;
        int                p_tx_mbox;
        std::list<Event *> p_events;
        pthread_cond_t     p_events_cond;
        pthread_mutex_t    p_events_mutex;
        bool               p_stop; 
        bool               p_initialized;
        unsigned char      p_dsp_id;
        dspheap            p_device_ddr_heap;
        dspheap            p_device_l2_heap;
        DLOAD_HANDLE       p_dload_handle;
};
}
#endif
