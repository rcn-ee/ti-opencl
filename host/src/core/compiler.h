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
 * \file compiler.h
 * \brief Compiler wrapper
 */

#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <string>

namespace Coal
{

class DeviceInterface;

/**
 * \brief Compiler
 *
 * This class runs an appropriate compiler and then retains compilation logs
 * and produced data.
 */
class Compiler
{
    public:
        /**
         * \brief Constructor
         * \param device \c Coal::DeviceInterface for which code will be compiled
         */
        Compiler(DeviceInterface *device);
        ~Compiler();

        bool CompileAndLink(const std::string &source,
                            const std::string &options,
                                  std::string &outfile);

        /**
         * \brief Compilation log
         * \note \c appendLog() can also be used to append custom info at the end
         *       of the log, for instance to keep compilation and linking logs
         *       in the same place
         * \return log
         */
        const std::string &log() const;

        /**
         * \brief Set options given at \c compile()
         * \return void
         */
        void set_options(const std::string &);

        /**
         * \brief Options given at \c compile()
         * \return options used during compilation
         */
        const std::string &options() const;

        /**
         * \brief Append a string to the log
         *
         * This function can be used to append linking or code-gen logs to the
         * internal compilation log kept by this class
         *
         * \param log log to be appended
         */
        void appendLog(const std::string &log);

    private:
        bool CompileAndLinkForDSP(const std::string &source,
                                  const std::string &options,
                                        std::string &outfile);

        DeviceInterface *p_device;
        std::string      p_log;
        std::string      p_options;
        bool             do_cache_kernels;
        bool             do_keep_files;
        bool             do_debug;
        bool             do_symbols;

};

}

#endif
