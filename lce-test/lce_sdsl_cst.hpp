/*******************************************************************************
 * lce-text/lce_naive_ultra.hpp
 *
 * Copyright (C) 2020 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <vector>

#include <sdsl/cst_sada.hpp>
#include <sdsl/cst_sct3.hpp>

#include "util/lce_interface.hpp"

template <typename t_index>
class LceSDSL : public LceDataStructure {

  t_index cst;

public:
  LceSDSL(std::string const& file) {
    sdsl::construct(cst, file, 1);
  }

  ~LceSDSL() { }

  uint64_t lce(uint64_t const i, uint64_t const j) {
    uint64_t const ip = cst.csa.isa[i];
    uint64_t const jp = cst.csa.isa[j];

    return cst.node_depth(cst.node(std::min(ip, jp), std::max(ip, jp)));
  };

  char operator[](const uint64_t i) { return 0; }

  int32_t isSmallerSuffix(uint64_t const i, uint64_t const j) {
    uint64_t const ip = cst.csa.isa[i];
    uint64_t const jp = cst.csa.isa[j];

    return ip < jp;
  }

  uint64_t getSizeInBytes() { return 0; }

}; // class LceSDSL

using LceSDSLsada = LceSDSL<sdsl::cst_sada<>>;
using LceSDSLsct3 = LceSDSL<sdsl::cst_sct3<>>;

/******************************************************************************/
