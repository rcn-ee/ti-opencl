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
#include "genfile_cache.h"

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>

std::string genfile_cache::lookup(const llvm::Module *module,
                                  const std::string &options)
{
    uint32_t hash = convert_mod2crc(module, options);
    return lookup(hash);
}

std::string genfile_cache::lookup(const std::string &source,
                                  const std::string &options)
{
    uint32_t hash = convert_src2crc(source, options);
    return lookup(hash);
}

std::string genfile_cache::lookup(uint32_t hash)
{
    std::vector<std::vector<std::string> > result;

    std::string query("select value from programs where hash = " +
              boost::lexical_cast<std::string>(hash));

    result = p_database.query(query.c_str());

    if (!result.empty())
    {
        string &filename = result[0][0];

        struct stat statbuf;
        if (stat(filename.c_str(), &statbuf) == 0)
            return filename;
        /*-----------------------------------------------------------------
        * if (the cached filename no longer exists, remove it from the DB
        *----------------------------------------------------------------*/
        else
        {
            std::string q2("delete from programs where hash = " +
                  boost::lexical_cast<std::string>(hash));

            p_database.query(q2.c_str());
            return std::string();
        }
    }
    else
        return std::string();
}

void genfile_cache::remember(const char *outfile, const llvm::Module *module,
                             const std::string &options)
{
    uint32_t hash = convert_mod2crc(module, options);
    remember(outfile, hash);
}

void genfile_cache::remember(const char *outfile, const std::string &source,
                             const std::string &options)
{
    uint32_t hash = convert_src2crc(source, options);
    remember(outfile, hash);
}

void genfile_cache::remember(const char *outfile, uint32_t hash)
{
    std::string query("insert into programs(hash, value) values("
                  + boost::lexical_cast<std::string>(hash)
                  + ", \""
                  + string(outfile)
                  + "\");");

    p_database.query(query.c_str());
}

uint32_t genfile_cache::convert_mod2crc(const llvm::Module *module,
                                        const std::string  &options)
{
    std::string llvm_ir;

    llvm::raw_string_ostream ostream(llvm_ir);
    llvm::WriteBitcodeToFile(module, ostream);
    ostream.str();

    llvm_ir += options;

    return get_crc(llvm_ir);
}

uint32_t genfile_cache::convert_src2crc(const std::string &source,
                                        const std::string &options)
{
    std::string source_options = source + options;
    return get_crc(source_options);
}

uint32_t genfile_cache::get_crc(const std::string& my_string)
{
    boost::crc_32_type result;
    result.process_bytes(my_string.data(), my_string.length());
    return result.checksum();
}

