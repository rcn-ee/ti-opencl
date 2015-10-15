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
 * \file context.h
 * \brief OpenCL context
 */

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "object.h"
#include "icd.h"

#include <CL/cl.h>

namespace Coal
{
  class Context;
}
struct _cl_context: public Coal::descriptor<Coal::Context, _cl_context> {};

namespace Coal
{

class DeviceInterface;

/**
 * \brief OpenCL context
 * 
 * This class represents a context for managing objects such as command-queues, memory,
 * program and kernel objects and for executing kernels on one or more devices specified
 * in the context.
 */
class Context : public _cl_context, public Object
{
    public:
        /**
         * \brief Constructor
         * \param properties properties of the context
         * \param num_devices number of devices that will be used
         * \param devices \c Coal::DeviceInterface to be used
         * \param pfn_notify function to  call when an error arises, to give 
         *        more detail
         * \param user_data user data to pass to \p pfn_notify
         * \param errcode_ret return code
         */
        Context(const cl_context_properties *properties,
                cl_uint num_devices,
                const cl_device_id *devices,
                void (CL_CALLBACK *pfn_notify)(const char *, const void *,
                                               size_t, void *),
                void *user_data,
                cl_int *errcode_ret);
        ~Context();

        /**
         * \brief Info about the context
         * \copydetails Coal::DeviceInterface::info
         */
        cl_int info(cl_context_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const;

        /**
         * \brief Check that this context contains a given \p device
         * \param device device to check
         * \return whether this context contains \p device
         */
        bool hasDevice(DeviceInterface *device) const;

    private:
        cl_context_properties *p_properties;
        void (CL_CALLBACK *p_pfn_notify)(const char *, const void *,
                                               size_t, void *);
        void *p_user_data;

        DeviceInterface **p_devices;
        unsigned int p_num_devices, p_props_len;
        cl_platform_id p_platform;
};

}

#endif
