/*******************************************************************************
 * popcount.hpp
 *
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <iostream>

template <size_t Words>
uint64_t popcount(uint64_t const * const buffer) {
  uint64_t popcount = 0;
  for (size_t i = 0; i < Words; ++i) {
    popcount += __builtin_popcountll(buffer[i]);
  }
  return popcount;
}

/******************************************************************************/
