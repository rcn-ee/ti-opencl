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
 * \file core/kernel.h
 * \brief Kernel
 */

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "object.h"
#include "icd.h"

#include <CL/cl.h>
#include <CL/cl_ext.h>

#include <vector>
#include <string>

namespace Coal
{
  class Kernel;
}
struct _cl_kernel: public Coal::descriptor<Coal::Kernel, _cl_kernel> {};

namespace llvm
{
    class Function;
    class Module;
}

namespace Coal
{

class Program;
class DeviceInterface;
class DeviceKernel;

/**
 * \brief Kernel
 * 
 * A kernel represents a LLVM function that can be run on a device. As 
 * \c Coal::Kernel objects are device-independent, they in fact represent only
 * the name of a kernel and the arguments the application wants to pass to it,
 * but it also contains a list of LLVM functions for each device for which its
 * parent \c Coal::Program has been built
 */
class Kernel : public _cl_kernel, public Object
{
    public:
        /**
         * \brief Constructor
         * \param program Parent \c Coal::Program
         */
        Kernel(Program *program);
        ~Kernel();

        /**
         * \brief Kernel argument
         * 
         * This class holds OpenCL-related information about the arguments of
         * a kernel. It is also used to check that a kernel takes the same
         * arguments on every device on which it has been built.
         */
        class Arg
        {
            public:
                /**
                 * \brief Memory address space qualifier
                 */
                enum File
                {
                    Private = 0,  /*!< \brief __private */
#if 1
                    Global = 1,   /*!< \brief __global */
                    Constant = 2, /*!< \brief __constant */
                    Local = 3     /*!< \brief __local */
#else
                    /* using clang defaults */
                    Global   = 0xFFFF00, /*!< \brief __global */
                    Local    = 0xFFFF01, /*!< \brief __local */
                    Constant = 0xFFFF02  /*!< \brief __constant */
#endif
                };

                /**
                 * \brief Kind of argument (its datatype)
                 */
                enum Kind
                {
                    Invalid,      /*!< \brief Invalid argument */
                    Int8,         /*!< \brief \c uchar or \c char, \c i8 in LLVM */
                    Int16,        /*!< \brief \c ushort or \c short, \c i16 in LLVM */
                    Int32,        /*!< \brief \c uint or \c int, \c i32 in LLVM */
                    Int64,        /*!< \brief \c ulong or \c long, \c i64 in LLVM */
                    Float,        /*!< \brief \c float, \c float in LLVM */
                    Double,       /*!< \brief \c double, \c double in LLVM */
                    Buffer,       /*!< \brief \c Coal::Buffer or \c Coal::SubBuffer, <tt>type*</tt> in LLVM */
                    Image2D,      /*!< \brief \c Coal::Image2D, <tt>\%struct.image2d*</tt> in LLVM */
                    Image3D,      /*!< \brief \c Coal::Image3D, <tt>\%struct.image3d*</tt> in LLVM */
                    Sampler       /*!< \brief \c Coal::Sampler::bitfield(), \c i32 in LLVM, see \c Coal::Kernel::setArg() */
                };

                /**
                 * \brief Constructor
                 * \param vec_dim vector dimension of the argument, 1 if not a vector
                 * \param file \c File of the argument
                 * \param kind \c Kind of the argument
                 */
                Arg(unsigned short vec_dim, File file, Kind kind, bool is_subword_int_uns);
                ~Arg();

                /**
                 * \brief Allocate the argument
                 * 
                 * This function must be called before \c loadData(). It 
                 * allocates a buffer in which the argument value can be stored.
                 * 
                 * \sa valueSize()
                 */
                void alloc();
                
                /**
                 * \brief Load a value into the argument
                 * \note \c alloc() must have been called before this function.
                 * \sa valueSize()
                 */
                void loadData(const void *data);
                
                /**
                 * \brief Set the number of bytes that must be allocated at run-time
                 * 
                 * \c __local arguments don't take a value given by the host
                 * application, but take pointers allocated on the device
                 * for each work-group.
                 * 
                 * This function allows to set the size of the device-allocated
                 * memory buffer used by this argument.
                 * 
                 * \param size size in byte of the buffer the device has to
                 *        allocate for each work-group of this kernel
                 */
                void setAllocAtKernelRuntime(size_t size);
                
                /**
                 * \brief Changes the \c Kind of this argument
                 * \param kind new \c Kind
                 */
                void refineKind(Kind kind);

                /**
                 * \brief Compares this argument with another
                 * 
                 * They are different if they \c vec_dim, \c file or \c kind are
                 * not the same.
                 *
                 * \param b other argument to compare
                 * \return true if the this arguments doesn't match \p b
                 */
                bool operator !=(const Arg &b);

                /**
                 * \brief Size of a field of this arg
                 * 
                 * This function returns the size of this argument based on its
                 * \c Kind
                 * 
                 * \note This size is not multiplied by \c vecDim(), you must do
                 *       this by yourself to find the total space taken by this 
                 *       arg.
                 * \return the size of this argument, in bytes, without any padding
                 */
                size_t valueSize() const;
                unsigned short vecDim() const;                 /*!< \brief Vector dimension */
                size_t vecValueSize() const;                   /*!< \brief Size of this whole arg/vector, padded */
                File file() const;                             /*!< \brief File */
                Kind kind() const;                             /*!< \brief Kind */
                bool defined() const;                          /*!< \brief Has the value of this argument already beed loaded by the host application ? */
                size_t allocAtKernelRuntime() const;           /*!< \brief Size of the \c __local buffer to allocate at kernel runtime */
                const void *value(unsigned short index) const; /*!< \brief Pointer to the value of this argument, for the \p index vector element */
                const void *data() const;                      /*!< \brief Pointer to the data of this arg, equivalent to <tt>value(0)</tt> */
                bool is_subword_int_uns() const ;

            private:
                unsigned short p_vec_dim;
                File p_file;
                Kind p_kind;
                void *p_data;
                bool p_defined;
                size_t p_runtime_alloc;
                bool p_is_subword_int_uns;
        };

        /**
         * \brief Add a \c llvm::Function to this kernel
         * 
         * This function adds a \c llvm::Function to this kernel for the
         * specified \p device. It also has the responsibility to find the
         * \c Arg::Kind of each of the function's arguments.
         * 
         * LLVM provides a \c llvm::Type for each argument:
         * 
         * - If it is a pointer, the kind of the argument is \c Arg::Buffer and
         *   its field is a simple cast from a LLVM \c addrspace to \c Arg::File.
         * - If it is a pointer to a struct whose name is either
         *   <tt>\%struct.image2d</tt> or <tt>\%struct.image3d</tt>, kind is set
         *   to \c Arg::Image2D or \c Arg::Image3D, respectively.
         * - If it is a vector, \c vec_dim is set to the vector size, and the
         *   rest of the computations are done on the element type
         * - Then we translate the LLVM type to an \c Arg::Kind. For instance,
         *   \c i32 becomes \c Arg::Int32
         * 
         * Samplers aren't detected at this stage because they are plain \c i32
         * types on the LLVM side. They are detected in \c setArg() when the
         * value being set to the argument appears to be a \c Coal::Sampler.
         * 
         * \param device device for which the function is added
         * \param function function to add
         * \param module LLVM module of this function
         */
        cl_int addFunction(DeviceInterface *device, llvm::Function *function,
                           llvm::Module *module);

        /**
         * \brief Get the LLVM function for a specified \p device
         * \param device the device for which a LLVM function is needed
         * \return the LLVM function for the given \p device
         */
        llvm::Function *function(DeviceInterface *device) const;
        
        /**
         * \brief Set the value of an argument
         * 
         * See the constructor's documentation for a note on the
         * \c Coal::Sampler objects
         * 
         * \param index index of the argument
         * \param size size of the value being stored in the argument, must match
         *        <tt>Arg::valueSize() * Arg::vecDim()</tt>
         * \param value pointer to the data that will be copied in the argument
         * \return \c CL_SUCCESS if success, an error code otherwise
         */
        cl_int setArg(cl_uint index, size_t size, const void *value);

        unsigned int numArgs() const;             /*!< \brief Number of arguments of this kernel */
        const Arg &arg(unsigned int index) const; /*!< \brief \c Arg at the given \p index */

        /*! \brief \c Coal::DeviceKernel for the specified \p device */
        DeviceKernel *deviceDependentKernel(DeviceInterface *device) const;
        llvm::Module *deviceDependentModule(DeviceInterface *device) const;

        bool argsSpecified() const;               /*!< \brief true if all the arguments have been set through \c setArg() */
        bool hasLocals() const;                   /*!< \brief true if one or more argument is in file \c Arg::Local */

        /**
         * \brief Get information about this kernel
         * \copydetails Coal::DeviceInterface::info
         */
        cl_int info(cl_kernel_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const;


        /**
         * \brief Get performance hints and device-specific data about this kernel
         * \copydetails Coal::DeviceInterface::info
         * \param device \c Coal::DeviceInterface on which the kernel will be run
         */
        cl_int workGroupInfo(DeviceInterface *device,
                             cl_kernel_work_group_info param_name,
                             size_t param_value_size,
                             void *param_value,
                             size_t *param_value_size_ret) const;

        void reqdWorkGroupSize(llvm::Module *module, cl_uint dims[3]) const;

        int get_wi_alloca_size() { return wi_alloca_size; }

    private:
        std::string p_name;
        bool p_has_locals;
        int wi_alloca_size;

        struct DeviceDependent
        {
            DeviceInterface *device;
            DeviceKernel    *kernel;
            llvm::Function  *function;
            llvm::Module    *module;
        };

        std::vector<DeviceDependent> p_device_dependent;
        std::vector<Arg> p_args;
        DeviceDependent null_dep;

        const DeviceDependent &deviceDependent(DeviceInterface *device) const;
        DeviceDependent &deviceDependent(DeviceInterface *device);

};

}

#endif
