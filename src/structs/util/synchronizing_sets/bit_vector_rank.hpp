/*******************************************************************************
 * bit_vector_rank.hpp
 *
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

/* Based on
 * @inproceedings{Zhou2013RankSelect,
 *    author    = {Dong Zhou and David G. Andersen and Michael Kaminsky},
 *    title     = {Space-Efficient, High-Performance Rank and Select Structures
 *                 on Uncompressed Bit Sequences},
 *    booktitle = {12th International Symposium on Experimental Algorithms
 *                 ({SEA})},
 *    series    = {LNCS},
 *    volume    = {7933},
 *    pages     = {151--163},
 *    publisher = {Springer},
 *    year      = {2013},
 * }
 */

// Code from the TLX, as this is used in the original implementation



#pragma once

#include "bit_vector.hpp"
#include <vector>
#include <array>
#include <cstdint>
#include <iostream>

#if defined(__GNUC__) || defined(__clang__)
#define TLX_LIKELY(c)   __builtin_expect((c), 1)
#define TLX_UNLIKELY(c) __builtin_expect((c), 0)
#else
#define TLX_LIKELY(c)   c
#define TLX_UNLIKELY(c) c
#endif

#if defined(__GNUC__) || defined(__clang__)
#define TLX_ATTRIBUTE_PACKED __attribute__ ((packed))
#else
#define TLX_ATTRIBUTE_PACKED
#endif

template <size_t Words>
uint64_t popcount(uint64_t const * const buffer) {
  uint64_t popcount = 0;
  for (size_t i = 0; i < Words; ++i) {
    popcount += __builtin_popcountll(buffer[i]);
  }
  return popcount;
}

struct l12_entry {
  l12_entry(uint32_t const _l1, std::array<uint16_t, 3> const _l2)
    : l1(_l1), l2_values(0ULL |((uint32_t(0b1111111111) & _l2[2]) << 20) |
                         ((uint32_t(0b1111111111) & _l2[1]) << 10) |
                         (uint32_t(0b1111111111) & _l2[0])) { }

  inline uint16_t operator[](size_t const idx) const {
    return (l2_values >> (10 * idx)) & uint16_t(0b1111111111);
  }

  uint32_t l1;
  uint32_t l2_values;
} TLX_ATTRIBUTE_PACKED; // struct l12_entry

class bit_vector_rank {

public:
  static constexpr size_t l2_bit_size = 512;
  static constexpr size_t l1_bit_size = 4 * l2_bit_size;
  static constexpr size_t l0_bit_size =
    static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) + 1;

  static constexpr size_t l2_word_size = l2_bit_size / (sizeof(uint64_t) * 8);
  static constexpr size_t l1_word_size = l1_bit_size / (sizeof(uint64_t) * 8);
  static constexpr size_t l0_word_size = l0_bit_size / (sizeof(uint64_t) * 8);

public:
  bit_vector_rank(bit_vector const& bv)
    : data_size_(bv.size_), data_(bv.data_),
      l0_size_((data_size_ / l0_word_size) + 1),
      l0_(static_cast<uint64_t*>(operator new(l0_size_*sizeof(uint64_t)))),
      l12_size_((data_size_ / l1_word_size) + 1),
      l12_(static_cast<l12_entry*>(operator new(l12_size_*sizeof(l12_entry)))) {

    l0_[0] = 0;

    size_t l0_pos = 1;
    size_t l12_pos = 0;
    uint32_t l1_entry = 0UL;

    uint64_t const * data = data_;
    uint64_t const * const data_end = data_ + data_size_;

    // For each full L12-Block
    std::array<uint16_t, 3> l2_entries = { 0, 0, 0 };
    while (data + 32 <= data_end) {
      uint32_t new_l1_entry = l1_entry;
      for (size_t i = 0; i < 3; ++i) {
        l2_entries[i] = popcount<8>(data);
        data += 8;
        new_l1_entry += l2_entries[i];
      }
      l12_[l12_pos++] = l12_entry(l1_entry, l2_entries);
      new_l1_entry += popcount<8>(data);
      data += 8;
      l1_entry = new_l1_entry;

      if (TLX_UNLIKELY(l12_pos % (l0_word_size / l1_word_size) == 0)) {
        l0_[l0_pos] += (l0_[l0_pos - 1] + l1_entry);
        ++l0_pos;
        l1_entry = 0;
      }
    }
    // For the last not full L12-Block
    size_t l2_pos = 0;
    while (data + 8 <= data_end) {
      l2_entries[l2_pos++] = popcount<8>(data);
      data += 8;
    }
    l12_[l12_pos++] = l12_entry(l1_entry, l2_entries);

    if (TLX_UNLIKELY(l12_pos % (l0_word_size / l1_word_size) == 0)) {
      l0_[l0_pos] += (l0_[l0_pos - 1] + l1_entry);
      ++l0_pos;
      l1_entry = 0;
    }
  }

  ~bit_vector_rank() {
    delete[] l0_;
    delete[] l12_;
  }

  size_t rank0(size_t const index) const {
    return (index + 1) - rank1(index);
  }

  size_t rank1(size_t const index) const {
    size_t result = 0;
    size_t remaining_bits = index + 1;

    size_t const l0_pos = remaining_bits / l0_bit_size;
    remaining_bits -= l0_pos * l0_bit_size;
    result += l0_[l0_pos];
    
    size_t const l1_pos = remaining_bits / l1_bit_size;
    remaining_bits -= l1_pos * l1_bit_size;
    l12_entry const l12 = l12_[l1_pos + (l0_pos * (l0_word_size /
                                                   l1_word_size))];
    result += l12.l1;

    size_t const l2_pos = remaining_bits / l2_bit_size;
    remaining_bits -= l2_pos * l2_bit_size;
    for (size_t i = 0; i < l2_pos; ++i) {
      result += l12[i];
    }

    size_t offset = (l1_pos * l1_word_size) + (l2_pos * l2_word_size);
    while (remaining_bits >= 64) {
      result += popcount<1>(data_ + offset++);
      remaining_bits -= 64;
    }
    if (remaining_bits > 0) {
      uint64_t const remaining = data_[offset] << (64 - remaining_bits);
      result += popcount<1>(&remaining);
    }
    return result;
  }

private:
  // Access to the bit vector
  size_t const data_size_;
  uint64_t const * const data_;

  // Members for the structure
  size_t const l0_size_;
  uint64_t * const l0_;

  size_t const l12_size_;
  l12_entry * const l12_;
}; // class bit_vector_rank

/******************************************************************************/
