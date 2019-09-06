/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2012-2014, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file deviceinterface.h
 * \brief Abstraction layer between Clover core and the devices
 */

#ifndef __DEVICEINTERFACE_H__
#define __DEVICEINTERFACE_H__

#include <CL/cl.h>
#include <string>
#include <vector>
#include "object.h"
#include "icd.h"
#include "kernelentry.h"

/* This pulls in legacy::PassManager when using LLVM >= 3.5 */
#include <llvm/PassManager.h>

namespace llvm
{
    class PassManager;
    class Module;
    class Function;
}

namespace Coal
{
class DeviceInterface;
}

struct _cl_device_id: public Coal::descriptor<Coal::DeviceInterface, _cl_device_id> {};

namespace tiocl
{
class SharedMemory;
}

namespace Coal
{

class DeviceBuffer;
class DeviceProgram;
class DeviceKernel;

class MemObject;
class Event;
class Program;
class Kernel;

/**
 * \brief Abstraction layer between core Clover objects and the devices
 *
 * This interface is used by the core Clover classes to communicate with the
 * devices, that must reimplement all the functions described here.
 */
class DeviceInterface : public _cl_device_id, public Object
{
    public:

        enum Type
        {
            T_Invalid = 0,
            T_CPU,
            T_C66x,
            T_C7x,
            T_EVE,
            T_SubDevice
        };

        DeviceInterface(Type type) :
          Object(Object::T_Device, 0), p_type (type) {}
        virtual ~DeviceInterface() {}

        /**
         * \brief Retrieve information about the device
         *
         * This function is used to retrieve information about an object.
         * Sometimes, the size of the data retrieved is unknown (for example, a
         * string). The application can call this function twice, the first time
         * to get the size, then it allocates a buffer, and finally get the data.
         *
         * \code
         * const char *string = 0;
         * size_t len;
         *
         * object->info(FOO_PROPERTY_STRING, 0, 0, &len);
         * string = std::malloc(len);
         * object->info(FOO_PROPERTY_STRING, len, string, 0);
         * \endcode
         *
         * \param param_name Name of the property to retrieve
         * \param param_value_size Size of the application-allocated buffer
         *                         in which to put the value.
         * \param param_value Pointer to an application-allocated buffer
         *                    where the property data will be stored. Ignored
         *                    if NULL.
         * \param param_value_size_ret Size of the value retrieved, ignored if
         *                             NULL.
         * \return CL_SUCCESS in case of success, otherwise a CL error code.
         */
        virtual cl_int info(cl_device_info param_name,
                            size_t param_value_size,
                            void *param_value,
                            size_t *param_value_size_ret) const = 0;

        /**
         * \brief Create a \c Coal::DeviceBuffer object for this device
         * \param buffer Memory object for which the buffer has to be created
         * \param rs Error code (\c CL_SUCCESS if no error)
         * \return a \c Coal::DeviceBuffer object, undefined if there is an error
         */
        virtual DeviceBuffer *createDeviceBuffer(MemObject *buffer, cl_int *rs) = 0;

        /**
         * \brief Create a \c Coal::DeviceProgram object for this device
         * \param program \c Coal::Program containing the device-independent
         *                program data
         * \return a \c Coal::DeviceProgram object
         */
        virtual DeviceProgram *createDeviceProgram(Program *program) = 0;

        /**
         * \brief Create a \c Coal::DeviceKernel object for this device
         * \param kernel \c Coal::Kernel containing the device-independent kernel
         *               data
         * \param function device-specific \c llvm::Function to be used
         * \return a \c Coal::DeviceKernel object
         */
        virtual DeviceKernel *createDeviceKernel(Kernel *kernel,
                                                 llvm::Function *function)
        { return nullptr; }

        /**
         * \brief Create a \c Coal::DeviceKernel object for this device using a
         *        built-in kernel
         * \param kernel \c Coal::Kernel containing the device-independent kernel
         *               data
         * \param kernel_entry a device-specific \c Coal::KernelEntry that describes
         *                     a built-in kernel on the device
         * \return a \c Coal::DeviceKernel object
         */

        virtual DeviceKernel *createDeviceBuiltInKernel(Kernel *kernel,
                                                        KernelEntry *kernel_entry)
        { return nullptr; }

        virtual const std::vector<KernelEntry*> *getKernelEntries() const
        { return nullptr; }

        /**
         * \brief Push an event on the device
         * \sa the end of \ref events
         * \param event the event to be pushed
         */
        virtual void pushEvent(Event *event) = 0;

        /**
         * \brief Initialize device-specific event data
         *
         * This call allows a device to initialize device-specific event data,
         * by using \c Coal::Event::setDeviceData(). For instance, an
         * hardware-accelerated device can associate a device command to an
         * event, and use it to manage the event when it gets pushed.
         *
         * @note This function has one obligation: it must call
         *       \c Coal::MapBufferEvent::setPtr() and
         *       \c Coal::MapImageEvent::setPtr() (and other function described
         *       in its documentation)
         *
         * \param event the event for which data can be set
         * \return CL_SUCCESS in case of success
         */
        virtual cl_int initEventDeviceData(Event *event) = 0;

        /**
         * \brief Free device-specific event data
         *
         * This function is called just before \p event gets deleted. It allows
         * a device to free device-specific data of this event, if any.
         *
         * \param event the event that will be destroyed
         */
        virtual void freeEventDeviceData(Event *event) = 0;

        virtual std::string builtinsHeader(void) const = 0;

        virtual void init() = 0;

        /**
         * \brief Ask device if it has enough work in its queue
         */
        virtual bool gotEnoughToWorkOn() { return false; }

        /**
         * \brief Get shared memory used by this device
         */
        virtual tiocl::SharedMemory* GetSHMHandler() const = 0;

        virtual bool IsDeviceType(Type type)
        {
            if (!this) return false;
            return p_type == type;
        }


        virtual       DeviceInterface* GetRootDevice() = 0;
        virtual const DeviceInterface* GetRootDevice() const = 0;

    protected:
        Type p_type;
};

/**
 * \brief Device-specific memory buffer
 *
 * This class is the backing-store used on a device for a \c Coal::MemObject. It
 * is created by \c Coal::DeviceInterface::createDeviceBuffer().
 */
class DeviceBuffer
{
    public:
        DeviceBuffer() {}
        virtual ~DeviceBuffer() {}

        /**
         * \brief Allocate the buffer on the device
         * \return true when success, false otherwise
         */
        virtual bool allocate() = 0;

        /**
         * \brief \c tiocl::SharedMemory of this buffer
         * \return parent \c tiocl::SharedMemory
         */
        virtual tiocl::SharedMemory* GetSHMHandler() const = 0;

        /**
         * \brief Allocation status
         * \return true if already allocated, false otherwise
         */
        virtual bool allocated() const = 0;

        /**
         * \brief Host-accessible memory pointer
         *
         * This function returns what is passed as arguments to native kernels
         * (\c clEnqueueNativeKernel(), \c Coal::NativeKernelEvent) in place of
         * \c Coal::MemObject pointers.
         *
         * For \c Coal::CPUDevice, it's simply a pointer in RAM, but
         * hardware-accelerated devices may need to do some copying or mapping.
         *
         * \warning Beware that this data may get written to by the native kernel.
         *
         * \return A memory pointer usable by a host native kernel
         */
        virtual void *nativeGlobalPointer() const = 0;
};

/**
 * \brief Device-specific program data
 */
class DeviceProgram
{
    public:
        DeviceProgram() {}
        virtual ~DeviceProgram() {}

        /**
         * \brief Linking or not \b stdlib with this program
         *
         * \b stdlib is a LLVM bitcode file containing some implementations of
         * OpenCL C built-ins. This function allows a device to tell
         * \c Coal::Program::build() if it wants \b stdlib to be linked or not.
         *
         * Linking the library may allow inlining of functions like \c ceil(),
         * \c floor(), \c clamp(), etc. So, if these functions are not better
         * handled by the device itself than by \b stdlib, it's a good thing
         * to link it.
         *
         * But if the device provides instructions for these functions, then
         * it could be better not to link \b stdlib and to replace the LLVM
         * calls to these functions with device-specific instructions.
         *
         * \warning \b Stdlib currently only works for \c Coal::CPUDevice, as
         *          it contains host-specific code (LLVM IR is not meant to be
         *          portable, pointer size changes for example).
         *
         * \return true if \b stdlib must be linked with the program
         */
        virtual bool linkStdLib() const
        { return false; }

        /**
         * \brief Build a device-specific representation of the program
         *
         * This function is called by \c Coal::Program::build() when the module
         * is compiled and linked. It can be used by the device to build a
         * device-specific representation of the program.
         *
         * \param module \c llvm::Module containing the program's LLVM IR
         * \param binary_str \c std::string containing dep.unlinked_binary
         * \return true in case of success, false otherwise
         * \param binary_filename \c char* binary already in file, if not NULL
         */
        virtual bool build(llvm::Module *module, std::string* binary_str,
                           char *binary_filename)
        { return false; }

        /**
         * \brief Extract binaries from MIXED binary
         *
         * This function is called to extract LLVM bitcode from the native
         * binary in the MIXED binary.
         * \param binary_str \c std::string containing mixed binary
         * \param bitcode \c std::string returns LLVM bitcode if not NULL
         * \param native \c std::string returns native binary if not NULL
         * \return true if the binary is indeed mixed
         */
        virtual bool ExtractMixedBinary(const std::string &binary_str,
                                              std::string &bitcode)
        {  return false;  }
};

/**
 * \brief Device-specific kernel data
 */
class DeviceKernel
{
    public:
        DeviceKernel() {}
        virtual ~DeviceKernel() {}

        /**
         * \brief Maximum work-group size of a kernel
         * \return Maximum work-group size of the kernel based on device-specific
         *         data such as memory usage, register pressure, etc)
         */
        virtual size_t workGroupSize() const = 0;

        /**
         * \brief Local memory used by the kernel
         * \return Local memory used by the kernel, in bytes
         */
        virtual cl_ulong localMemSize() const = 0;

        /**
         * \brief Private memory used by the kernel
         * \return Private memory used by the kernel, in bytes
         */
        virtual cl_ulong privateMemSize() const = 0;

        /**
         * \brief Preferred work-group size multiple
         * \return The size multiple a work-group can have to work the best and
         *         the fastest on the device
         */
        virtual size_t preferredWorkGroupSizeMultiple() const = 0;

        /**
         * \brief Optimal work-group size
         *
         * This function allows a device to calculate the optimal work-group size
         * for this kernel, using it's memory usage, SIMD dimension, etc.
         *
         * \c Coal::CPUDevice tries to split the kernel into a number of
         * work-groups the closest possible to the number of CPU cores.
         *
         * \param num_dims Number of working dimensions
         * \param dim Dimension for which the multiple is being calculated
         * \param global_work_size Total number of work-items to split into
         *                         work-groups
         * \return optimal size of a work-group, for the \p dim dimension.
         */
        virtual size_t guessWorkGroupSize(cl_uint num_dims, cl_uint dim,
                                          size_t global_work_size) const = 0;
};

}

#endif
