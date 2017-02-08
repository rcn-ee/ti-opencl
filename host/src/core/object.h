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
 * \file object.h
 * \brief Object tree
 */

#ifndef __REFCOUNTED_H__
#define __REFCOUNTED_H__

namespace Coal
{

/**
 * \brief Base class of all the Clover objects
 * 
 * This class implements functions needed by all the Clover objects, like
 * reference counting, the object tree (parents/children), etc.
 * 
 * It also uses a special list of known objects, used to check that a pointer
 * passed by the user to an OpenCL function actually is an object of the correct
 * type. See \c isA().
 */
class Object
{
    public:
        /**
         * \brief Type of object the inherited class actually is
         */
        enum Type
        {
            T_Invalid,      /* Invalid type */
            T_Device,       /*!< \brief \c Coal::DeviceInterface */
            T_CommandQueue, /*!< \brief \c Coal::CommandQueue */
            T_Event,        /*!< \brief \c Coal::Event */
            T_Context,      /*!< \brief \c Coal::Context */
            T_Kernel,       /*!< \brief \c Coal::Kernel */
            T_MemObject,    /*!< \brief \c Coal::MemObject */
            T_Program,      /*!< \brief \c Coal::Program */
            T_Sampler       /*!< \brief \c Coal::Sampler */
        };

        /**
         * \brief Constructor
         * \param type type of the child class calling this constructor
         * \param parent parent object
         */
        Object(Type type, Object *parent = 0);
        virtual ~Object();

        // Disable default constructor, copy constuction and assignment
        Object()                         =delete;
        Object(const Object&)            =delete;
        Object& operator=(const Object&) =delete;

        /**
         * \brief Increments the reference counter
         */
        void reference();
        
        /**
         * \brief Decrements the reference counter
         * \return true if the reference counter has reached 0
         */
        bool dereference();
        
        /**
         * \brief Reference counter
         * \return the number of references of this class currently in use
         */
        unsigned int references() const;
        
        /**
         * \brief Set if the parent object has to be deleted if its reference count reaches 0
         * 
         * The destructor of \c Coal::Object dereferences its parent object.
         * This is done in order to correctly free objects when no object has
         * a reference to it anymore.
         * 
         * Some objects such as \c Coal::CommandQueue need to do some operations
         * before being deleted. This function tells \c Coal::Object to
         * dereference its parent object, but not to call \b delete on it.
         *
         * \param release true to have \b delete called on the parent object
         *        when its reference count reaches 0, false to keep it
         */
        void setReleaseParent(bool release);

        Object *parent() const;    /*!< \brief Parent object */
        Type type() const;         /*!< \brief Type */

        /**
         * \brief Returns whether this object is an instance of \p type
         * \note This function begins with a NULL-check on the \c this pointer,
         *       so it's safe to use even when \c this is not guaranteed not to
         *       be NULL.
         * \param type type this object must have for the check to pass
         * \return true if this object exists and has the correct type
         */
        bool isA(Type type) const;

    private:
        Type p_type;
        unsigned int p_references;
        Object *p_parent;
        bool p_release_parent;
};

}

#endif
