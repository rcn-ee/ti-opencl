/******************************************************************************
 * Copyright (c) 2011-2014, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#ifndef _ICD_H
#define _ICD_H

#include <iostream>
#include <cassert>

#include "CL/TI/cl.h"

typedef void *(KHRicdVendorDispatch)[];
extern KHRicdVendorDispatch dispatch_table;

namespace Coal
{

// Templates to define the transformations between internal clover objects
// and external OpenCL C API objects expected by the ICD loader.
// This is based on the template system used by the clover state tracker of
// Mesa, at freedesktop.org
// See: src/gallium/state_trackers/clover/core/object.hpp

// Proxy template class that represents an OpenCL API object, and is the base
// class of all clover objects.
template<typename T, typename S>
struct descriptor {
    typedef T object_type;
    typedef S descriptor_type;

    descriptor() : dispatch(&dispatch_table) {
       static_assert(std::is_standard_layout<descriptor_type>::value,
                       "ICD requires CL API objects to be standard layout.");
   }

   const void *dispatch;
};

// Downcast an API object ptr to a Clover object ptr
// Note: clover object validation is done by subsequent ->isA methods.
template<typename D>
typename D::object_type *
pobj(D *d) {
    return static_cast<typename D::object_type *>(d);
}


// Upcast a Clover object ptr to an API object ptr.
template<typename O>
typename O::descriptor_type *
desc(O *o) {
    return static_cast<typename O::descriptor_type *>(o);
}

// Convert a list of API objects into Clover objects,
// copying into a previously allocated list of at least size n.
template<typename O, typename D>
void pobj_list(O **objs, D **descs, int n) {
    for (int i=0;i<n;i++) {
       objs[i] = pobj(descs[i]);
    }
}

// Const overload of above
template<typename O, typename D>
void pobj_list(O **objs, D * const *descs, int n) {
    for (int i=0;i<n;i++) {
      objs[i] = pobj(descs[i]);
    }
}

// Partial Vector overload of above
template<typename O, class D>
void pobj_list(O **objs, D const& descs) {
    for (int i=0;i<descs.size();i++) {
      objs[i] = pobj(descs[i]);
    }
}

// Vector overload of above
template<class O, class D>
void pobj_list(O& objs, D const& descs) {
    for (auto d : descs) {
      objs.push_back(pobj(d));
    }
}

// Convert a list of Clover objects to a list of API objects,
// copying into a previously allocated list of at least size n.
template<typename O, typename D>
void desc_list(D **descs, O **objs, int n) {
    for (int i=0;i<n;i++) {
       descs[i] = desc(objs[i]);
    }
}

// Const overload of above
template<typename O, typename D>
void desc_list(D **descs, O * const*objs, int n) {
    for (int i=0;i<n;i++) {
       descs[i] = desc(objs[i]);
    }
}

// Partial Vector overload of above
template<class O, typename D>
void desc_list(D **descs, O const& objs) {
    for (int i=0;i<objs.size();i++) {
      descs[i] = desc(objs[i]);
    }
}

// Vector overload of above
template<class O, class D>
void desc_list(D& descs, O const& objs) {
    for (auto o : objs) {
      descs.push_back(desc(o));
    }
}

}

#endif // _ICD_H

