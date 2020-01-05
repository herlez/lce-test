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
#include <vector>
#include <algorithm> //std::sort
#include <string>
#include "RMQRMM64.h"

#include "sais.h"

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

struct rank_tuple {
  uint64_t index;
  uint64_t rank;

  rank_tuple(uint64_t _index, uint64_t _rank) : index(_index), rank(_rank) { }

  friend std::ostream& operator<<(std::ostream& os, rank_tuple const& rt) {
    os << "[ " << rt.index << ", " << rt.rank << "]";
  }
}; // struct rank_tuple

class Lce_rmq {

public:
  Lce_rmq(uint8_t const * const v_text, uint64_t const v_text_size,
          std::vector<uint64_t> const& sync_set,
          std::vector<uint64_t> const& s_fingerprints) 
    : text(v_text), text_size(v_text_size) {

    std::vector<indexed_string> strings_to_sort;
    for (uint64_t i = 0; i < sync_set.size(); ++i) {
      strings_to_sort.emplace_back(sync_set[i], text, text_size, tau * 3);
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
      if (strings_to_sort[i][depth] > strings_to_sort[i - 1][depth]) {
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
    sais_int(new_text.data(), new_sa.data(), new_text.size(), cur_rank + 1);

    // std::vector<rank_tuple> rank_tuples;
    // rank_tuples.reserve(s_fingerprints.size());
    // for (size_t i = 0; i < s_fingerprints.size(); ++i) {
    //   rank_tuples.emplace_back(i, s_fingerprints[i]);
    //   std::cout << "rank_tuples.back() " << rank_tuples.back() << std::endl;
    // }

    // std::sort(rank_tuples.begin(), rank_tuples.end(),
    //           [](rank_tuple const& a, rank_tuple const& b) {
    //             return a.rank < b.rank;
    // });

    // uint64_t old_rank = 0;
    // uint32_t cur_rank = 0;
    // for (size_t i = 0; i < rank_tuples.size(); ++i) {
    //   if (old_rank != rank_tuples[i].rank) {
    //     old_rank = rank_tuples[i].rank;
    //     ++cur_rank;
    //   }
    //   rank_tuples[i].rank = cur_rank;
    //   std::cout << "rank_tuples[i] " << rank_tuples[i] << std::endl;
    // }

    // std::sort(rank_tuples.begin(), rank_tuples.end(),
    //           [](rank_tuple const& b, rank_tuple const& a) {
    //             return a.index < b.index;
    // });

    // std::vector<int32_t> new_text;
    // std::vector<int32_t> new_sa(rank_tuples.size() + 1, 0);
    // new_text.reserve(rank_tuples.size());
    // for (size_t i = 0; i < rank_tuples.size(); ++i) {
    //   new_text.push_back(static_cast<int32_t>(rank_tuples[i].rank));
    // }
    // new_text.push_back(0);
    // sais_int(new_text.data(), new_sa.data(), new_text.size(), 2*cur_rank + 5);


    // std::cout << "NAIVELY SA" << std::endl;

    // std::vector<uint32_t> sa(rank_tuples.size(), 0);
    // for (size_t i = 0; i < sa.size(); ++i) {
    //   sa[i] = i;
    // }
    // std::sort(sa.begin(), sa.end(), [=](uint64_t i, uint64_t j) {
    //   const uint64_t start_i = sync_set[i];
    //   const uint64_t start_j = sync_set[j];
    //   uint64_t max_lce = text_size - (start_i > start_j ? start_i : start_j);
						
    //   for(uint64_t k = 0; k < max_lce; ++k) {
    //     if (text[start_i + k] != text[start_j + k]) {
    //       return (text[start_i + k] < text[start_j + k]);
    //     }
    //   }
    //   return i > j;
    // });

    // std::cout << "FINISH NAIVE SA" << std::endl;

    // // for (size_t i = 0; i < sa.size(); ++i) {
    // //   std::cout << "sa[i] " << sa[i] << std::endl;
    // // }

    // // for (size_t i = 0; i < new_sa.size(); ++i) {
    // //   std::cout << "new_sa[i] " << new_sa[i] << std::endl;
    // // }

    // for (size_t i = 0; i < sa.size(); ++i) {
    //   if (sa[i] != new_sa[i + 1]) {
    //     std::cout << "NOPE" << std::endl;
    //   }
    // }

    lcp = std::vector<uint64_t>(new_sa.size(), 0);

    isa.resize(new_sa.size() - 1);
    for(uint64_t i = 0; i < new_sa.size() - 1; ++i) {
      isa[new_sa[i + 1]] = i;
      lcp[i + 1] = lce_in_text(sync_set[new_sa[i + 1]], sync_set[new_sa[i + 2]]);
    }

    // std::vector<uint64_t> old_lcp(sa.size());
    // for(uint64_t i = 1; i < sa.size(); ++i) {
    //   old_lcp[i] = lce_in_text(sync_set[sa[i-1]], sync_set[sa[i]]);
    // }

    // for (size_t i = 0; i < lcp.size(); ++i) {
    //   if (lcp[i] != old_lcp[i]) {
    //     std::cout << "lcp.size() " << lcp.size() << std::endl;
    //     std::cout << "old_lcp.size() " << old_lcp.size() << std::endl;
    //     std::cout << "i " << i << std::endl;
    //     std::cout << "lcp[i] " << lcp[i] << std::endl;
    //     std::cout << "old_lcp[i] " << old_lcp[i] << std::endl;
    //     std::cout << "NOPE LCP" << std::endl;
    //   }
    // }

    //Build RMQ data structure
    rmq_ds1 = new RMQRMM64((long int*)lcp.data(), lcp.size());
  }
	

	
  uint64_t lce(uint64_t i, uint64_t j) const {
    if(i == j) {
      return text_size - i;
    }
    if(isa[i] < isa[j]) {
      return lcp[rmq_ds1->queryRMQ(isa[i]+1, isa[j])];
    } else {
      return lcp[rmq_ds1->queryRMQ(isa[j]+1, isa[i])];
    }
  }
	
  uint64_t get_size() {
    return text_size;
  }

private:
  const uint64_t tau = 1024;
  uint8_t const * const text;
  uint64_t text_size;
	
  std::vector<uint64_t> isa;
  std::vector<uint64_t> lcp;
  //Rmq * rmq_ds;
  RMQRMM64 * rmq_ds1;

  void inline inssort(indexed_string* strings, size_t n, int64_t depth = 0) {
    if (n <= 1) {
      return;
    }

    size_t begin = 0;
    size_t j = 0;
    for (size_t i = begin + 1; --n != 0; ++i) {
      indexed_string tmp = strings[i];
      j = i;

      while (j != begin) {
        auto* s = strings[j - 1].string() + depth;
        auto* t = tmp.string() + depth;
        while (*s != 0 && *s == *t) {
          ++s, ++t;
        }
        if (*s <= *t) {
          break;
        }
        strings[j] = strings[j - 1];
        --j;
      }
      strings[j] = tmp;
    }
  }

  template <size_t IS_THRESHOLD = 32>
  void inline msd_CE0(indexed_string* strings, indexed_string* sorted, size_t n,
                      uint64_t depth) {
    if (n <= 1) {
      return;
    }

    if (n <= IS_THRESHOLD || depth == 1024) {
      inssort(strings, n, depth);
      return;
    }

    constexpr size_t max_char = std::numeric_limits<uint8_t>::max() + 1;
    std::array<size_t, max_char> bucket_sizes = { 0 };
    for (size_t i = 0; i < n; ++i) {
      ++bucket_sizes[strings[i][depth]];
    }

    {
      std::array<indexed_string*, max_char> buckets;
      buckets[0] = sorted;
      for (size_t i = 1; i < max_char; ++i) {
        buckets[i] = buckets[i - 1] + bucket_sizes[i - 1];
      }
      for (auto* cur_string = strings; cur_string < strings + n; ++cur_string) {
        *(buckets[(*cur_string)[depth]]++) = *cur_string;
      }
    }
    std::copy_n(sorted, n, strings);
    auto* bucket_border = strings + bucket_sizes[0];
    for (size_t i = 1; i < max_char; ++i) {
      if (bucket_sizes[i] > 0) {
        msd_CE0(bucket_border, sorted, bucket_sizes[i], depth + 1);
        bucket_border += bucket_sizes[i];
      }
    }

  }

  inline void radixsort(indexed_string* strings, size_t n) {
    std::vector<indexed_string> sorted_buffer(n);
    msd_CE0(strings, sorted_buffer.data(), n, 0);
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
