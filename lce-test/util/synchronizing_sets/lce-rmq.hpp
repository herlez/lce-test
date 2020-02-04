/*******************************************************************************
 * structs/lce_semi_synchronizing_sets.hpp
 *
 * Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de> 
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <tlx/define/likely.hpp>

#include "./rmq.hpp"
#include "indexed_string.hpp"
#include "string_sorting.hpp"
#include <vector>
#include <algorithm> //std::sort
#include <string>
#include "RMQRMM64.h"

#include "sais.h"

struct rank_tuple {
  uint64_t index;
  uint64_t rank;

  rank_tuple(uint64_t _index, uint64_t _rank) : index(_index), rank(_rank) { }

  friend std::ostream& operator<<(std::ostream& os, rank_tuple const& rt) {
    return os << "[ " << rt.index << ", " << rt.rank << "]";
  }
}; // struct rank_tuple

template <typename sss_type, uint64_t kTau = 1024>
class Lce_rmq {

public:
  Lce_rmq(uint8_t const * const v_text, uint64_t const v_text_size,
          std::vector<sss_type> const& sync_set,
          std::vector<uint64_t> const& s_fingerprints) 
    : text(v_text), text_size(v_text_size) {

    std::vector<indexed_string> strings_to_sort;
    for (uint64_t i = 0; i < sync_set.size(); ++i) {
      strings_to_sort.emplace_back(sync_set[i], text, text_size, kTau * 3);
    }

    radixsort(strings_to_sort.data(), strings_to_sort.size());

    std::vector<rank_tuple> rank_tuples;
    rank_tuples.reserve(strings_to_sort.size());
    uint64_t cur_rank = 1;
    rank_tuples.emplace_back(strings_to_sort[0].index(), cur_rank);
    for (uint64_t i = 1; i < strings_to_sort.size(); ++i) {
      uint64_t const max_length = std::min(strings_to_sort[i].max_length(),
                                           strings_to_sort[i - 1].max_length());
      uint64_t depth = 0;
      while (depth < max_length &&
             strings_to_sort[i][depth] == strings_to_sort[i - 1][depth]) {
        ++depth;
      }
      if (strings_to_sort[i][depth] != strings_to_sort[i - 1][depth]) {
        ++cur_rank;
      }
      rank_tuples.emplace_back(strings_to_sort[i].index(), cur_rank);
    }
    std::sort(rank_tuples.begin(), rank_tuples.end(),
              [](rank_tuple const& lhs, rank_tuple const& rhs) {
                return lhs.index < rhs.index;
    });

    std::vector<int32_t> new_text;
    std::vector<int32_t> new_sa(rank_tuples.size() + 1, 0);
    new_text.reserve(rank_tuples.size());
    for (size_t i = 0; i < rank_tuples.size(); ++i) {
      new_text.push_back(static_cast<int32_t>(rank_tuples[i].rank));
    }
    new_text.push_back(0);
    sais_int(new_text.data(), new_sa.data(), new_text.size(), 2 * cur_rank + 1);

    lcp = std::vector<uint64_t>(new_sa.size() - 1, 0);

    isa.resize(new_sa.size() - 1);
    for(uint64_t i = 1; i < new_sa.size() - 1; ++i) {
      isa[new_sa[i]] = i - 1;
      lcp[i] = lce_in_text(sync_set[new_sa[i]],
                           sync_set[new_sa[i + 1]]);
    }

    //Build RMQ data structure
    rmq_ds1 = std::make_unique<RMQRMM64>((long int*)lcp.data(), lcp.size());
  }
    

    
  uint64_t lce(uint64_t i, uint64_t j) const {
    if(i == j) {
      return text_size - i;
    }

    auto min = std::min(isa[i], isa[j]) + 1;
    auto max = std::max(isa[i], isa[j]);

    if (max - min > 1024) {
      return lcp[rmq_ds1->queryRMQ(min, max)];
    }

    auto result = lcp[min];
    for (auto i = min + 1; i <= max; ++i) {
      result = std::min(result, lcp[i]);
    }
    return result;
  }
    
  uint64_t get_size() {
    return text_size;
  }

private:
  uint8_t const * const text;
  uint64_t text_size;
    
  std::vector<uint64_t> isa;
  std::vector<uint64_t> lcp;
  //Rmq * rmq_ds;
  std::unique_ptr<RMQRMM64> rmq_ds1;

  inline void radixsort(indexed_string* strings, size_t n) {
    bingmann_msd_CI3_sb(strings, n);
  }


  uint64_t lce_in_text(uint64_t i, uint64_t j) {
    const uint64_t maxLce = text_size - (i > j ? i : j); 
    uint64_t lce_naive = 0;
    while (lce_naive < maxLce) {
      if (text[i+lce_naive] != text[j+lce_naive]) {
        return lce_naive;
      }
      ++lce_naive;
    }
    std::cout << std::endl;
    return lce_naive;
  }
};

/******************************************************************************/
