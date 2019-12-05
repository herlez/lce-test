/*******************************************************************************
 * util/lce_interface.hpp
 *
 * Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <inttypes.h>
#include <string>
#include <fstream>

class LceDataStructure {
public:
  virtual ~LceDataStructure() = 0;
  virtual uint64_t lce(const uint64_t i, const uint64_t j) = 0;
  //virtual char getChar(const uint64_t i) = 0;
  virtual char operator[](const uint64_t i) = 0;
  virtual int isSmallerSuffix(const uint64_t i, const uint64_t j) = 0;
  virtual uint64_t getSizeInBytes() = 0;
}; // class LceDataStructure

LceDataStructure::~LceDataStructure() { }

/******************************************************************************/
