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

#ifdef _SYS_BIOS
#include <Singleton.h>
#else
#define  LOKI_PTHREAD_H
#include <loki/Singleton.h>
#endif


// Wrap known_objects in a Loki singleton to ensure its lifetime is
// longer than any OpenCL objects tracked by known_objects.
#ifndef _SYS_BIOS
typedef Loki::SingletonHolder<concurrent_set<Object *>, Loki::CreateUsingNew,
                               Loki::NoDestroy, Loki::ClassLevelLockable> the_known_objects;
#else
typedef Loki::SingletonHolder<concurrent_set<Object *>, Loki::CreateUsingNew,
                               Loki::DefaultLifetime, Loki::SingleThreaded> the_known_objects;
#endif

__attribute__((destructor)) static void __delete_the_known_objects()
{
    delete  &the_known_objects::Instance();
}

Object::Object(Type type, Object *parent)
: p_references(1), p_parent(parent), p_type(type), p_release_parent(true)
{
    if (parent)
        parent->reference();

    // Add object in the list of known objects
    the_known_objects::Instance().insert(this);
}

Object::~Object()
{
    if (p_parent && p_parent->dereference() && p_release_parent)
        delete p_parent;

    // Remove object from the list of known objects
    the_known_objects::Instance().erase(this);
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

    return the_known_objects::Instance().memberp((Object *) this) && type == p_type;
}
