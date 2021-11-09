/*******************************************************************************
 * lce-text/lce_naive_ultra.hpp
 *
 * Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <vector>

#include <tlx/define/likely.hpp>

#include "util/lce_interface.hpp"

/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */

class LceNaive : public LceDataStructure {
public:
  __extension__ typedef unsigned __int128 uint128_t;

  LceNaive(std::vector<uint8_t> const& text)
    : text_(text), text_length_in_bytes_(text.size()) { }

  /* Naive LCE-query */
  uint64_t lce(const uint64_t i, const uint64_t j) {

    if (TLX_UNLIKELY(i == j)) {
      return text_length_in_bytes_ - i;
    }

    const uint64_t max_length = text_length_in_bytes_ - ((i < j) ? j : i);
    uint64_t lce = 0;
    // First we compare the first few characters. We do this, because in the
    // usual case the lce is low.
    for(; lce < 8; ++lce) {
      if(TLX_UNLIKELY(lce >= max_length)) {
        return max_length;
      }
      if(text_[i + lce] != text_[j + lce]) {
        return lce;
      }
    }

    // Accelerate search by comparing 16-byte blocks
    lce = 0;
    uint128_t const* const text_blocks_i =
      reinterpret_cast<uint128_t const*>(text_.data() + i);
    uint128_t const * const text_blocks_j =
      reinterpret_cast<uint128_t const *>(text_.data() + j);
    for(; lce < max_length/16; ++lce) {
      if(text_blocks_i[lce] != text_blocks_j[lce]) {
        break;
      }
    }
    lce *= 16;
    // The last block did not match. Here we compare its single characters
    uint64_t lce_end = std::min(lce + 16, lce + max_length);
    for (; lce < lce_end; ++lce) {
      if(text_[i + lce] != text_[j + lce]) {
        break;
      }
    }
    return lce;
  }

  inline char operator[](const uint64_t i) {
    return text_[i];
  }

  int isSmallerSuffix(const uint64_t i, const uint64_t j) {
    uint64_t lce_s = lce(i, j);
    if(TLX_UNLIKELY((i + lce_s + 1 == text_length_in_bytes_) ||
                (j + lce_s + 1 == text_length_in_bytes_))) {
      return true;
    }
    return (text_[i + lce_s] < text_[j + lce_s]);
  }

  uint64_t getSizeInBytes() {
    return text_length_in_bytes_;
  }

private: 
  std::vector<uint8_t> const& text_;
  const uint64_t text_length_in_bytes_;
};

/******************************************************************************/
