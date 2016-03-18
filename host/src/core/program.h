/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2012-2016, Texas Instruments Incorporated - http://www.ti.com/
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
 * \file core/program.h
 * \brief Program
 */

#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "object.h"
#include "icd.h"

#include <CL/cl.h>
#include <string>
#include <vector>

namespace Coal
{
  class Program;
}
struct _cl_program: public Coal::descriptor<Coal::Program, _cl_program> {};


namespace llvm
{
    class LLVMContext;
    class MemoryBuffer;
    class Module;
    class Function;
}

namespace Coal
{

class Context;
class Compiler;
class DeviceInterface;
class DeviceProgram;
class Kernel;

/**
 * \brief Program object
 *
 * This class compiles and links a source or binaries into LLVM modules for each
 * \c Coal::DeviceInterface for which the program is built.
 *
 * It then contains functions to get the list of kernels available in the
 * program, using \c Coal::Kernel objects.
 */
class Program : public _cl_program, public Object
{
    public:
        /**
         * \brief Constructor
         * \param ctx parent \c Coal::Context
         */
        Program(Context *ctx);
        ~Program();

        /**
         * \brief Program type
         */
        enum Type
        {
            Invalid, /*!< Invalid or unknown, type of a program not already loaded */
            Source,  /*!< Program made of sources that must be compiled and linked */
            Binary   /*!< Program made of pre-built binaries that only need to be (transformed)/linked */
        };

        /**
         * \brief Program state
         */
        enum State
        {
            Empty,   /*!< Just created */
            Loaded,  /*!< Source or binary loaded */
            Built,   /*!< Built */
            Failed,  /*!< Build failed */
        };

        /**
         * \brief Load sources into the program
         *
         * This function loads the source-code given in \p strings into the
         * program and sets its type to \c Source.
         *
         * \param count number of strings in \p strings
         * \param strings array of pointers to strings, either null-terminated
         *        or of length given in \p lengths
         * \param lengths lengths of the strings. If a field is 0, the
         *        corresponding string is null-terminated. If \p lengths is
         *        0, all the strings are null-terminated
         * \return \c CL_SUCCESS if success, an error code otherwise
         */
        cl_int loadSources(cl_uint count, const char **strings,
                           const size_t *lengths);

        /**
         * \brief Load binaries into the program
         *
         * This function allows client application to load a source, retrieve
         * binaries using \c buildInfo(), and then re-create the same program
         * (after a restart for example) by giving it a precompiled binary.
         *
         * This function loads the binaries for each device and parse them into
         * LLVM modules, then sets the program type to \c Binary or
         * \c NativeBinary.
         *
         * \param data array of pointers to binaries, one for each device
         * \param lengths lengths of the binaries pointed to by \p data
         * \param binary_status array that will be filled by this function with
         *        the status of each loaded binary (\c CL_SUCCESS if success)
         * \param num_devices number of devices for which a binary is loaded
         * \param device_list list of devices for which the binaries are loaded
         * \return \c CL_SUCCESS if success, an error code otherwise
         */
        cl_int loadBinaries(const unsigned char **data, const size_t *lengths,
                            cl_int *binary_status, cl_uint num_devices,
                            DeviceInterface * const*device_list);

        /**
         * \brief Build the program
         *
         * This function compiles the sources, if any, and then link the
         * resulting binaries if the devices for which they are compiled asks
         * \c Coal::Program to do so, using \c Coal::DeviceProgram::linkStdLib().
         *
         * \param options options to pass to the compiler, see the OpenCL
         *        specification.
         * \param pfn_notify callback function called at the end of the build
         * \param user_data user data given to \p pfn_notify
         * \param num_devices number of devices for which binaries are being
         *        built. If it's a source-based program, this can be 0.
         * \param device_list list of devices for which the program will be built.
         * \return \c CL_SUCCESS if success, an error code otherwise
         */
        cl_int build(const char *options,
                     void (CL_CALLBACK *pfn_notify)(cl_program program,
                                                    void *user_data),
                     void *user_data, cl_uint num_devices,
                     DeviceInterface * const*device_list);

        Type type() const;   /*!< \brief Type of the program */
        State state() const; /*!< \brief State of the program */

        /**
         * \brief Create a kernel given a \p name
         * \param name name of the kernel to be created
         * \param errcode_ret return code (\c CL_SUCCESS if success)
         * \return a \c Coal::Kernel object corresponding to the given \p name
         */
        Kernel *createKernel(const std::string &name, cl_int *errcode_ret);
        
        /**
         * \brief Create all the kernels of the program
         * \param errcode_ret return code (\c CL_SUCCESS if success)
         * \return the list of \c Coal::Kernel objects of this program
         */
        std::vector<Kernel *> createKernels(cl_int *errcode_ret);

        /**
         * \brief Device-specific program
         * \param device device for which the device-specific program is needed
         * \return the device-specific program requested, 0 if not found
         */
        DeviceProgram *deviceDependentProgram(DeviceInterface *device) const;
        std::string    deviceDependentCompilerOptions(DeviceInterface *device) const;

        /**
         * \brief Get information about this program
         * \copydetails Coal::DeviceInterface::info
         */
        cl_int info(cl_program_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const;

        /**
         * \brief Get build info about this program (log, binaries, etc)
         * \copydetails Coal::DeviceInterface::info
         * \param device \c Coal::DeviceInterface for which info is needed
         */
        cl_int buildInfo(DeviceInterface *device,
                         cl_program_build_info param_name,
                         size_t param_value_size,
                         void *param_value,
                         size_t *param_value_size_ret) const;

         std::string source() { return p_source; }

    private:
        Type        p_type;
        State       p_state;
        std::string p_source;

        llvm::LLVMContext   * p_llvmcontext;

        struct DeviceDependent
        {
            DeviceInterface * device;
            DeviceProgram   * program;
            std::string       unlinked_binary;
            bool              is_native_binary; // llvm kernel bitcode vs final native binary
            char            * native_binary_filename;  // if file exist already
            llvm::Module    * linked_module;
            Compiler        * compiler;
        };

        std::vector<DeviceDependent> p_device_dependent;
        DeviceDependent              p_null_device_dependent;

        void setDevices(cl_uint num_devices, DeviceInterface * const*devices);
	void resetDeviceDependent();
        DeviceDependent &deviceDependent(DeviceInterface *device);
        const DeviceDependent &deviceDependent(DeviceInterface *device) const;
        std::vector<llvm::Function *> kernelFunctions(DeviceDependent &dep);
};

}

#endif
