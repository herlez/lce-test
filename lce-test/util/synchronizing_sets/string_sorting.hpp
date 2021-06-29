/*******************************************************************************
 * /string_sorting.hpp
 *
 * Based on:
 *
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

#pragma once

#include "indexed_string.hpp"
#include <stack>

namespace ssss_lce {
static inline uint16_t get_char(indexed_string str, size_t depth) {
    uint16_t v = 0;
    if (str[depth] == 0) return v;
    v |= (uint16_t(str[depth]) << 8);
    v |= (uint16_t(str[depth + 1]) << 0);
    return v;
}


static void inline inssort(indexed_string* strings, size_t n,
                           int64_t depth = 0) {
  if (n <= 1) {
    return;
  }

  size_t begin = 0;
  size_t j = 0;
  for (size_t i = begin + 1; --n != 0; ++i) {
    indexed_string tmp = strings[i];
    j = i;

    while (j != begin) {
      auto* s = strings[j - 1].string() + depth;
      auto* t = tmp.string() + depth;
      while (*s != 0 && *s == *t) {
        ++s, ++t;
      }
      if (*s <= *t) {
        break;
      }
      strings[j] = strings[j - 1];
      --j;
    }
    strings[j] = tmp;
  }
}

template <size_t IS_THRESHOLD = 32>
void inline msd_CE0(indexed_string* strings, indexed_string* sorted, size_t n,
                    uint64_t depth) {
  if (n <= 1) {
    return;
  }

  if (n <= IS_THRESHOLD) {
    inssort(strings, n, depth);
    return;
  }

  constexpr size_t max_char = std::numeric_limits<uint8_t>::max() + 1;
  std::array<size_t, max_char> bucket_sizes = { 0 };
  for (size_t i = 0; i < n; ++i) {
    ++bucket_sizes[strings[i][depth]];
  }

  {
    std::array<indexed_string*, max_char> buckets;
    buckets[0] = sorted;
    for (size_t i = 1; i < max_char; ++i) {
      buckets[i] = buckets[i - 1] + bucket_sizes[i - 1];
    }
    for (auto* cur_string = strings; cur_string < strings + n; ++cur_string) {
      *(buckets[(*cur_string)[depth]]++) = *cur_string;
    }
    std::copy_n(sorted, n, strings);
  }
  auto* bucket_border = strings + bucket_sizes[0];
  for (size_t i = 1; i < max_char; ++i) {
    if (bucket_sizes[i] > 0) {
      msd_CE0(bucket_border, sorted, bucket_sizes[i], depth + 1);
      bucket_border += bucket_sizes[i];
    }
  }

}

inline void msd_CE0(indexed_string* strings, size_t n) {
  std::vector<indexed_string> buffer(n);
  msd_CE0(strings, buffer.data(), n, 0);
}

struct RadixStep_CI2_sb
{
  indexed_string * str;
  size_t idx;
  size_t bkt_size[256];

  RadixStep_CI2_sb(indexed_string* strings, size_t n, size_t depth, uint8_t* charcache)
  {
    // cache characters
    uint8_t* cc = charcache;
    for (indexed_string* s = strings; s != strings + n; ++s, ++cc)
      *cc = (*s)[depth];

    // count character occurances
    memset(bkt_size, 0, sizeof(bkt_size));
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
        indexed_string perm = strings[i];
        uint8_t permch = charcache[i];
        while ((j = --bkt[permch]) > i)
          {
            std::swap(perm, strings[j]);
            std::swap(permch, charcache[j]);
          }
        strings[i] = perm;
        i += bkt_size[permch];
      }

    str = strings + bkt_size[0];
    idx = 0; // will increment to 1 on first process, bkt 0 is not sorted further
  }
};

template <size_t IS_THRESHOLD = 32>
static inline void
bingmann_msd_CI2_sb(indexed_string* strings, size_t n, size_t depth,
                    uint8_t* charcache)
{
  typedef RadixStep_CI2_sb RadixStep;

  std::stack<RadixStep, std::vector<RadixStep> > radixstack;
  radixstack.emplace(strings, n, depth, charcache);

  while (TLX_LIKELY(!radixstack.empty()))
    {
      while (TLX_LIKELY(radixstack.top().idx < 255))
        {
          RadixStep& rs = radixstack.top();
          ++rs.idx; // process the bucket rs.idx

          if (TLX_UNLIKELY(rs.bkt_size[rs.idx] == 0))
            ;
          else if (TLX_UNLIKELY(rs.bkt_size[rs.idx] < IS_THRESHOLD))
            {
              inssort(rs.str, rs.bkt_size[rs.idx],
                      depth + radixstack.size());
              rs.str += rs.bkt_size[rs.idx];
            }
          else
            {
              // have to increment first, as rs may be invalidated
              rs.str += rs.bkt_size[rs.idx];
              radixstack.emplace(
                                 rs.str - rs.bkt_size[rs.idx], rs.bkt_size[rs.idx],
                                 depth + radixstack.size(), charcache);
            }
        }
      radixstack.pop();
    }
}

template <size_t IS_THRESHOLD = 32>
static inline void
bingmann_msd_CI2_sb(indexed_string* strings, size_t n)
{
  if (n < IS_THRESHOLD)
    return inssort(strings, n, 0);

  uint8_t* charcache = new uint8_t[n];

  bingmann_msd_CI2_sb(strings, n, /* depth */ 0, charcache);

  delete[] charcache;
}

struct RadixStep_CI3_sb
{
  static const size_t RADIX = 0x10000;

  indexed_string              * str;
  size_t              idx;
  size_t              bkt_size[RADIX];

  RadixStep_CI3_sb(indexed_string* strings, size_t n, size_t depth, uint16_t* charcache)
  {
    // read characters and count character occurrences
    memset(bkt_size, 0, sizeof(bkt_size));
    uint16_t* cc = charcache;
    for (indexed_string* s = strings; s != strings + n; ++s, ++cc)
      *cc = get_char(*s, depth);
    for (cc = charcache; cc != charcache + n; ++cc)
      ++bkt_size[*cc];

    // inclusive prefix sum
    size_t bkt[RADIX];
    bkt[0] = bkt_size[0];
    size_t last_bkt_size = bkt_size[0];
    for (size_t i = 1; i < RADIX; ++i) {
      bkt[i] = bkt[i - 1] + bkt_size[i];
      if (bkt_size[i]) last_bkt_size = bkt_size[i];
    }

    // premute in-place
    for (size_t i = 0, j; i < n - last_bkt_size; )
      {
        indexed_string perm = strings[i];
        uint16_t permch = charcache[i];
        while ((j = --bkt[permch]) > i)
          {
            std::swap(perm, strings[j]);
            std::swap(permch, charcache[j]);
          }
        strings[i] = perm;
        i += bkt_size[permch];
      }

    str = strings + bkt_size[0];
    idx = 0; // will increment to 1 on first process, bkt 0 is not sorted further
  }
};

template <size_t IS_THRESHOLD = 32>
static inline void
bingmann_msd_CI3_sb(indexed_string* strings, size_t n)
{
  static const size_t RADIX = 0x10000;

  if (n < IS_THRESHOLD)
    return inssort(strings, n, 0);

  if (n < RADIX)
    return bingmann_msd_CI2_sb(strings, n);

  typedef RadixStep_CI3_sb RadixStep;

  uint16_t* charcache = new uint16_t[n];

  std::stack<RadixStep, std::vector<RadixStep> > radixstack;
  radixstack.emplace(strings, n, 0, charcache);

  while (TLX_LIKELY(!radixstack.empty()))
    {
      while (TLX_LIKELY(radixstack.top().idx < RADIX))
        {
          RadixStep& rs = radixstack.top();
          ++rs.idx;                                      // process the bucket rs.idx

          if (TLX_UNLIKELY(rs.bkt_size[rs.idx] == 0))
            ;
          else if (TLX_UNLIKELY((rs.idx & 0xFF) == 0)) { // zero-termination
            rs.str += rs.bkt_size[rs.idx];
          }
          else if (TLX_UNLIKELY(rs.bkt_size[rs.idx] < IS_THRESHOLD))
            {
              inssort(rs.str, rs.bkt_size[rs.idx], 2* radixstack.size());
              rs.str += rs.bkt_size[rs.idx];
            }
          else if (rs.bkt_size[rs.idx] < RADIX)
            {
              bingmann_msd_CI2_sb(rs.str, rs.bkt_size[rs.idx],
                                  /* depth */ 2 * radixstack.size(),
                                  reinterpret_cast<uint8_t*>(charcache));
              rs.str += rs.bkt_size[rs.idx];
            }
          else
            {
              rs.str += rs.bkt_size[rs.idx];
              radixstack.emplace(
                                 rs.str - rs.bkt_size[rs.idx], rs.bkt_size[rs.idx],
                                 2 * radixstack.size(), charcache);
              // cannot add here, because rs may have invalidated
            }
        }
      radixstack.pop();
    }

  delete[] charcache;
}
}
/******************************************************************************/
