/*******************************************************************************
 * structs/lce_semi_synchronizing_sets.hpp
 *
 * Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include "util/lce_interface.hpp"
#include "util/util.hpp"

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)    __builtin_expect(!!(x), 0) 

/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */

class LceUltraNaive : public LceDataStructure {
public:
  LceUltraNaive() = delete;
  /* Loads the full file located at PATH. */
  LceUltraNaive(std::vector<uint8_t> const& text)
    : text_(text.data()), text_length_in_bytes_(text.size()) { }
			
  ~LceUltraNaive() {
    delete[] text_;
  }
		
  /* Naive LCE-query */
  uint64_t lce(const uint64_t i, const uint64_t j) {			
    if (unlikely(i == j)) {
      return text_length_in_bytes_ - i;
    }
			
    const uint64_t max_length = text_length_in_bytes_ - ((i < j) ? j : i);
    uint64_t lce = 0;
    while(likely(lce < max_length) && text_[i + lce] == text_[j + lce]) {
      lce++;
    }
    return lce;
  }
		
  inline char operator[](const uint64_t i) {
    return text_[i];
  }
		
  int isSmallerSuffix(const uint64_t i, const uint64_t j) {
    uint64_t lce_s = lce(i, j);
    if(unlikely((i + lce_s + 1 == text_length_in_bytes_) ||
                (j + lce_s + 1 == text_length_in_bytes_))) {
      return true;
    }
    return (operator[](i + lce_s) < operator[](j + lce_s));
  }
		
  uint64_t getSizeInBytes() {
    return text_length_in_bytes_;
  }
		
private:
  uint8_t const * const text_;
  const uint64_t text_length_in_bytes_;
};

/******************************************************************************/
