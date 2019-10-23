/*******************************************************************************
 * bit_vector.hpp
 *
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

class bit_access {

public:
  bit_access(uint64_t * const data, size_t const pos) : data_(data),
                                                              pos_(pos) { }
  operator bool() const {
    uint8_t const offset = uint64_t(pos_) & uint64_t(0b111111);
    return (data_[pos_ >> 6] >> offset) & 1ULL;
  }

  bit_access& operator=(bool const value) {
    uint64_t const mask = 1ULL << (uint64_t(pos_) & uint64_t(0b111111));
    data_[pos_ >> 6] = (data_[pos_ >> 6] & ~mask) | (-value & mask);
    return *this;
  }

private:
  uint64_t * const data_;
  size_t const pos_;
}; // class bit_access

class bit_vector {

  friend class bit_vector_rank;
  friend class bit_vector_select;

public:
  // The bit vector uses at most 512 additional bits, as this allows for faster
  // computation of the rank and select support data structures
  bit_vector(size_t const size) : bit_size_(size),
                                  size_((bit_size_ >> 6) +
                                        ((bit_size_ % 512 > 0) ? 8 : 0)),
                                  data_(new uint64_t[size_]) {
    std::fill(data_, data_ + size_, 0ULL);
  }

  ~bit_vector() {
    delete[] data_;
  }

  bit_access operator[](size_t const idx) {
    return bit_access(data_, idx);
  }

  inline void bitset(size_t idx, bool value) {
    // https://graphics.stanford.edu/~seander/bithacks.html
    // (ConditionalSetOrClearBitsWithoutBranching)
    uint64_t const mask = 1ULL << (uint64_t(idx) & uint64_t(0b111111));
    data_[idx >> 6] = (data_[idx >> 6] & ~mask) | (-value & mask);
  }

  inline bool bitread(size_t idx) const {
    uint8_t const offset = uint64_t(idx) & uint64_t(0b111111);
    return (data_[idx >> 6] >> offset) & 1ULL;
  }

  inline size_t size() const {
    return bit_size_;
  }

private:
  size_t const bit_size_;
  size_t const size_;
  uint64_t * const data_;
}; // class bit_vector

/******************************************************************************/
