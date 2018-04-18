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
 * \file memobject.h
 * \brief Memory objects
 */

#ifndef __MEMOBJECT_H__
#define __MEMOBJECT_H__

#include "object.h"
#include "icd.h"
#include "dsp/u_concurrent_stack.h"
#include <list>

#include <CL/cl.h>

namespace Coal
{
  class MemObject;
}
struct _cl_mem: public Coal::descriptor<Coal::MemObject, _cl_mem> {};

namespace tiocl
{
  class SharedMemory;
}

namespace Coal
{

class DeviceBuffer;
class Context;
class DeviceInterface;
class BufferEvent;

/**
 * \brief Base class for all the memory objects
 */
class MemObject : public _cl_mem, public Object
{
    public:
        /**
         * \brief Type of memory object
         */
        enum Type
        {
            Buffer,
            SubBuffer,
            Image2D,
            Image3D
        };

        /**
         * \brief Constructor
         * \param ctx parent \c Coal::Context
         * \param flags memory object flags
         * \param host_ptr host pointer used by some flags (see the OpenCL spec)
         * \param errcode_ret return value
         * \note Don't do any initialization here, but in \c init(). We only fill
         *       the private variables and check the values passed in argument.
         * \sa init
         */
        MemObject(Context *ctx, cl_mem_flags flags, void *host_ptr,
                  cl_int *errcode_ret);
        virtual ~MemObject();

        // Disable default constructor, copy constuction and assignment
        MemObject()                            =delete;
        MemObject(const MemObject&)            =delete;
        MemObject& operator=(const MemObject&) =delete;

        /**
         * \brief Initialize the memory object
         * 
         * Memory objects are device-independent classes. This function creates
         * one \c Coal::DeviceBuffer per device present in the context by
         * calling \c Coal::DeviceInterface::createDeviceBuffer().
         * 
         * If there is only one device, its \c Coal::DeviceBuffer is directly
         * allocated. If there are more than one device, the allocation is
         * deferred until a \c Coal::Event is pushed for this device.
         * 
         * \return \c CL_SUCCESS if success, an error code otherwise
         */
        virtual cl_int init();
        virtual bool allocate(DeviceInterface *device); /*!< \brief Allocate this memory object on the given \p device */
        virtual size_t size() const = 0;                /*!< \brief Device-independent size of the memory object */
        virtual Type type() const = 0;                  /*!< \brief Type of the memory object */

        cl_mem_flags flags() const;                     /*!< \brief Flags */
        void *host_ptr() const;                         /*!< \brief Host pointer */
        DeviceBuffer *deviceBuffer(DeviceInterface *device) const; /*!< \brief \c Coal::DeviceBuffer for the given \p device */

        DeviceBuffer *deviceBuffer(tiocl::SharedMemory *shm) const; /*!< \brief \c Coal::DeviceBuffer for the given \p shared memory handler */


        void deviceAllocated(DeviceBuffer *buffer);     /*!< \brief Is the \c Coal::DeviceBuffer for \p buffer allocated ? */

        /**
         * \brief Set a destructor callback for this memory object
         * 
         * This callback is called when this memory object is deleted. It is
         * currently called from the destructor, so the memory object is already
         * invalid, but as OpenCL objects are immutable, the callback cannot
         * use its \c memobj parameter except in a pointer comparison, and there
         * is no problem.
         * 
         * \param pfn_notify function to call when the memory object is deleted
         * \param user_data user data to pass to this function
         */
        void setDestructorCallback(void (CL_CALLBACK *pfn_notify)(cl_mem memobj,
                                                             void *user_data),
                                   void *user_data);

        /**
         * \brief Get information about this memory object
         * \copydetails Coal::DeviceInterface::info
         */
        cl_int info(cl_mem_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const;

        virtual bool            addMapEvent(BufferEvent *mapped_event) { return false; }
        virtual BufferEvent* removeMapEvent(void *mapped_ptr) { return NULL; }

        void set_host_ptr_clMalloced() {  p_host_ptr_clMalloced = true;  }
        bool get_host_ptr_clMalloced() {  return p_host_ptr_clMalloced;  }

    protected:
        cl_mem_flags             p_flags;
        std::list<BufferEvent *> p_mapped_events;

    private:
        unsigned int   p_num_devices, p_devices_to_allocate;
        void          *p_host_ptr;
        bool           p_host_ptr_clMalloced;
        DeviceBuffer **p_devicebuffers;

        typedef std::pair<void (CL_CALLBACK *)(cl_mem memobj, void *user_data), void*> dtor_callback_t;
        concurrent_stack<dtor_callback_t> p_dtor_callback_stack;

        //void (CL_CALLBACK *p_dtor_callback)(cl_mem memobj, void *user_data);
        //void *p_dtor_userdata;
};

/**
 * \brief Simple buffer object
 */
class Buffer : public MemObject
{
    public:
        /**
         * \brief Constructor
         * \param ctx parent \c Coal::Context
         * \param size size of the buffer, in bytes
         * \param host_ptr host pointer
         * \param flags memory flags
         * \param errcode_ret return code
         */
        Buffer(Context *ctx, size_t size, void *host_ptr, cl_mem_flags flags,
               cl_int *errcode_ret);

        size_t size() const; /*!< \brief Size of the buffer, in bytes */
        Type type() const;   /*!< \brief Return that we are a \c Coal::MemObject::Buffer */

        bool            addMapEvent(BufferEvent *mapped_event);
        BufferEvent* removeMapEvent(void *mapped_ptr);
    private:
        size_t p_size;

};

/**
 * \brief Sub-buffer
 */
class SubBuffer : public MemObject
{
    public:
        /**
         * \brief Constructor
         * \param parent parent \c Coal::Buffer
         * \param offset offset in \p parent of the start of this sub-buffer
         * \param size size of the sub-buffer
         * \param flags memory flags (must be compatible with the \p parent's ones)
         * \param errcode_ret return code
         */
        SubBuffer(class Buffer *parent, size_t offset, size_t size,
                  cl_mem_flags flags, cl_int *errcode_ret);
        ~SubBuffer();

        size_t size() const;                    /*!< \brief Size */
        Type type() const;                      /*!< \brief Return that we are a \c Coal::MemObject::SubBuffer */
        bool allocate(DeviceInterface *device); /*!< \brief Allocate the \b parent \c Coal::Buffer */

        size_t offset() const;                  /*!< \brief Offset in bytes */
        class Buffer *parent() const;           /*!< \brief Parent \c Coal::Buffer */

        bool            addMapEvent(BufferEvent *mapped_event);
        BufferEvent* removeMapEvent(void *mapped_ptr);
    private:
        size_t p_offset, p_size;
        class Buffer *p_parent;
};

/**
 * \brief 2D image
 */
class Image2D : public MemObject
{
    public:
        /**
         * \brief Constructor
         * \param ctx parent \c Coal::Context
         * \param width width of the image
         * \param height height of the image
         * \param row_pitch number of bytes in a row of pixels. If 0, defaults to <tt>width * pixel_size()</tt>
         * \param format image format
         * \param host_ptr host pointer
         * \param flags memory flags
         * \param errcode_ret return code
         */
        Image2D(Context *ctx, size_t width, size_t height, size_t row_pitch,
                const cl_image_format *format, void *host_ptr,
                cl_mem_flags flags, cl_int *errcode_ret);

        virtual size_t size() const;           /*!< \brief Size in bytes */
        virtual Type type() const;             /*!< \brief Return that we are a \c Coal::MemObject::Image2D */

        size_t width() const;                  /*!< \brief Width */
        size_t height() const;                 /*!< \brief Height */
        size_t row_pitch() const;              /*!< \brief Size in bytes of a row of pixels */
        virtual size_t slice_pitch() const;    /*!< \brief Size in bytes of the image */
        const cl_image_format &format() const; /*!< \brief Image format descriptor */

        /**
         * \brief Information about this image object
         * 
         * This function is also usable for \c Coal::Image3D objects as it does
         * casting when necessary in order to give information when needed.
         * 
         * \copydetails Coal::DeviceInterface::info
         */
        cl_int imageInfo(cl_image_info param_name,
                         size_t param_value_size,
                         void *param_value,
                         size_t *param_value_size_ret) const;

        static size_t element_size(const cl_image_format &format);  /*!< \brief Size in bytes of each channel of \p format */
        static unsigned int channels(const cl_image_format &format);/*!< \brief Number of channels of \p format */
        static size_t pixel_size(const cl_image_format &format);    /*!< \brief Size in bytes of a pixel in \p format */
        size_t pixel_size() const;                                  /*!< \brief Pixel size of this image */
        size_t element_size() const;                                /*!< \brief Channel size of this image */
        unsigned int channels() const;                              /*!< \brief Number of channels of this image */

    private:
        size_t p_width, p_height, p_row_pitch;
        cl_image_format p_format;
};

/**
 * \brief 3D image
 */
class Image3D : public Image2D
{
    public:
        /**
         * \brief Constructor
         * \param ctx parent \c Coal::Context
         * \param width width of the image
         * \param height height of the image
         * \param depth depth of the image
         * \param row_pitch number of bytes in a row of pixels. If 0, defaults to <tt>width * pixel_size()</tt>
         * \param slice_pitch number of bytes in a 2D slice. If 0, defaults to <tt>height * row_pitch()</tt>
         * \param format image format
         * \param host_ptr host pointer
         * \param flags memory flags
         * \param errcode_ret return code
         */
        Image3D(Context *ctx, size_t width, size_t height, size_t depth,
                size_t row_pitch, size_t slice_pitch,
                const cl_image_format *format, void *host_ptr,
                cl_mem_flags flags, cl_int *errcode_ret);

        size_t size() const;        /*!< \brief Size in bytes of this image */
        Type type() const;          /*!< \brief Return that we are a \c Coal::MemObject::Image3D */

        size_t depth() const;       /*!< \brief Depth of the image */
        size_t slice_pitch() const; /*!< \brief Size in bytes of a 2D slice */

    private:
        size_t p_depth, p_slice_pitch;
};

}

#endif
