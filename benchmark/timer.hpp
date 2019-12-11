/*******************************************************************************
 * include/timer.hpp<include>
 *
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <chrono>

class timer {

public:
  timer() : begin_(std::chrono::system_clock::now()) { }

  void reset() {
    begin_ = std::chrono::system_clock::now();
  }

  size_t get() const {
    auto const end = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - begin_).count();
  }

  size_t get_and_reset() {
    auto const time = get();
    reset();
    return time;
  }

private:
  std::chrono::system_clock::time_point begin_;
}; // class timer

/******************************************************************************/
