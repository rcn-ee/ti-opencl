/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
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
 * \file buffer.h
 * \brief CPU buffer
 */

#ifndef __CPU_BUFFER_H__
#define __CPU_BUFFER_H__

#include "../deviceinterface.h"

namespace Coal
{

class CPUDevice;
class MemObject;

/**
 * \brief CPU implementation of \c Coal::MemObject
 *
 * This class is responsible of the actual allocation of buffer objects, using
 * \c malloc() or by reusing a given \c host_ptr.
 */
class CPUBuffer : public DeviceBuffer
{
    public:
        /**
         * \brief Constructor
         * \param device Device for which the buffer is allocated
         * \param buffer \c Coal::MemObject holding information about the buffer
         * \param rs return code (\c CL_SUCCESS if all is good)
         */
        CPUBuffer(CPUDevice *device, MemObject *buffer, cl_int *rs);
        ~CPUBuffer();

        bool allocate();
        DeviceInterface *device() const;
        void *data() const;                 /*!< \brief Pointer to the buffer's data */
        void *nativeGlobalPointer() const;
        bool allocated() const;

    private:
        CPUDevice *p_device;
        MemObject *p_buffer;
        void *p_data;
        bool p_data_malloced;
};

}

#endif
