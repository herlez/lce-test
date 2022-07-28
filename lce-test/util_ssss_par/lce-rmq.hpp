/*******************************************************************************
 * structs/lce_semi_synchronizing_sets.hpp
 *
 * Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <algorithm>  //std::sort
#include <chrono>
#include <string>
#include <vector>
#include <src/libsais64.h>
#include <src/libsais.h>

#include <ips4o.hpp>
#include <tlx/sort/strings/parallel_sample_sort.hpp>

#include "par_rmq_n.hpp"
#include "string_sort_helper.hpp"

#ifdef DETAILED_TIME
#include <malloc_count.h>
#endif

namespace lce_test::par {

template <typename sss_type>
struct rank_tuple {
  sss_type index;
  uint32_t rank; //We assume the string synchronizing set hold less than 2^31 values. For tau=512 this is around 500GB text.

  rank_tuple() = default;
  rank_tuple(sss_type _index, uint32_t _rank) : index(_index), rank(_rank) {}

  friend std::ostream& operator<<(std::ostream& os, rank_tuple const& rt) {
    return os << "[ " << rt.index << ", " << rt.rank << "]";
  }
};  // struct rank_tuple

template <typename sss_type, uint64_t kTau = 1024>
class Lce_rmq_par {
 public:
  Lce_rmq_par(uint8_t const* const v_text, size_t const v_text_size,
              string_synchronizing_set_par<kTau, sss_type> const& sync_set)
      : text(v_text), text_size(v_text_size) {
#ifdef DETAILED_TIME
    size_t mem_before = malloc_count_current();
    malloc_count_reset_peak();
    std::chrono::system_clock::time_point begin = std::chrono::system_clock::now();
#endif

    // Sort 3*tau long strings starting at string synchronizing set positions in parallel
    std::vector<sss_type> strings_to_sort(sync_set.get_sss().begin(), sync_set.get_sss().end());
    mock_string text_str(v_text, v_text_size);
    StringShortSuffixSet<3 * kTau, sss_type> sufset{text_str, strings_to_sort.begin(), strings_to_sort.end(), sync_set};

    tlx::sort_strings_detail::StringPtr strptr(sufset);
    tlx::sort_strings_detail::parallel_sample_sort(strptr, 0, 0);
    
    // Check sorted strings
    /*{
      for (size_t idx = 1; idx < strings_to_sort.size(); ++idx) {
        size_t i = strings_to_sort[idx - 1];
        size_t j = strings_to_sort[idx];
        assert(leq_three_tau(i, j, sync_set));
      }
    }*/

#ifdef DETAILED_TIME
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    std::cout << "string_sort_time="
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
              << "string_sort_mem=" << (malloc_count_peak() - mem_before) << " ";
#endif

#ifdef DETAILED_TIME
    mem_before = malloc_count_current();
    malloc_count_reset_peak();
    begin = std::chrono::system_clock::now();
#endif

    // Reduce alphabet by giving sorted strings their rank.
    std::vector<rank_tuple<sss_type>> rank_tuples(strings_to_sort.size()); // Store <sss_index, rank> tuples.
    // We may need to adjust ranks at thread borders.
    int nt = omp_get_max_threads();
    std::vector<sss_type> max_ranks(nt);  // Max rank in block
    std::vector<char> all_ranks_equal(nt); // Are all ranks in the block equal?
    std::vector<char> rank_extends_prev_block(nt); // Is first rank in block == last rank in prev block?

#pragma omp parallel
    {
      // First compare strings in block and adjust ranks
      const int t = omp_get_thread_num();
      const int nt = omp_get_num_threads();
      const size_t size_per_thread = rank_tuples.size() / nt;
      const size_t start_i = t * size_per_thread;
      const size_t end_i = (t == nt - 1) ? rank_tuples.size() : (t + 1) * size_per_thread;

      uint32_t cur_rank = start_i + 1;
      rank_tuples[start_i] = {static_cast<sss_type>(strings_to_sort[start_i]), cur_rank};

      for (size_t i = start_i + 1; i < end_i; ++i) {
        assert(leq_three_tau(strings_to_sort[i-1], strings_to_sort[i], sync_set));
        if(!eq_three_tau(strings_to_sort[i-1], strings_to_sort[i], sync_set)) {
          ++cur_rank;
        }
        rank_tuples[i] = {strings_to_sort[i], cur_rank};
      }
      max_ranks[t] = cur_rank;

#pragma omp barrier
      assert (rank_tuples[start_i].rank == start_i+1);
      all_ranks_equal[t] = (max_ranks[t] == start_i+1);
      rank_extends_prev_block[t] = (t == 0) ? false : eq_three_tau(strings_to_sort[start_i-1], strings_to_sort[start_i], sync_set);
#pragma omp barrier
      // Now adjust ranks between blocks 
      if(t != 0) {
        if(rank_extends_prev_block[t]) {
          size_t target_t = t-1;
          while(all_ranks_equal[target_t] && rank_extends_prev_block[target_t]) {
            --target_t;
          }
          sss_type target_rank = max_ranks[target_t];
          
          const uint32_t rank_to_decrease = rank_tuples[start_i].rank;
          for (size_t i = start_i; i < end_i && rank_tuples[i].rank == rank_to_decrease; ++i) { 
            rank_tuples[i].rank = target_rank;
          }
        } 
      }
    }
    uint32_t max_rank = rank_tuples.back().rank + 1;
    // Check rank_tuples
    /*{
      assert(rank_tuples.size() == strings_to_sort.size());
      for(size_t i = 0; i < rank_tuples.size(); ++i) {
        assert(rank_tuples[i].index == strings_to_sort[i]);
      }

      for(size_t i = 1; i < rank_tuples.size(); ++i) {
        bool neq_neighbors = (rank_tuples[i-1].rank) < (rank_tuples[i].rank);
        assert(neq_neighbors == !eq_three_tau(rank_tuples[i-1].index, rank_tuples[i].index, sync_set));
      }
    }*/

    ips4o::sort(rank_tuples.begin(), rank_tuples.end(),
                [](rank_tuple<sss_type> const& lhs, rank_tuple<sss_type> const& rhs) {
                  return lhs.index < rhs.index;
                });

    std::vector<uint32_t> new_text(rank_tuples.size() + 1, 0);
    std::vector<uint32_t> new_sa(new_text.size(), 0);
#pragma omp parallel for
    for (size_t i = 0; i < rank_tuples.size(); ++i) {
      new_text[i] = rank_tuples[i].rank;
    }
    new_text.back() = 0;
    libsais_int_omp(reinterpret_cast<int32_t*>(new_text.data()), reinterpret_cast<int32_t*>(new_sa.data()), new_text.size(), max_rank + 1, 0, omp_get_max_threads());

#ifdef DETAILED_TIME
    end = std::chrono::system_clock::now();
    std::cout << "sa_construct_time="
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
              << "sa_construct_mem=" << (malloc_count_peak() - mem_before) << " ";
#endif

#ifdef DETAILED_TIME
    mem_before = malloc_count_current();
    malloc_count_reset_peak();
    begin = std::chrono::system_clock::now();
#endif
    isa.resize(new_sa.size());
#pragma omp parallel for
    for (uint32_t i = 0; i < new_sa.size(); ++i) {
      isa[new_sa[i]] = i;
    }

    lcp.resize(new_sa.size());
    lcp[0] = 0;
    lcp[1] = 0;
    size_t current_lcp = 0;
#pragma omp parallel for firstprivate (current_lcp)
    for (size_t i = 0; i < lcp.size()-1; ++i) {
      size_t suffix_array_pos = isa[i];
      assert(suffix_array_pos != 0); //We stop loop before before isa[lce.size()-1]==0
      if (suffix_array_pos == 1) {continue;} //We can not do lce_query with sentinel new_sa.back()
      size_t preceding_suffix_pos = new_sa[suffix_array_pos - 1];
      current_lcp += lce_in_text(sync_set[i] + current_lcp, sync_set[preceding_suffix_pos] + current_lcp);
      lcp[suffix_array_pos] = current_lcp;

      uint64_t diff = sync_set[i + 1] - sync_set[i];
      if (current_lcp < 2 * kTau + diff) {
        current_lcp = 0;
      } else {
        current_lcp -= diff;
      }
    }

    //Check SA and LCP array
    /*{
      for(volatile size_t i = 2; i < new_sa.size(); ++i) {
        volatile size_t text_index_left = sync_set[new_sa[i-1]];
        volatile size_t text_index_right = sync_set[new_sa[i]];
        
        uint64_t const max_length = std::min(text_size - text_index_left, text_size - text_index_right);
        volatile size_t lce = lce_in_text(text_index_left, text_index_right);
        assert((lce < max_length && text[text_index_left + lce] < text[text_index_right + lce]) 
             || (lce == max_length && text_index_left > text_index_right));
        assert(lce == lcp[i]);
      }
    }*/
    

#ifdef DETAILED_TIME
    end = std::chrono::system_clock::now();
    std::cout << "lcp_construct_time="
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
              << "lcp_construct_mem=" << (malloc_count_peak() - mem_before) << " ";
#endif

#ifdef DETAILED_TIME
    mem_before = malloc_count_current();
    malloc_count_reset_peak();
    begin = std::chrono::system_clock::now();
#endif
    // Build RMQ data structure
    rmq_ds1 = std::make_unique<par_RMQ_n<sss_type>>(lcp);

#ifdef DETAILED_TIME
    end = std::chrono::system_clock::now();
    std::cout << "rmq_construct_time="
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
              << "rmq_construct_mem=" << (malloc_count_peak() - mem_before) << " ";
#endif
  }

  uint64_t lce(uint64_t i, uint64_t j) const {
    if (i == j) {
      return text_size - i;
    }

    auto min = std::min(isa[i], isa[j]) + 1;
    auto max = std::max(isa[i], isa[j]);
    if (max - min > 1024) {  // THIS 1024 HAS NOTHING TO DO WITH KTAU; DONT CHANGE IT
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
  uint8_t const* const text;
  size_t text_size;

  std::vector<uint32_t> isa;
  std::vector<sss_type> lcp;
  std::unique_ptr<par_RMQ_n<sss_type>> rmq_ds1;

  uint64_t lce_in_text(uint64_t i, uint64_t j, uint64_t up_to = std::numeric_limits<uint64_t>::max()) {
    uint64_t const max_length = std::min({text_size - i, text_size - j, up_to});
    uint64_t lce_naive = 0;
    while (lce_naive < max_length) {
      if (text[i + lce_naive] != text[j + lce_naive]) {
        return lce_naive;
      }
      ++lce_naive;
    }
    return lce_naive;
  }

  uint64_t lce_in_text_exact(uint64_t text_pos_i, uint64_t text_pos_j, uint64_t exact_up_to) {
    uint64_t lce = 0;
    while(lce < exact_up_to && text[text_pos_i + lce] == text[text_pos_j + lce]){
      ++lce;
    }
    return lce;
  }

  bool leq_three_tau(size_t text_pos_i, size_t text_pos_j, string_synchronizing_set_par<kTau, sss_type> const& sync_set) {
    size_t const max_length = std::min({text_size - text_pos_i, text_size - text_pos_j, 3 * kTau});
    size_t text_lce = lce_in_text_exact(text_pos_i, text_pos_j, max_length);
    return (text_lce < max_length && text[text_pos_i + text_lce] < text[text_pos_j + text_lce]) 
        || (text_lce == max_length && sync_set.get_run_info(text_pos_i) <= sync_set.get_run_info(text_pos_j));
  }

  
  bool eq_three_tau(size_t text_pos_i, size_t text_pos_j, string_synchronizing_set_par<kTau, sss_type> const& sync_set) {
    size_t const max_length = std::min({text_size - text_pos_i, text_size - text_pos_j, 3 * kTau});
    size_t text_lce = lce_in_text_exact(text_pos_i, text_pos_j, max_length);
    return ((text_lce == max_length) && sync_set.get_run_info(text_pos_i) == sync_set.get_run_info(text_pos_j));
  }
};
}  // namespace lce_test::par

/******************************************************************************/
