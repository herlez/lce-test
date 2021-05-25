/*******************************************************************************
 * util/indexed_string.hpp
 *
 * Copyright (C) 2020 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

class indexed_string {
public:
  indexed_string() = default;

  indexed_string(uint64_t const index, uint8_t const * const string,
                 uint64_t const string_length, uint64_t const kTau)
    : string_(string + index),
      max_length_(std::min<uint64_t>(kTau, string_length - index)),
      index_(index) { }

  uint8_t operator[](size_t const index) {
    if(index >= max_length_) {
      return 0;
    }
    return string_[index];
  }

  uint8_t const* string() const {
    return string_;
  }

  uint64_t index() const {
    return index_;
  }

  uint64_t max_length() const {
    return max_length_;
  }

  friend std::ostream& operator<<(std::ostream& os, indexed_string const& idx_str) {
    os << "[i=" << idx_str.index_ << ", n=" << idx_str.max_length_ << "| ";
    // for (uint64_t i = 0; i < idx_str.max_length_; ++i) {
    //   os << static_cast<uint64_t>(idx_str.string_[i]);
    // }
    os << "]";
    return os;
  }

private:
  uint8_t const * string_;
  uint64_t max_length_;
  uint64_t index_;
}; // class indexed_string

/******************************************************************************/
