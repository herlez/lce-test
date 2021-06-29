/*******************************************************************************
 * src/sequential/bingmann-radix_sort.hpp
 *
 * Experiments with sequential radix sort implementations.
 * Based on rantala/msd_c?.h
 *
 *******************************************************************************
 * Copyright (C) 2013-2015 Timo Bingmann <tb@panthema.net>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef PSS_SRC_SEQUENTIAL_BINGMANN_RADIX_SORT_HEADER
#define PSS_SRC_SEQUENTIAL_BINGMANN_RADIX_SORT_HEADER

#include "../tools/globals.hpp"
#include "../tools/stringtools.hpp"
#include "../tools/stringset.hpp"
#include "inssort.hpp"

#include <stack>

namespace bingmann {

static const size_t g_inssort_threshold = 32;

typedef unsigned char* string;


/******************************************************************************/
// CI Variants
/******************************************************************************/

/******************************************************************************/
// msd_CI0

// Note: CI in-place variants cannot be done with just one prefix-sum bucket
// array, because during in-place permutation the beginning _and_ end boundaries
// of each bucket must be kept.


/******************************************************************************/
// msd_CI2

static inline size_t *
msd_CI2_run(string * strings, uint8_t * charcache, size_t n, size_t depth)
{
    // cache characters
    uint8_t* cc = charcache;
    for (string* s = strings; s != strings + n; ++s, ++cc)
        *cc = (*s)[depth];

    // cache and count character occurrences
    size_t* bkt_size = new size_t[256]();
    for (cc = charcache; cc != charcache + n; ++cc)
        ++bkt_size[*cc];

    // inclusive prefix sum
    size_t bkt[256];
    bkt[0] = bkt_size[0];
    size_t last_bkt_size = bkt_size[0];
    for (size_t i = 1; i < 256; ++i) {
        bkt[i] = bkt[i - 1] + bkt_size[i];
        if (bkt_size[i]) last_bkt_size = bkt_size[i];
    }

    // premute in-place
    for (size_t i = 0, j; i < n - last_bkt_size; )
    {
        string perm = strings[i];
        uint8_t permch = charcache[i];
        while ((j = --bkt[permch]) > i)
        {
            std::swap(perm, strings[j]);
            std::swap(permch, charcache[j]);
        }
        strings[i] = perm;
        i += bkt_size[permch];
    }

    return bkt_size;
}

static inline void msd_CI2(
    string* strings, uint8_t* charcache, size_t n, size_t depth)
{
    if (n < g_inssort_threshold)
        return inssort::inssort(strings, n, depth);

    size_t* bkt_size = msd_CI2_run(strings, charcache, n, depth);

    // recursion
    string* bsum = strings + bkt_size[0];
    for (size_t i = 1; i < 256; ++i) {
        if (bkt_size[i] > 1)
            msd_CI2(bsum, charcache, bkt_size[i], depth + 1);
        bsum += bkt_size[i];
    }

    delete[] bkt_size;
}

static inline void bingmann_msd_CI2(string* strings, size_t n)
{
    uint8_t* charcache = new uint8_t[n];
    msd_CI2(strings, charcache, n, 0);
    delete[] charcache;
}

//TODO: Stack Overflow
static inline void msd_CI(string* strings, size_t n, size_t depth)
{
    uint8_t* charcache = new uint8_t[n];
    msd_CI2(strings, charcache, n, depth);
    delete[] charcache;
}

} // namespace bingmann

#endif // !PSS_SRC_SEQUENTIAL_BINGMANN_RADIX_SORT_HEADER

/******************************************************************************/
