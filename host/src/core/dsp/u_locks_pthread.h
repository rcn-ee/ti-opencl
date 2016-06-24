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
*  @file    u_locks_pthread.h
*
*  @brief   TI implementation classes for mutual exclusion and locking.
*
*  @ingroup Utilities
*
*  @version 1.00.00
*
******************************************************************************/
#ifndef _U_LOCKS_PTHREAD_H_
#define _U_LOCKS_PTHREAD_H_

#include "../tiocl_thread.h"
                
/**************************************************************************//**
* @brief Simple mutex implemented using the pthreads library
*
* @details This mutex is simply a wrapper around a pthread mutex. Two regions 
*           of code cannot have the mutex locked at the same time.
*
******************************************************************************/
class Mutex
{
  public:
    Mutex()       { pthread_mutex_init   (&mutex, 0); } //!< Construct a mutex
    ~Mutex()      { pthread_mutex_destroy(&mutex);    } //!< Destruct a mutex
    void Lock()   { pthread_mutex_lock   (&mutex);    } //!< Lock a mutex
    void Unlock() { pthread_mutex_unlock (&mutex);    } //!< Unlock a mutex

    pthread_mutex_t* raw() { return &mutex; } //!< Return raw ptr to underlying

  private:
    pthread_mutex_t mutex;                    //!< The underlying pthread mutex

  private: // prevent copy construction and assignment
    Mutex(const Mutex &);               
    Mutex & operator = (const Mutex &);
};

/**************************************************************************//**
* @brief Simple condition variable implemented using the pthreads library.
*
* @details Condition variables are synchronization primitives that enable 
*           threads to wait until a particular condition occurs.  Condition 
*           variables enable threads to atomically release a lock and sleep. 
*           Condition variables support operations that "wake one" or 
*           "wake all" waiting threads. After a thread is woken, it 
*           re-acquires the lock it released when the thread entered the 
*           sleeping state.
*
******************************************************************************/
class CondVar
{
  public:

    CondVar()           { pthread_cond_init     (&cond, 0); } //!< Constructor
    ~CondVar()          { pthread_cond_destroy  (&cond);    } //!< Destructor

    /**********************************************************************//**
    * @brief Signal 1 of N threads waiting on the condition variable
    **************************************************************************/
    void notify_one()   { pthread_cond_signal   (&cond);    }

    /**********************************************************************//**
    * @brief Signal all N threads waiting on the condition variable
    **************************************************************************/
    void notify_all()   { pthread_cond_broadcast(&cond);    }

    /**********************************************************************//**
    * @brief Wait on the condition variable and release the passed mutex.
    **************************************************************************/
    void wait(Mutex* m) { pthread_cond_wait(&cond, m->raw()); }

  private:
    pthread_cond_t cond;    //!<  The underlying pthread condition variable

  private: // prevent copy construction and assignment
    CondVar(CondVar&);
    CondVar& operator=(CondVar&);
};

/**************************************************************************//**
* @brief Objects of this type lock the remainder of the enclosing scope.
*
* @details Declare one of these in a scope and pass a mutex reference and the 
*            mutex will be locked for the remainder of the scope. This is a 
*            safer way to lock and unlock a mutex, because the mutex will 
*            automatically be unlocked when the scope level is exited.  This
*            helps prevent an unlocked mutex from occuring during exceptions or 
*            forgotten early function returns.
*
******************************************************************************/
class ScopedLock
{
  public:
    ScopedLock(Mutex &m) : mutex(m) { mutex.Lock();   } //!< Constructor
    ~ScopedLock()                   { mutex.Unlock(); } //!< Destructor

  private:
    //mutable 
        Mutex& mutex;  //!< The Underlying mutex reference

  private: // prevent copy construction and assignment
    ScopedLock(const ScopedLock&);
    ScopedLock& operator=(const ScopedLock&);
};

#endif
