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
*  @file    u_concurrent_stack.h
*  @brief   TI implementation class that implements a thread safe stack.
*
******************************************************************************/
#ifndef _U_CONCURRENT_STACK_H_
#define _U_CONCURRENT_STACK_H_

#include <iostream>
#include <stack>
#include "u_lockable.h"

/**************************************************************************//**
* @class concurrent_stack
*
* @brief A thread safe stack implementation
*
* @details This implementation wraps a standard stl stack with some locking 
*          capability to make the member functions mutually exclusive 
*          regions.  In derives from the class Lockable which defines a type 
*          Lock that can be used to define a type in a scope.  The result will
*          be that the remainder of the scope (or until unlock is called) is a 
*          mutex.
*
******************************************************************************/
template<typename T>
class concurrent_stack : public Lockable
{
public:
    concurrent_stack() : S(), num_elements(0) {}
    ~concurrent_stack() {}

    /**********************************************************************//**
    * @brief Place an object in the stack.
    * @param data is the item to psh on the stack
    ***************************************************************************/
    void push(T const data)
    {
        Lock lock(this);
        S.push(data);
        num_elements++;
    }

    /**********************************************************************//**
    * @brief How many elements are in the stack.
    * @returns The number of elements in the stack.
    ***************************************************************************/
    int size() const
    {
        Lock lock(this);
        return num_elements;
    }

    /**********************************************************************//**
    * @brief Determine if the stack is empty.
    * @returns true if the stack is empty, otherwise false.
    ***************************************************************************/
    bool empty() const
    {
        Lock lock(this);
        return (num_elements == 0);
    }

    /**********************************************************************//**
    * @brief Attempt to pop an item off the stack.
    * @param popped_value is an output parameter that contains the object popped
    *         if the stack is successfully popped.
    * @returns true if a value is popped, otherwise false
    ***************************************************************************/
    bool pop(T& popped_value)
    {
        Lock lock(this);
        if (num_elements == 0) return false;

        popped_value = S.top();
        S.pop();
        num_elements--;
        return true;
    }

    /*-------------------------------------------------------------------------
    * The class's data
    *------------------------------------------------------------------------*/
private:
    std::stack<T> S;        //!< standard stl stack
    int num_elements;

    /*-------------------------------------------------------------------------
    * Prevent copy construction and assignment
    *------------------------------------------------------------------------*/
private: 
    concurrent_stack(const concurrent_stack&);
    concurrent_stack& operator=(const concurrent_stack&);
};

#endif //_U_CONCURRENT_STACK_H_
