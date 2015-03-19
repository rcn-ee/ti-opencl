/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
/**************************************************************************//**
*
*  @file    u_concurrent_set.h
*  @brief   TI implementation class that implements a thread safe set.
*
******************************************************************************/
#ifndef _U_CONCURRENT_SET_H_
#define _U_CONCURRENT_SET_H_

#include <iostream>
#include <set>
#include "u_lockable.h"

/**************************************************************************//**
* @class concurrent_set
*
* @brief A thread safe set implementation
*
* @details This implementation wraps a standard stl set with some locking 
*          capability to make the member functions mutually exclusive 
*          regions.  In derives from the class Lockable which defines a type 
*          Lock that can be used to define a type in a scope.  The result will
*          be that the remainder of the scope (or until unlock is called) is a 
*          mutex.
*
******************************************************************************/
template<typename T>
class concurrent_set : public Lockable
{
public:
    concurrent_set() : S() {}
    ~concurrent_set() {}

    void insert (T const data) { Lock lock(this); S.insert(data); }
    void erase  (T data)       { Lock lock(this); S.erase(data);  }
    bool memberp(T data) const 
    { Lock lock(this); return S.find(data) != S.end(); }

    /*-------------------------------------------------------------------------
    * The class's data
    *------------------------------------------------------------------------*/
private:
    std::set<T> S;        //!< standard stl set

    /*-------------------------------------------------------------------------
    * Prevent copy construction and assignment
    *------------------------------------------------------------------------*/
private: 
    concurrent_set(const concurrent_set&);
    concurrent_set& operator=(const concurrent_set&);
};

#endif //_U_CONCURRENT_SET_H_
