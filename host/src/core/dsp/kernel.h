#ifndef __DSP_KERNEL_H__
#define __DSP_KERNEL_H__

#include "../deviceinterface.h"
#include "message.h"
#include "device.h"
#include <core/config.h>

#include <vector>
#include <string>
#include <pthread.h>
#include <stdint.h>

namespace llvm
{
    class Function;
}

typedef std::pair<DSPDevicePtr, size_t> DSPMemRange;

namespace Coal
{
class DSPDevice;
class Kernel;
class KernelEvent;

class DSPKernel : public DeviceKernel
{
    public:
        DSPKernel(DSPDevice *device, Kernel *kernel);
        ~DSPKernel();

        size_t       workGroupSize()          const { return 0; }
        cl_ulong     localMemSize()           const { return 0; }
        cl_ulong     privateMemSize()         const { return 0; }
        size_t       preferredWorkGroupSizeMultiple() const { return 0; }

        size_t       guessWorkGroupSize(cl_uint num_dims, cl_uint dim,
                                        size_t global_work_size) const;
        DSPDevicePtr device_entry_pt();
        DSPDevicePtr data_page_ptr();
        void         preAllocBuffers();

        Kernel *     kernel() const;     
        DSPDevice *  device() const;  

        llvm::Function *function() const;  
        static size_t typeOffset(size_t &offset, size_t type_len);

    private:
        DSPDevice *     p_device;
        Kernel *        p_kernel;
        DSPDevicePtr    p_device_entry_pt;
        DSPDevicePtr    p_data_page_ptr;
};

class DSPKernelEvent
{
    public:
        DSPKernelEvent  (DSPDevice *device, KernelEvent *event);
        ~DSPKernelEvent ();

        bool run      ();
        int  callArgs (unsigned *rs, 
                       std::vector<DSPDevicePtr> *locals_to_free,
                       std::vector<DSPMemRange> *buffers_to_flush);

    private:
        DSPDevice *     p_device;
        KernelEvent *   p_event;
        DSPKernel *     p_kernel;
};
}
#endif
