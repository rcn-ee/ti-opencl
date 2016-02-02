/******************************************************************************
 * Copyright (c) 2013-2016, Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef _genfile_cache_
#define _genfile_cache_

namespace llvm
{
    class Module;
}

#include <boost/lexical_cast.hpp>
#include <boost/crc.hpp>

#include <sys/stat.h>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdint.h>
#include "u_locks_pthread.h"
#include "database.h"

class genfile_cache
{
  public:
    std::string lookup   (const llvm::Module *module,
                          const std::string &options);
    std::string lookup   (const std::string  &source,
                          const std::string &options);

    void        remember (const char   *outfile, const llvm::Module *module,
                          const std::string  &options);
    void        remember (const char   *outfile, const std::string  &source,
                          const std::string &options);

    /*-------------------------------------------------------------------------
    * Thread safe instance function for singleton behavior
    *------------------------------------------------------------------------*/
    static genfile_cache* instance ()
    {
        static Mutex Cache_instance_mutex;
        genfile_cache* tmp = pInstance;

        __sync_synchronize();

        if (tmp == 0)
        {
            ScopedLock lck(Cache_instance_mutex);

            tmp = pInstance;
            if (tmp == 0)
            {
                char *user = getenv("USER");
                tmp = new genfile_cache("/tmp/opencl_ofdb_" + string(user));
                __sync_synchronize();
                pInstance = tmp;
            }
        }
        return tmp;
    }


  private:
    static genfile_cache* pInstance;
    std::string p_dbname;
    Database    p_database;

  private:
    genfile_cache(std::string db_name) :
                            p_dbname(db_name), p_database(db_name.c_str())
    {
        p_database.query("create table if not exists "
                "programs(hash integer, value string);");
    }

    std::string lookup          (uint32_t hash);
    void        remember        (const char *outfile, uint32_t hash);
    uint32_t    convert_mod2crc (const llvm::Module *module,
                                 const std::string &options);
    uint32_t    convert_src2crc (const std::string  &source,
                                 const std::string &options);
    uint32_t    get_crc         (const std::string& my_string);

    genfile_cache(const genfile_cache&);              // copy ctor disallowed
    genfile_cache& operator=(const genfile_cache&);   // assignment disallowed
};

#endif // _genfile_cache_
