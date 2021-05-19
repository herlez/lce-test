/*******************************************************************************
 * /ring_buffer.hpp
 *
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <tlx/math/round_to_power_of_two.hpp>

template <typename DataType>
class ring_buffer {
public:
  ring_buffer(uint64_t const buffer_size)
    : buffer_size_(tlx::round_up_to_power_of_two(buffer_size)),
      mod_mask_(buffer_size_ - 1), size_(0), data_(buffer_size_) { }

  void push_back(DataType const data) {
    uint64_t const insert_pos = size_++ & mod_mask_;
    data_[insert_pos] = data;
  }

  size_t size() const {
    return size_;
  }

  void resize(size_t s) {
    size_ = s;
  }

  DataType operator[](size_t const index) {
    return data_[index & mod_mask_];
  }

  DataType operator[](size_t const index) const {
    return data_[index & mod_mask_];
  }

private:
  uint64_t const buffer_size_;
  uint64_t const mod_mask_;

  uint64_t size_;

  std::vector<DataType> data_;

}; // class ring_buffer

/******************************************************************************/
