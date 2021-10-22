/*******************************************************************************
 * structs/lce_semi_synchronizing_sets.hpp
 *
 * Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <src/libsais_internal.h>

#include <algorithm>  //std::sort
#include <chrono>
#include <execution>
#include <string>
#include <tlx/define/likely.hpp>
#include <vector>
//#include <src/libsais64.h>

#undef SS  // RMQ library defines SS, conflicting with libsais members
#include <ips4o.hpp>
#include <src/libsais64.c>
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
  sss_type rank;

  rank_tuple() = default;
  rank_tuple(sss_type _index, sss_type _rank) : index(_index), rank(_rank) {}

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

    // parallel
    std::vector<sss_type> strings_to_sort(sync_set.get_sss().begin(), sync_set.get_sss().end());
    mock_string text_str(v_text, v_text_size);
    StringShortSuffixSet<3 * kTau> sufset{text_str, strings_to_sort.begin(), strings_to_sort.end(), sync_set};

    tlx::sort_strings_detail::StringPtr strptr(sufset);
    tlx::sort_strings_detail::parallel_sample_sort(strptr, 0, 0);
    // Check strings to sort
    /*
    {
      size_t e1 = 0;
      size_t e2 = 0;
      for (size_t idx = 1; idx < strings_to_sort.size(); ++idx) {
        size_t i = strings_to_sort[idx - 1];
        size_t j = strings_to_sort[idx];
        size_t text_lce = lce_in_text(i, j, 3 * kTau);
        if (text_lce < 3 * kTau) {
          if (text[i + text_lce] > text[j + text_lce]) {
            ++e1;
          }
        } else {
          if (sync_set.get_run_info(i) > sync_set.get_run_info(j)) {
            printf("i=%lu(%li) j=%lu(%li)", i, sync_set.get_run_info(i), j, sync_set.get_run_info(j));
            ++e2;
          }
        }
      }
      std::cout << "\nERROR ShortMM " << e1
                << "  LongMM " << e2
                << '\n';
    }
    */

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

    std::vector<rank_tuple<sss_type>> rank_tuples;
    rank_tuples.resize(strings_to_sort.size());
#pragma omp parallel
    {
      const int t = omp_get_thread_num();
      const int nt = omp_get_num_threads();
      const size_t size_per_thread = rank_tuples.size() / nt;
      const size_t start_i = t * size_per_thread;
      const size_t end_i = (t == nt - 1) ? rank_tuples.size() : (t + 1) * size_per_thread;

      sss_type cur_rank = start_i + 1;
      rank_tuples[start_i] = {static_cast<sss_type>(strings_to_sort[start_i]), cur_rank};

      for (size_t i = start_i + 1; i < end_i; ++i) {
        uint64_t const max_length = std::min({text_size - strings_to_sort[i], text_size - strings_to_sort[i - 1], 3 * kTau});
        uint64_t text_lce = lce_in_text(strings_to_sort[i], strings_to_sort[i - 1], max_length);
        if (text_lce < max_length || sync_set.get_run_info(strings_to_sort[i]) != sync_set.get_run_info(strings_to_sort[i - 1])) {
          ++cur_rank;
        }
        rank_tuples[i] = {static_cast<sss_type>(strings_to_sort[i]), cur_rank};
      }

#pragma omp barrier
      // Check whether first string is equal to last string of last block.
      if (t != 0) {
        // If strings at borders are equal, we need to align rank of the latter
        uint64_t const max_length = std::min({text_size - strings_to_sort[start_i], text_size - strings_to_sort[start_i - 1], 3 * kTau});
        uint64_t text_lce = lce_in_text(strings_to_sort[start_i], strings_to_sort[start_i - 1], max_length);
        if (!(text_lce < max_length || sync_set.get_run_info(strings_to_sort[start_i]) != sync_set.get_run_info(strings_to_sort[start_i - 1]))) {
          // if (text_lce == max_length && sync_set.get_run_info(strings_to_sort[start_i] == sync_set.get_run_info(strings_to_sort[start_i - 1]))) {
          const size_t rank_to_decrease = rank_tuples[start_i].rank;
          for (size_t i = start_i; i < end_i && rank_tuples[i].rank == rank_to_decrease; ++i) {
            rank_tuples[i].rank = rank_tuples[i - 1].rank;
          }
        }
      }
    }
    size_t max_rank = rank_tuples.back().rank + 1;
    // Check strings to sort
    /*
    {
      size_t e1 = 0;
      size_t e2 = 0;
      size_t e3 = 0;
      for (size_t idx = 1; idx < rank_tuples.size(); ++idx) {
        size_t i = rank_tuples[idx - 1].index;
        size_t j = rank_tuples[idx].index;
        size_t i_rank = rank_tuples[idx - 1].rank;
        size_t j_rank = rank_tuples[idx].rank;
        size_t max_lce = std::min(3 * kTau, text_size - std::max(i, j));
        size_t text_lce = lce_in_text(i, j, 3 * kTau);
        if (i_rank > j_rank) {
          ++e1;
        }
        if (text_lce < max_lce) {
          if (i_rank >= j_rank) {
            ++e2;
          }
        } else {
          if (sync_set.get_run_info(i) > sync_set.get_run_info(j)) {
            //printf("i=%lu(%li) j=%lu(%li)", i, sync_set.get_run_info(i), j, sync_set.get_run_info(j));
            ++e3;
          }
        }
      }
      std::cout << "\nERROR RankMM " << e1
                << "  ShortRankMM " << e2
                << "  LongRankMM " << e3
                << '\n';
    }
    */

    ips4o::sort(rank_tuples.begin(), rank_tuples.end(),
                [](rank_tuple<sss_type> const& lhs, rank_tuple<sss_type> const& rhs) {
                  return lhs.index < rhs.index;
                });

    std::vector<sss_type> new_text(rank_tuples.size() + 1, 0);
    std::vector<sss_type> new_sa(new_text.size(), 0);
#pragma omp parallel for
    for (size_t i = 0; i < rank_tuples.size(); ++i) {
      new_text[i] = static_cast<int32_t>(rank_tuples[i].rank);
    }
    new_text.back() = 0;
    if constexpr (std::is_same<sss_type, uint32_t>::value) {
      libsais_main_32s_internal(reinterpret_cast<int32_t*>(new_text.data()), reinterpret_cast<int32_t*>(new_sa.data()), new_text.size(), max_rank + 1, 0, omp_get_num_threads());
    } else if constexpr (std::is_same<sss_type, uint64_t>::value) {
      libsais64_main_32s(reinterpret_cast<int32_t*>(new_text.data()), reinterpret_cast<int32_t*>(new_sa.data()), new_text.size(), max_rank + 1, 0, omp_get_num_threads());
    } else {
      std::cerr << "NO SUFFIX ARRAY CONSTRUCTION ALGORITHM FOR DATA TYPE\n";
    }

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
    for (uint64_t i = 0; i < new_sa.size(); ++i) {
      isa[new_sa[i]] = i;
    }

    lcp.resize(new_sa.size());
    lcp[0] = 0;
    lcp[1] = 0;
    size_t current_lcp = 0;
#pragma omp parallel for
    for (size_t i = 2; i < lcp.size(); ++i) {
      if (i != new_sa[0]) {
        size_t suffix_index = isa[i];
        size_t preceding_suffix = new_sa[suffix_index - 1];
        current_lcp += lce_in_text(sync_set[i] + current_lcp, sync_set[preceding_suffix] + current_lcp);
        lcp[suffix_index] = current_lcp;

        uint64_t diff = sync_set[i + 1] - sync_set[i];
        if (current_lcp < 2 * kTau + diff) {
          current_lcp = 0;
        } else {
          current_lcp -= diff;
        }
      }
    }

#ifdef DETAILED_TIME
    end = std::chrono::system_clock::now();
    std::cout << "lcp_construct_time="
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
              << "lcp_construct_mem=" << (malloc_count_peak() - mem_before) << " ";
#endif
    // Build RMQ data structure

#ifdef DETAILED_TIME
    mem_before = malloc_count_current();
    malloc_count_reset_peak();
    begin = std::chrono::system_clock::now();
#endif

    rmq_ds1 = std::make_unique<par_RMQ_n<sss_type>>(lcp);

#ifdef DETAILED_TIME
    end = std::chrono::system_clock::now();
    std::cout << "rmq_construct_time="
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
              << "rmq_construct_mem=" << (malloc_count_peak() - mem_before) << " ";
#endif
    /*
    {
      //check correctness of sa and lcp
      size_t e1{0};
      size_t e2{0};
      size_t e3{0};
      for (size_t idx = 2; idx < new_sa.size(); ++idx) {
        auto i = sync_set[new_sa[idx - 1]];
        auto j = sync_set[new_sa[idx]];
        auto lce = lce_in_text(i, j);

        //Check SA order
        if (j + lce == text_size) {
          //std::cout << "\nError EndCheck " << idx << "\n";
          ++e1;
        } else if (text[i + lce] > text[j + lce]) {

          // std::cout << "\nError LexCheck " << idx << "\n";
          // for (size_t k = 0; k <= lce; ++k) {
          //   std::cout << text[i + k];
          // }
          // std::cout << '\n';
          // for (size_t k = 0; k <= lce; ++k) {
          //   std::cout << text[j + k];
          // }
          // std::cout << '\n';
          // return;

          ++e2;
        }
        //Check LCP
        if (lcp[idx] != lce) {
          // std::cout << "\nError LCP " << idx << "  lcp[idx]=" << lcp[idx] << "  lce=" << lce << "\n";
          // for (size_t k = 0; k <= lce; ++k) {
          //   std::cout << text[i + k];
          // }
          // std::cout << '\n';
          // for (size_t k = 0; k <= lce; ++k) {
          //   std::cout << text[j + k];
          // }
          // std::cout << '\n';
          // return;
          ++e3;
        }
      }
      std::cout << "\nERROR Endcheck " << e1
                << "  LexCheck " << e2
                << "  LCP " << e3
                << '\n';
    }
    */
  }

  uint64_t
  lce(uint64_t i, uint64_t j) const {
    if (i == j) {
      return text_size - i;
    }

    auto min = std::min(isa[i], isa[j]) + 1;
    auto max = std::max(isa[i], isa[j]);
    // printf("i %lu, j %lu, min %lu, max %lu, isai %lu, isaj %lu\n", i, j, min, max, isa[i], isa[j]);
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

  std::vector<sss_type> isa;
  std::vector<sss_type> lcp;
  std::unique_ptr<par_RMQ_n<sss_type>> rmq_ds1;

  uint64_t lce_in_text(uint64_t i, uint64_t j, uint64_t up_to = std::numeric_limits<uint64_t>::max()) {
    uint64_t maxLce = text_size - std::max(i, j);
    maxLce = std::min(maxLce, up_to);
    uint64_t lce_naive = 0;
    while (lce_naive < maxLce) {
      if (text[i + lce_naive] != text[j + lce_naive]) {
        return lce_naive;
      }
      ++lce_naive;
    }
    return lce_naive;
  }
};
}  // namespace lce_test::par

/******************************************************************************/
