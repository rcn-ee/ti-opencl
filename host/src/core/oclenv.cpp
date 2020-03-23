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

#include <stdlib.h>
#include <errno.h>
#include "oclenv.h"

#include <CL/TI/cl.h>
#include "tiocl_thread.h"
#ifdef _SYS_BIOS
#include <Singleton.h>
#else
#define  LOKI_PTHREAD_H
#include <loki/Singleton.h>
#endif

using namespace tiocl;

// MCT-779: Set EnvVar's lifetime policy to NoDestroy, destroy it explicitly
// using a gcc destructor, __delete_theplatform(), after destroying Platform
// singleton object.  This is to avoid a race condition during early exit
// that EnvVar has been destroyed in main thread, but again used by worker
// thread.
#ifndef _SYS_BIOS
typedef Loki::SingletonHolder <tiocl::EnvVar, Loki::CreateUsingNew,
Loki::NoDestroy, Loki::ClassLevelLockable> SingleEnvVar;
#else
typedef Loki::SingletonHolder <tiocl::EnvVar, Loki::CreateUsingNew,
Loki::DefaultLifetime, Loki::SingleThreaded> SingleEnvVar;
#endif

bool EnvVar::constructed = false;

EnvVar::EnvVar() { constructed = true; }

/******************************************************************************
* Thread safe instance function for singleton behavior
******************************************************************************/
EnvVar& EnvVar::Instance()
{
  return SingleEnvVar::Instance();
}

namespace tiocl {

#define __ACCESS_STATUS_VAL(var, RET_T) \
template<> \
const char * EnvVar::GetName<EnvVar::Var:: var>() \
{ \
    return name_##var; \
}; \
template<> \
EnvVar::VarStatus EnvVar::GetStatus<EnvVar::Var:: var>() \
{ \
    return status_##var; \
}; \
template<> \
void EnvVar::SetStatus<EnvVar::Var:: var>(EnvVar::VarStatus status) \
{ \
  status_##var = status; \
} \
template<> \
RET_T EnvVar::GetVal<EnvVar::Var:: var, RET_T>() \
{ \
  return val_##var; \
} \
template<> \
void EnvVar::SetVal<EnvVar::Var:: var, RET_T>(RET_T val) \
{ \
  val_##var = val; \
} 

__ENV_VAR_LIST(__ACCESS_STATUS_VAL)

// Specialized getenv based on return type
template <>
EnvVar::VarStatus
EnvVar::GetEnv_Helper<char *>(const char *name, char *default_val, char **val)
{
  *val = getenv(name);
  if (*val == nullptr)
  {
    *val = default_val;
    return EnvVar::VarStatus::Queried_Undefined;
  }

  return EnvVar::VarStatus::Queried_Defined;
}

template <>
EnvVar::VarStatus
EnvVar::GetEnv_Helper<cl_int>(const char *name, cl_int default_val, cl_int *val)
{
  char *env = getenv(name);
  if (env == nullptr)
  {
    *val = default_val;
    return EnvVar::VarStatus::Queried_Undefined;
  }

  errno = 0; 
  char *tmp;
  *val = strtol(env, &tmp, 10);
  if (errno != 0 || tmp == env || *tmp != '\0')
  {
    *val = default_val;
    return EnvVar::VarStatus::Queried_Undefined;
  }

  return EnvVar::VarStatus::Queried_Defined;
}

template <>
EnvVar::VarStatus
EnvVar::GetEnv_Helper<cl_ulong>(const char *name, cl_ulong default_val,
                                   cl_ulong *val)
{
  char *env = getenv(name);
  if (env == nullptr)
  {
    *val = default_val;
    return EnvVar::VarStatus::Queried_Undefined;
  }

  errno = 0; 
  char *tmp;
  *val = strtoull(env, &tmp, 10);
  if (errno != 0 || tmp == env || *tmp != '\0')
  {
    *val = default_val;
    return EnvVar::VarStatus::Queried_Undefined;
  }

  return EnvVar::VarStatus::Queried_Defined;
}

} // namespace tiocl
