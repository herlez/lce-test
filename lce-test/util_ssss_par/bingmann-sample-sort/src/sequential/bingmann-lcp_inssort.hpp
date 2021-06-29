/*******************************************************************************
 * src/sequential/bingmann-lcp_inssort.hpp
 *
 * LCP-aware insertion sort, used in parallel variants as base case.
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

#ifndef PSS_SRC_SEQUENTIAL_BINGMANN_LCP_INSSORT_HEADER
#define PSS_SRC_SEQUENTIAL_BINGMANN_LCP_INSSORT_HEADER

#include <cstring>

#include "../tools/stringtools.hpp"
#include <tlx/die.hpp>

namespace bingmann {

using namespace stringtools;

//! LCP insertion sort
template <bool SaveCache, typename StringSet>
static inline
void lcp_insertion_sort(
    const StringSet& str, uintptr_t* lcp, typename StringSet::Char* cache,
    size_t depth)
{
    typedef typename StringSet::String String;
    typedef typename StringSet::Iterator Iterator;
    typedef typename StringSet::CharIterator CharIterator;

    Iterator begin = str.begin();
    size_t n = str.size();

    if (n <= 1) return;

    for (size_t j = 0; j < n - 1; ++j)
    {
        // insert strings[j] into sorted strings[0..j-1]

        String new_str = std::move(str[begin + j]);
        size_t new_lcp = depth; // start with LCP depth

        size_t i = j;
        while (i > 0)
        {
            size_t prev_lcp = new_lcp;

            String cur_str = std::move(str[begin + i - 1]);
            size_t cur_lcp = lcp[i];

            if (cur_lcp < new_lcp)
            {
                // CASE 1: lcp goes down -> insert string

                // move comparison string back
                str[begin + i - 1] = std::move(cur_str);
                break;
            }
            else if (cur_lcp == new_lcp)
            {
                // CASE 2: compare more characters

                CharIterator c1 = str.get_chars(new_str, new_lcp);
                CharIterator c2 = str.get_chars(cur_str, new_lcp);

                while (str.is_equal(new_str, c1, cur_str, c2))
                    ++c1, ++c2, ++new_lcp;

                // if (new_str >= curr_str) -> insert string
                if (!str.is_less(new_str, c1, cur_str, c2))
                {
                    // update lcp of prev (smaller string) with inserted string
                    lcp[i] = new_lcp;
                    if (SaveCache)
                        cache[i] = str.is_end(new_str, c1) ? 0 : *c1;
                    // lcp of inserted string with next string
                    new_lcp = prev_lcp;

                    // move comparison string back
                    str[begin + i - 1] = std::move(cur_str);
                    break;
                }
            }
            // else (cur_lcp > new_lcp), CASE 3: nothing to do

            str[begin + i] = std::move(cur_str);
            if (SaveCache)
                cache[i] = cache[i - 1];

            lcp[i + 1] = cur_lcp;

            --i;
        }

        str[begin + i] = std::move(new_str);
        lcp[i + 1] = new_lcp;
        if (SaveCache)
            cache[i + 1] = str.get_char(str[begin + i + 1], new_lcp);
    }

    // last loop specialized with checks for out-of-bound access to lcp.
    {
        size_t j = n - 1;

        // insert strings[j] into sorted strings[0..j-1]

        String new_str = std::move(str[begin + j]);
        size_t new_lcp = depth; // start with LCP depth

        size_t i = j;
        while (i > 0)
        {
            size_t prev_lcp = new_lcp;

            String cur_str = std::move(str[begin + i - 1]);
            size_t cur_lcp = lcp[i];

            if (cur_lcp < new_lcp)
            {
                // CASE 1: lcp goes down -> insert string

                // move comparison string back
                str[begin + i - 1] = std::move(cur_str);
                break;
            }
            else if (cur_lcp == new_lcp)
            {
                // CASE 2: compare more characters

                CharIterator c1 = str.get_chars(new_str, new_lcp);
                CharIterator c2 = str.get_chars(cur_str, new_lcp);

                while (str.is_equal(new_str, c1, cur_str, c2))
                    ++c1, ++c2, ++new_lcp;

                // if (new_str >= curr_str) -> insert string
                if (!str.is_less(new_str, c1, cur_str, c2))
                {
                    // update lcp of prev (smaller string) with inserted string
                    lcp[i] = new_lcp;
                    if (SaveCache)
                        cache[i] = str.is_end(new_str, c1) ? 0 : *c1;
                    // lcp of inserted string with next string
                    new_lcp = prev_lcp;

                    // move comparison string back
                    str[begin + i - 1] = std::move(cur_str);
                    break;
                }
            }
            // else (cur_lcp > new_lcp), CASE 3: nothing to do

            str[begin + i] = std::move(cur_str);
            if (SaveCache)
                cache[i] = cache[i - 1];

            if (i + 1 < n) // check out-of-bounds copy
                lcp[i + 1] = cur_lcp;

            --i;
        }

        str[begin + i] = std::move(new_str);

        if (i + 1 < n) { // check out-of-bounds save
            lcp[i + 1] = new_lcp;
            if (SaveCache)
                cache[i + 1] = str.get_char(str[begin + i + 1], new_lcp);
        }
    }
}

//! LCP insertion sort, without distinguishing character cache output
template <typename StringSet>
static inline
void lcp_insertion_sort(const StringSet& str, uintptr_t* lcp, size_t depth)
{
    return lcp_insertion_sort</* SaveCache */ false>(
        str, lcp, reinterpret_cast<typename StringSet::Char*>(NULL), depth);
}

//! LCP insertion sort, but immediately discard the lcp
template <typename StringSet>
static inline
void lcp_insertion_sort_nolcp(const StringSet& ss, size_t depth)
{
    uintptr_t tmp_lcp[ss.size()];
    return lcp_insertion_sort</* SaveCache */ false, StringSet>(
        ss, tmp_lcp, reinterpret_cast<typename StringSet::Char*>(NULL), depth);
}

} // namespace bingmann

#endif // !PSS_SRC_SEQUENTIAL_BINGMANN_LCP_INSSORT_HEADER

/******************************************************************************/
