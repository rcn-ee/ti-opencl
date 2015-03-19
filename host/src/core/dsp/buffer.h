#ifndef __DSP_BUFFER_H__
#define __DSP_BUFFER_H__

#include "../deviceinterface.h"
#include "device.h"

namespace Coal
{

class DSPDevice;
class MemObject;

class DSPBuffer : public DeviceBuffer
{
    public:
        DSPBuffer(DSPDevice *device, MemObject *buffer, cl_int *rs);
        ~DSPBuffer();

        bool allocate();
        DeviceInterface *device() const;
        void *data() const ;
        void *nativeGlobalPointer() const ;
        bool allocated() const;

    private:
        DSPDevice *  p_device;
        MemObject *  p_buffer;
        DSPDevicePtr p_data;
        bool         p_data_malloced;
        unsigned int p_buffer_idx;
};
}
#endif
