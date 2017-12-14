/******************************************************************************
 * Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Texas Instruments Incorporated nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#pragma once

#include <CL/cl.h>

#define __ENV_VAR_LIST(__FUNC) \
  __FUNC(TI_OCL_CACHE_KERNELS,                          char *) \
  __FUNC(TI_OCL_COMPUTE_UNIT_LIST,                      char *) \
  __FUNC(TI_OCL_CPU_DEVICE_ENABLE,                      char *) \
  __FUNC(TI_OCL_CUSTOM_DEVICE_ENABLE,                   cl_int) \
  __FUNC(TI_OCL_DEBUG,                                  char *) \
  __FUNC(TI_OCL_DEVICE_PROGRAM_INFO,                    char *) \
  __FUNC(TI_OCL_DSP_1_25GHZ,                            char *) \
  __FUNC(TI_OCL_ENABLE_FP64,                            char *) \
  __FUNC(TI_OCL_INSTALL,                                char *) \
  __FUNC(TI_OCL_LIMIT_DEVICE_MAX_MEM_ALLOC_SIZE,      cl_ulong) \
  __FUNC(TI_OCL_KEEP_FILES,                             char *) \
  __FUNC(TI_OCL_KERNEL_TIMEOUT_COMPUTE_UNIT,            cl_int) \
  __FUNC(TI_OCL_PROFILING_EVENT_TYPE,                   cl_int) \
  __FUNC(TI_OCL_PROFILING_EVENT_NUMBER1,                cl_int) \
  __FUNC(TI_OCL_PROFILING_EVENT_NUMBER2,                cl_int) \
  __FUNC(TI_OCL_PROFILING_STALL_CYCLE_THRESHOLD,        cl_int) \
  __FUNC(TI_OCL_WG_SIZE_LIMIT,                          cl_int) \
  __FUNC(TI_OCL_WORKER_NICE,                            cl_int) \
  __FUNC(TI_OCL_WORKER_SLEEP,                           cl_int) \
  __FUNC(TARGET_ROOTDIR,                                char *) \


namespace tiocl {

// TI OpenCL environment variables
// - to be set by environment variables in Linux (or HLOS)
// - to be set by package parameter in TI-RTOS
class EnvVar
{
  public: 
    EnvVar();
    virtual ~EnvVar() {}

    enum Var
    {
      TI_OCL_CACHE_KERNELS = 0,
      TI_OCL_COMPUTE_UNIT_LIST,
      TI_OCL_CPU_DEVICE_ENABLE,
      TI_OCL_CUSTOM_DEVICE_ENABLE,
      TI_OCL_DEBUG,
      TI_OCL_DEVICE_PROGRAM_INFO,
      TI_OCL_DSP_1_25GHZ,
      TI_OCL_ENABLE_FP64,
      TI_OCL_INSTALL,
      TI_OCL_LIMIT_DEVICE_MAX_MEM_ALLOC_SIZE,
      TI_OCL_KEEP_FILES,
      TI_OCL_KERNEL_TIMEOUT_COMPUTE_UNIT,
      TI_OCL_PROFILING_EVENT_TYPE,
      TI_OCL_PROFILING_EVENT_NUMBER1,
      TI_OCL_PROFILING_EVENT_NUMBER2,
      TI_OCL_PROFILING_STALL_CYCLE_THRESHOLD,
      TI_OCL_WG_SIZE_LIMIT,
      TI_OCL_WORKER_NICE,
      TI_OCL_WORKER_SLEEP,
      TARGET_ROOTDIR,
    };

    enum VarStatus
    {
      Unqueried = 0,
      Queried_Defined,
      Queried_Undefined
    };

    template<Var var>           struct var_traits {};
    template<Var var>     const char * GetName();
    template<Var var>        VarStatus GetStatus();
    template<Var var>             void SetStatus(VarStatus status);
    template<Var var, typename T>    T GetVal();
    template<Var var, typename T> void SetVal(T val);

    template <Var var>
    typename var_traits<var>::return_type
    GetEnv(typename var_traits<var>::return_type default_val)
    {
       VarStatus status = GetStatus<var>();
       if (status == VarStatus::Queried_Undefined)
         return default_val;
       else if (status == VarStatus::Queried_Defined)
         return GetVal<var, typename var_traits<var>::return_type>();

       typename var_traits<var>::return_type val;
       SetStatus<var>(GetEnv_Helper(GetName<var>(), default_val, &val));
       SetVal<var>(val);
       return val;
    }

    // Disable copy construction and assignment
    EnvVar(const EnvVar&)            =delete;
    EnvVar& operator=(const EnvVar&) =delete;

    static EnvVar& Instance();

  private:
    #define __DECLARE_STATUS_VAL(var, RET_T) \
    const char *name_##var = #var; \
    VarStatus status_##var = VarStatus::Unqueried; \
    RET_T val_##var;

    __ENV_VAR_LIST(__DECLARE_STATUS_VAL)

    template <typename T>
    VarStatus GetEnv_Helper(const char *name, T default_val, T *val);

  public:
    static bool constructed;
};

#define __DECLARE_VAR_TRAITS(var, RET_T) \
template<> \
struct EnvVar::var_traits<EnvVar::Var:: var> \
{ \
  typedef RET_T return_type; \
};

__ENV_VAR_LIST(__DECLARE_VAR_TRAITS)

}  // namespace tiocl

