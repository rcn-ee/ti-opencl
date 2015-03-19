/******************************************************************************
* The Loki Library
* Copyright (c) 2001 by Andrei Alexandrescu
* This code accompanies the book:
* Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
*     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
* Permission to use, copy, modify, distribute and sell this software for any
*     purpose is hereby granted without fee, provided that the above copyright
*     notice appear in all copies and that both that copyright notice and this
*     permission notice appear in supporting documentation.
* The author or Addison-Wesley Longman make no representations about the
*     suitability of this software for any purpose. It is provided "as is"
*     without express or implied warranty.
******************************************************************************/

/**************************************************************************//**
*
*  Copyright (c) 2010, Texas Instruments Incorporated
*
*  All rights reserved. Property of Texas Instruments Incorporated.
*  Restricted rights to use, duplicate or disclose this code are
*  granted through contract.
*
*  ============================================================================
*
*  @file    u_lockable.h
*
*  @brief   Defines a base class that provides a derived class with a Lock type.
*
*  @version 1.00.00
*
*  @note    The Locakable class is a modified version of the ObjectLevelLockable
*           class from the LOKI library.  The copyright from that library is 
*           also included at the bottom of this file. 
*
******************************************************************************/
#ifndef _U_LOCKABLE_H_
#define _U_LOCKABLE_H_
#include "u_locks_pthread.h"
                
/**************************************************************************//**
* @brief used as a base class to give your derived class a Lock type.
* @details Have a class derive from this class and you can lock member 
*           functions of your class by defining a lock like this   
*           Lock lock(this);
******************************************************************************/
class Lockable
{
  public:
    Lockable()                : mutex() {}  //!< Default Constructor
    Lockable(const Lockable&) : mutex() {}  //!< Copy Constructor
    ~Lockable()                         {}  //!< Destructor

    /**********************************************************************//**
    * @brief The Lock type defined by inheriting from Lockable.
    **************************************************************************/
    class Lock
    { 
      public:

        /*******************************************************************//**
        * @brief Constructing a Lock object will lock the parent object's mutex
        ***********************************************************************/
        explicit Lock(const Lockable* host_) : host(*host_) 
                      { host.mutex.Lock();   }

        /*******************************************************************//**
        * @brief Destructing a Lock object will unlock the parent object's mutex
        ***********************************************************************/
        ~Lock()       { host.mutex.Unlock(); }

        /*******************************************************************//**
        * @brief Unlock the parent object's mutex
        ***********************************************************************/
        void unlock() { host.mutex.Unlock(); }

        /*******************************************************************//**
        * @brief Return a raw pointer to the parent object's mutex
        ***********************************************************************/
        Mutex* raw()  { return &host.mutex;  }

      private:
        const Lockable& host;       //!< a pointer back to the parent object

      private: // prevent copy construction and assignment
        Lock(const Lock&);
        Lock& operator=(const Lock&);
    };

  protected:
    mutable Mutex mutex;
};

/*-----------------------------------------------------------------------------
* Can use to turn off locking without chaning client code using Lockable
*----------------------------------------------------------------------------*/
class Lockable_off
{
  public:
    Lockable_off() {}

    class Lock
    { 
      public:

        explicit Lock(const Lockable_off* host_) { }
        void unlock() { }

      private: // prevent copy construction and assignment
        Lock(const Lock&);
        Lock& operator=(const Lock&);
    };
};

#endif
