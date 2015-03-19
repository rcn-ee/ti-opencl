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
 * \file api_gl.cpp
 * \brief OpenGL bindings (unimplemented)
 */

#define GL_GLEXT_PROTOTYPES
#include "GL/gl.h"
#include "GL/glext.h"

#include "CL/cl.h"
#include "CL/cl_gl.h"

cl_mem
clCreateFromGLBuffer(cl_context     context,
                     cl_mem_flags   flags,
                     GLuint         bufobj,
                     int *          errcode_ret)
{
   return 0;
}

cl_mem
clCreateFromGLTexture2D(cl_context      context,
                        cl_mem_flags    flags,
                        GLenum          target,
                        GLint           miplevel,
                        GLuint          texture,
                        int *           errcode_ret)
{
   return 0;
}

cl_mem
clCreateFromGLTexture3D(cl_context      context,
                        cl_mem_flags    flags,
                        GLenum          target,
                        GLint           miplevel,
                        GLuint          texture,
                        int *           errcode_ret)
{
   return 0;
}

cl_mem
clCreateFromGLRenderbuffer(cl_context   context,
                           cl_mem_flags flags,
                           GLuint       renderbuffer,
                           int *        errcode_ret)
{
   return 0;
}

cl_int
clGetGLObjectInfo(cl_mem                memobj,
                  cl_gl_object_type *   gl_object_type,
                  GLuint *              gl_object_name)
{
   return 0;
}

cl_int
clGetGLTextureInfo(cl_mem               memobj,
                   cl_gl_texture_info   param_name,
                   size_t               param_value_size,
                   void *               param_value,
                   size_t *             param_value_size_ret)
{
   return 0;
}

cl_int
clEnqueueAcquireGLObjects(cl_command_queue      command_queue,
                          cl_uint               num_objects,
                          const cl_mem *        mem_objects,
                          cl_uint               num_events_in_wait_list,
                          const cl_event *      event_wait_list,
                          cl_event *            event)
{
   return 0;
}

cl_int
clEnqueueReleaseGLObjects(cl_command_queue      command_queue,
                          cl_uint               num_objects,
                          const cl_mem *        mem_objects,
                          cl_uint               num_events_in_wait_list,
                          const cl_event *      event_wait_list,
                          cl_event *            event)
{
   return 0;
}
