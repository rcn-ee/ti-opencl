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
*  @file    u_concurrent_map.h
*  @brief   TI implementation class that implements a thread safe map.
*
******************************************************************************/
#ifndef _U_CONCURRENT_MAP_H_
#define _U_CONCURRENT_MAP_H_

#include <iostream>
#include <map>
#include "u_lockable.h"

/**************************************************************************//**
* @class concurrent_map
*
* @brief A thread safe map implementation
*
* @details This implementation wraps a standard stl map with some locking 
*          capability to make the member functions mutually exclusive 
*          regions.  In derives from the class Lockable which defines a type 
*          Lock that can be used to define a type in a scope.  The result will
*          be that the remainder of the scope (or until unlock is called) is a 
*          mutex.
*
******************************************************************************/
template<typename I, typename T>
class concurrent_map : public Lockable
{
public:
    concurrent_map() : M(), num_elements(0) {}
    ~concurrent_map() {}

    /**********************************************************************//**
    * @brief Place an object in the map.
    * @param data is the item to psh on the map
    ***************************************************************************/
    void push(I index, T const data, unsigned cnt = 1)
    {
        Lock lock(this);
        M[index] = std::pair<T,unsigned int>(data, cnt);
        num_elements++;
    }

    /**********************************************************************//**
    * @brief How many elements are in the map.
    * @returns The number of elements in the map.
    ***************************************************************************/
    int size() const
    {
        Lock lock(this);
        return num_elements;
    }

    /**********************************************************************//**
    * @brief Determine if the map is empty.
    * @returns true if the map is empty, otherwise false.
    ***************************************************************************/
    bool empty() const
    {
        Lock lock(this);
        return (num_elements == 0);
    }

    /**********************************************************************//**
    * @brief Attempt to pop an item off the map.
    * @param popped_value is an output parameter that contains the object popped
    *         if the map is successfully popped.
    * @returns true if a value is popped, otherwise false
    ***************************************************************************/
    bool try_pop(I idx, T& popped_value)
    {
        Lock lock(this);
        auto it = M.find(idx);

        if (it != M.end() && --it->second.second == 0)
        {
            popped_value = it->second.first;
            M.erase (it);
            num_elements--;
            return true;
        }


        return false;
    }

    void dump()
    {
       for (auto &i : M) 
           std::cout << i.first << " ==> " << i.second.first 
                     << "(" << i.second.second << ")" 
                     << std::endl;
    }

    /*-------------------------------------------------------------------------
    * The class's data
    *------------------------------------------------------------------------*/
private:
    std::map<I, std::pair<T, unsigned int>> M;  //!< standard stl map
    int num_elements;

    /*-------------------------------------------------------------------------
    * Prevent copy construction and assignment
    *------------------------------------------------------------------------*/
private: 
    concurrent_map(const concurrent_map&);
    concurrent_map& operator=(const concurrent_map&);
};

#endif //_U_CONCURRENT_MAP_H_
