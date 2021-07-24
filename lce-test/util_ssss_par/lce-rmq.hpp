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
#include <chrono>
#include <vector>
#include <algorithm> //std::sort
#include <string>
#include <execution>

#include <src/libsais_internal.h>
//#include <src/libsais64.h>

#undef SS //RMQ library defines SS, conflicting with libsais members
#include <src/libsais64.c>
#include <tlx/sort/strings/parallel_sample_sort.hpp>
#include "par_rmq_n.hpp"
#include "string_sort_helper.hpp"
#include <ips4o.hpp>

#ifdef DETAILED_TIME
#include <malloc_count.h>
#endif

namespace lce_test::par {
  
template <typename sss_type>
struct rank_tuple {
  sss_type index;
  sss_type rank;

  rank_tuple() = default;
  rank_tuple(sss_type _index, sss_type _rank) : index(_index), rank(_rank) { }

  friend std::ostream& operator<<(std::ostream& os, rank_tuple const& rt) {
    return os << "[ " << rt.index << ", " << rt.rank << "]";
  }
}; // struct rank_tuple

template <typename sss_type, uint64_t kTau = 1024>
class Lce_rmq_par {

public:
  Lce_rmq_par(uint8_t const * const v_text, size_t const v_text_size,
          std::vector<sss_type> const& sync_set, bool print_times=false) 
    : text(v_text), text_size(v_text_size) {
#ifdef DETAILED_TIME
    size_t mem_before = malloc_count_current();
    malloc_count_reset_peak();
    std::chrono::system_clock::time_point begin = std::chrono::system_clock::now();
#endif
    
    //parallel
    std::vector<size_t> strings_to_sort(sync_set.begin(), sync_set.end());
    MockString text_str(reinterpret_cast<const char* const>(v_text), v_text_size);
    StringShortSuffixSet<3*kTau> sufset{text_str, strings_to_sort.begin(), strings_to_sort.end()};

    lcp.resize(strings_to_sort.size());
    tlx::sort_strings_detail::StringLcpPtr strptr(sufset, lcp.data());
    tlx::sort_strings_detail::parallel_sample_sort(strptr, 0, 0);

#ifdef DETAILED_TIME
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    if (print_times) {
      std::cout << "string_sort_time=" 
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
                << "string_sort_mem=" << (malloc_count_peak() - mem_before) << " ";
    }
#endif

#ifdef DETAILED_TIME
    mem_before = malloc_count_current();
    malloc_count_reset_peak();
    begin = std::chrono::system_clock::now();
#endif

    std::vector<rank_tuple<sss_type>> rank_tuples;
    rank_tuples.resize(strings_to_sort.size()); 
    #pragma omp parallel
    {
      const int t = omp_get_thread_num();
      const int nt = omp_get_num_threads();
      const size_t size_per_thread = strings_to_sort.size() / nt;
      const size_t start_i = t * size_per_thread;
      const size_t end_i = (t == nt-1) ? strings_to_sort.size() : (t+1)*size_per_thread;  

      sss_type cur_rank = start_i + 1;
      rank_tuples[start_i] = {static_cast<sss_type>(strings_to_sort[start_i]), cur_rank};

      for (size_t i = start_i + 1; i < end_i; ++i) {
        uint64_t const max_length = std::min(std::min(text_size - strings_to_sort[i],
                                            text_size - strings_to_sort[i - 1]), 3*kTau);
        if(lcp[i] != max_length) {
          ++cur_rank;
        }
        rank_tuples[i] = {static_cast<sss_type>(strings_to_sort[i]), cur_rank};
      }

      #pragma omp barrier
      //Check whether first string is equal to last string of last block.
      if(start_i != 0) {
        uint64_t const max_length = std::min(std::min(text_size - strings_to_sort[start_i],
                                            text_size - strings_to_sort[start_i - 1]), 3*kTau);
        uint64_t depth = 0;
        while (depth < max_length &&
              text[strings_to_sort[start_i] + depth] == text[strings_to_sort[start_i - 1] + depth]) {
          ++depth;
        }
        //If strings are equal, we need to align rank of the latter
        if(lcp[start_i] == max_length) {
          const size_t rank_to_decrease = rank_tuples[start_i].rank;
          for (size_t i = start_i; i < std::min((t + 1) * size_per_thread, strings_to_sort.size()); ++i) {
            if(rank_tuples[i].rank == rank_to_decrease) {
              rank_tuples[i].rank = rank_tuples[i-1].rank;
            } else {
              break;
            }
          }
        }
      }
    }
    size_t max_rank = rank_tuples.back().rank + 1;

    ips4o::sort(rank_tuples.begin(), rank_tuples.end(), 
              [](rank_tuple<sss_type> const& lhs, rank_tuple<sss_type> const& rhs) {
                return lhs.index < rhs.index;
    });
    
    std::vector<sss_type> new_text(rank_tuples.size() + 1, 0);
    std::vector<sss_type> new_sa(rank_tuples.size() + 1, 0);
    #pragma omp parallel for
    for (size_t i = 0; i < rank_tuples.size(); ++i) {
      new_text[i] = static_cast<int32_t>(rank_tuples[i].rank);
    }
    new_text.back() = 0;
    if constexpr(std::is_same<sss_type, uint32_t>::value) {
      libsais_main_32s_internal(reinterpret_cast<int32_t*>(new_text.data()), reinterpret_cast<int32_t*>(new_sa.data()), new_text.size(), max_rank + 1, 0, omp_get_num_threads());
    } else if constexpr(std::is_same<sss_type, uint64_t>::value) {
      libsais64_main_32s(reinterpret_cast<int32_t*>(new_text.data()), reinterpret_cast<int32_t*>(new_sa.data()), new_text.size(), max_rank + 1, 0, omp_get_num_threads());
    } else {
      std::cerr << "NO SUFFIX ARRAY CONSTRUCTION ALGORITHM FOR DATA TYPE\n";
    }

#ifdef DETAILED_TIME
    end = std::chrono::system_clock::now();
    if (print_times) {
      std::cout << "sa_construct_time=" 
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
                << "sa_construct_mem=" << (malloc_count_peak() - mem_before) << " ";
    }
#endif
    
#ifdef DETAILED_TIME
    mem_before = malloc_count_current();
    malloc_count_reset_peak();
    begin = std::chrono::system_clock::now();
#endif
    
    //lcp = std::vector<sss_type>(new_sa.size() - 1, 0);
    std::vector<sss_type> suffix_pred(lcp.size());
    isa.resize(new_sa.size() - 1);

    #pragma omp parallel for 
    for(size_t i = 1; i < new_sa.size(); ++i) {
      isa[new_sa[i]] = i - 1;
      suffix_pred[new_sa[i]] = new_sa[i - 1];
    }
    uint64_t cur_lce = 0;
    #pragma omp parallel for firstprivate(cur_lce)
    for(size_t i = 0; i < new_sa.size(); ++i) {
      if(i == static_cast<uint64_t>(new_sa[0])) {
        continue;
      }
      //We look at the suffix starting at 'i' and its preceding suffix.
      const uint64_t preceding_suffix = suffix_pred[i];
      cur_lce += lce_in_text(sync_set[i] + cur_lce, sync_set[preceding_suffix] + cur_lce);
      suffix_pred[i] = cur_lce;
      //If the current lcp is too small, we can't deduce that the following positions synchronize
      uint64_t diff = sync_set[i+1] - sync_set[i];
      if (cur_lce < kTau + diff) {
        cur_lce = 0;
      } else {
        cur_lce -= diff;
      }
    }

    //We put the values back into suffix array order.
    #pragma omp parallel for
    for (size_t i = 0; i < lcp.size(); ++i) {
      lcp[i] = suffix_pred[new_sa[i+1]];
    }

#ifdef DETAILED_TIME
    end = std::chrono::system_clock::now();
    if (print_times) {
      std::cout << "lcp_construct_time=" 
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
                << "lcp_construct_mem=" << (malloc_count_peak() - mem_before) << " ";
    }
#endif
    //Build RMQ data structure

#ifdef DETAILED_TIME
    mem_before = malloc_count_current();
    malloc_count_reset_peak();
    begin = std::chrono::system_clock::now();
#endif

    rmq_ds1 = std::make_unique<par_RMQ_n<sss_type>>(lcp);

#ifdef DETAILED_TIME
    end = std::chrono::system_clock::now();
    if (print_times) {
      std::cout << "rmq_construct_time=" 
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
                << "rmq_construct_mem=" << (malloc_count_peak() - mem_before) << " ";
    }
#endif
  }
    

    
  uint64_t lce(uint64_t i, uint64_t j) const {
    if(i == j) {
      return text_size - i;
    }

    auto min = std::min(isa[i], isa[j]) + 1;
    auto max = std::max(isa[i], isa[j]);

    if (max - min > 1024) { //THIS 1024 HAS NOTHING TO DO WITH KTAU; DONT CHANGE IT
      return lcp[rmq_ds1->rmq(min, max)];
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
  size_t text_size;
    
  std::vector<sss_type> isa;
  std::vector<sss_type> lcp;
  std::unique_ptr<par_RMQ_n<sss_type>> rmq_ds1;

  uint64_t lce_in_text(uint64_t i, uint64_t j) {
    const uint64_t maxLce = text_size - (i > j ? i : j); 
    uint64_t lce_naive = 0;
    while (lce_naive < maxLce) {
      if (text[i+lce_naive] != text[j+lce_naive]) {
        return lce_naive;
      }
      ++lce_naive;
    }
    return lce_naive;
  }
};
}

/******************************************************************************/
