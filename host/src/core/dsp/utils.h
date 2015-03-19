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
