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
 * \file propertylist.h
 * \brief Helper macros for \c info() functions
 * 
 * The OpenCL API is full of functions like \c clGetXXXInfo(). They all take
 * the same arguments and are handled the same way. This file contains macros
 * easing the implementation of these info functions.
 * 
 * One info function, using these macros, looks like that:
 *
 * \code
 * cl_int Foo::info(cl_foo_info param_name,
 *                  size_t param_value_size,
 *                  void *param_value,
 *                  size_t *param_value_size_ret) const
 * {
 *     void *value = 0;
 *     size_t value_length = 0;
 * 
 *     union {
 *         cl_uint cl_uint_var;
 *         cl_context cl_context_var;
 *     };
 * 
 *     switch (param_name)
 *     {
 * 		case CL_UINT_PARAM:
 * 		    SIMPLE_ASSIGN(cl_uint, the_value);
 * 		    break;
 * 		case CL_CONTEXT_PARAM:
 * 		    SIMPLE_ASSIGN(cl_context, a_call());
 * 		    break;
 * 		case CL_STRING_PARAM:
 * 		    STRING_ASSIGN("This is a string");
 * 		    break;
 * 		case CL_BINARY_PARAM:
 * 		    MEM_ASSIGN(sizeof(something), something);
 * 		    break;
 *      default:
 *          return CL_INVALID_VALUE;
 *     }
 * 
 *     if (param_value && param_value_size < value_length)
 *         return CL_INVALID_VALUE;
 * 
 *     if (param_value_size_ret)
 *         *param_value_size_ret = value_length;
 * 
 *     if (param_value)
 *         std::memcpy(param_value, value, value_length);
 * 
 *     return CL_SUCCESS;
 * }
 * \endcode
 */

#ifndef __PROPERTYLIST_H__
#define __PROPERTYLIST_H__

/**
 * \brief Assign a value of a given type to the return value
 * \param type type of the argument
 * \param _value value to assign
 */
#define SIMPLE_ASSIGN(type, _value) do {    \
    value_length = sizeof(type);            \
    type##_var = (type)_value;              \
    value = & type##_var;                   \
} while (0);

/**
 * \brief Assign a string to the return value
 * \param string the string to assign, as a constant
 */
#define STRING_ASSIGN(string) do {          \
    static const char str[] = string;       \
    value_length = sizeof(str);             \
    value = (void *)str;                    \
} while (0);

/**
 * \brief Assign a memory buffer to the return value
 * \note the buffer must remain valid after the end of the \c info() call
 * \param size size of the buffer
 * \param buf buffer (of type <tt>void *</tt> for instance)
 */
#define MEM_ASSIGN(size, buf) do {          \
    value_length = size;                    \
    value = (void *)buf;                    \
} while (0);

#endif
