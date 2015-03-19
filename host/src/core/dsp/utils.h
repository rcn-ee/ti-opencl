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
#ifndef __UTILS_H
#define __UTILS_H

/**
 * \brief Increment a n-component vector given a maximum value
 *
 * This function is used to increment a vector for which a set of maximum values
 * each of its element can reach before the next is incremented.
 *
 * For example, if \p dims is \c 3, \p vec starts at <tt>{0, 0, 0}</tt> and
 * \p maxs if <tt>{2, 3, 1}</tt>, repeatedly calling this function with the
 * same vector will produce the following results :
 *
 * \code
 * {0, 0, 1}
 * {0, 1, 0}
 * {0, 1, 1}
 * {0, 2, 0}
 * {0, 2, 1}
 * {0, 3, 0}
 * {0, 3, 1}
 * {1, 0, 0}
 * ...
 * \endcode
 *
 * Until \p vec reaches <tt>{2, 3, 1}</tt>.
 *
 * \param dims number of elements in the vectors
 * \param vec vector whose elements will be incremented
 * \param maxs vector containing a maximum value above which each corresponding
 *             element of \p vec cannot go.
 * \return false if the increment was ok, true if \p vec was already at it's
 *         maximum value and couldn't be further incremented.
 */
template<typename T>
bool incVec(unsigned long dims, T *vec, T *maxs)
{
    bool overflow = false;

    for (unsigned int i=0; i<dims; ++i)
    {
        vec[i] += 1;

        if (vec[i] > maxs[i])
        {
            vec[i] = 0;
            overflow = true;
        }
        else
        {
            overflow = false;
            break;
        }
    }

    return overflow;
}
#endif
