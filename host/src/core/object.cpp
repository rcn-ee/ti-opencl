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
 * \file object.cpp
 * \brief Reference-counted object tree
 */

#include "object.h"
#include "dsp/u_concurrent_set.h"

using namespace Coal;

/*-----------------------------------------------------------------------------
* This static was previously inside the getKnownObjects function in order to 
* delay its construction until first use.  Since we now delay the construction
* of the platform until first use, we need to make sure that known_objects 
* lifetime is a superset of the the_platform and all opencl objects lifetimes.
* Therefore we moved the definition of known_objects to global scope which 
* will ensure that it exists before the_platform and should also ensure that
* it is destroyed after the_platform, since objects are destructed in reverse
* order of construction.  Both singletons created with new and statics are 
* both placed in the same dtor queue.
*----------------------------------------------------------------------------*/
#ifdef _SYS_BIOS
static concurrent_set<Object *>& getKnownObjects()
{
    static concurrent_set<Object *> known_objects;
    return known_objects;
}
#else
static concurrent_set<Object *> known_objects;
#endif

Object::Object(Type type, Object *parent)
: p_references(1), p_parent(parent), p_type(type), p_release_parent(true)
{
    if (parent)
        parent->reference();

    // Add object in the list of known objects
#ifdef _SYS_BIOS
    getKnownObjects().insert(this);
#else
    known_objects.insert(this);
#endif
}

Object::~Object()
{
    if (p_parent && p_parent->dereference() && p_release_parent)
        delete p_parent;

    // Remove object from the list of known objects

#ifdef _SYS_BIOS
    getKnownObjects().erase(this);
#else
    known_objects.erase(this);
#endif
    p_type = T_Invalid;
}

void Object::reference()
{
    __sync_fetch_and_add(&p_references, 1);
}

bool Object::dereference()
{
    unsigned int oldval = __sync_fetch_and_sub(&p_references, 1);
    return (oldval == 1);
}

void Object::setReleaseParent (bool release)
{
    p_release_parent = release;
}

unsigned int Object::references() const
{
    return p_references;
}

Object *Object::parent() const
{
    return p_parent;
}

Object::Type Object::type() const
{
    return p_type;
}

bool Object::isA(Object::Type type) const
{
    // Check for null values
    if (this == 0) return false;

#ifdef _SYS_BIOS
    return getKnownObjects().memberp((Object *) this) && type == p_type;
#else
    return known_objects.memberp((Object *) this) && type == p_type;
#endif
}
