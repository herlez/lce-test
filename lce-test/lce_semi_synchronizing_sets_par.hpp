/*******************************************************************************
 * structs/lce_semi_synchronizing_sets.hpp
 *
 * Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <chrono>
#include <cmath>
#include <memory>
#include <tlx/define/likely.hpp>
#include <vector>

#include "util/lce_interface.hpp"
#include "util/successor/index_par.hpp"
#include "util/util.hpp"
#include "util_ssss_par/lce-rmq.hpp"
#include "util_ssss_par/ssss_par.hpp"
#include "util_ssss_par/sss_checker.hpp"

#ifdef DETAILED_TIME
#include <malloc_count.h>
#endif

namespace lce_test::par {
__extension__ typedef unsigned __int128 uint128_t;
/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */
template <uint64_t kTau = 1024>
class LceSemiSyncSetsPar : public LceDataStructure {
 public:
  using sss_type = uint64_t;

 public:
  LceSemiSyncSetsPar(std::vector<uint8_t> const& text, bool const print_ss_size)
      : text_(text), text_length_in_bytes_(text_.size()) {
#ifdef DETAILED_TIME
    size_t mem_before = malloc_count_current();
    malloc_count_reset_peak();
    std::chrono::system_clock::time_point begin = std::chrono::system_clock::now();
#endif

    sync_set_ = string_synchronizing_set_par<kTau, sss_type>(text_);
    //check_string_synchronizing_set(text, sync_set_);

#ifdef DETAILED_TIME
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    if (print_ss_size) {
      std::cout << "sss_construct_time="
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
                << "sss_construct_mem=" << (malloc_count_peak() - mem_before) << " "
                << "sss_size=" << sync_set_.size() << " "
                << "sss_repetetive=" << std::boolalpha << sync_set_.has_runs() << " "
                << "sss_runs=" << sync_set_.num_runs() << " ";
    }
#endif

#ifdef DETAILED_TIME
    mem_before = malloc_count_current();
    malloc_count_reset_peak();
    begin = std::chrono::system_clock::now();
#endif

    ind_ = std::make_unique<stash::pred::index_par<std::vector<sss_type>, sss_type, 7>>(sync_set_.get_sss());

#ifdef DETAILED_TIME
    end = std::chrono::system_clock::now();
    if (print_ss_size) {
      std::cout << "pred_construct_time="
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
                << "pred_construct_mem=" << (malloc_count_peak() - mem_before) << " ";
    }
#endif
    lce_rmq_ = std::make_unique<Lce_rmq_par<sss_type, kTau>>(text_.data(),
                                                             text_length_in_bytes_,
                                                             sync_set_);
  }

  /* Answers the lce query for position i and j */
  inline uint64_t lce(uint64_t i, uint64_t j) {
    if (TLX_UNLIKELY(i == j)) {
      return text_length_in_bytes_ - i;
    }
    if (i > j) {
      std::swap(i, j);
    }

    /* naive part */
    uint64_t const sync_length = 3 * kTau;
    uint64_t const max_length = std::min(sync_length, text_length_in_bytes_ - j);
    uint64_t lce = 0;
    for (; lce < 8; ++lce) {
      if (TLX_UNLIKELY(lce >= max_length)) {
        return max_length;
      }
      if (text_[i + lce] != text_[j + lce]) {
        return lce;
      }
    }

    lce = 0;
    uint128_t const* const text_blocks_i =
        reinterpret_cast<uint128_t const*>(text_.data() + i);
    uint128_t const* const text_blocks_j =
        reinterpret_cast<uint128_t const*>(text_.data() + j);
    for (; lce < max_length / 16; ++lce) {
      if (text_blocks_i[lce] != text_blocks_j[lce]) {
        lce *= 16;
        // The last block did not match. Here we compare its single characters
        uint64_t lce_end = std::min(lce + 16, max_length);
        for (; lce < lce_end; ++lce) {
          if (text_[i + lce] != text_[j + lce]) {
            return lce;
          }
        }
        return lce;
      }
    }
    lce *= 16;

    /* strSync part */
    uint64_t const i_ = suc(i + 1);
    uint64_t const j_ = suc(j + 1);

    uint64_t const i_diff = sync_set_[i_] - i;
    uint64_t const j_diff = sync_set_[j_] - j;

    if (i_diff == j_diff) {
      return i_diff + lce_rmq_->lce(i_, j_);
    } else {
      return std::min(i_diff, j_diff) + 2 * kTau - 1;
    }
  }
  char operator[](size_t i) {
    if (i > text_length_in_bytes_) {
      return '\00';
    }
    return text_[i];
  }

  int isSmallerSuffix([[maybe_unused]] const uint64_t i,
                      [[maybe_unused]] const uint64_t j) {
    return 0;
  }

  size_t getSizeInBytes() {
    return text_length_in_bytes_;
  }

  size_t getSyncSetSize() {
    return sync_set_.size();
  }

  std::vector<sss_type> getSyncSet() {
    return sync_set_.get_sss();
  }

 private:
  /* Finds the smallest element that is greater or equal to i
     Because s_ is ordered, that is equal to the 
     first element greater than i */
  inline sss_type suc(sss_type i) const {
    return ind_->successor(i).pos;
    
    /*size_t aprx_pos = std::min(1.0*i/text_.size() * sync_set_.size(), static_cast<double>(sync_set_.size())) + 67; 
    size_t scan_pos = aprx_pos;
    if(aprx_pos != 0 && sync_set_[scan_pos-1] > i) {
      //scan left
      while(scan_pos > 0 && sync_set_[scan_pos] >= i) {
        --scan_pos;
      } 
      ++scan_pos;
    } else {
      while(scan_pos < sync_set_.size() && sync_set_[scan_pos] < i) {
        //scan right
        ++scan_pos;
      }
    }
    err.push_back(static_cast<int64_t>(aprx_pos) - static_cast<int64_t>(scan_pos));
    if(err.size() == 1'000'000) {
      std::cout << "err_avg=" << std::accumulate(err.begin(), err.end(), 0.0)/err.size() << " ";
      for(auto& i : err) {
        i = std::abs(i);
      }
      std::cout << "err_abs_avg=" << std::accumulate(err.begin(), err.end(), 0.0)/err.size() << " ";
    }
    return scan_pos;*/
  }

 private:
  std::vector<uint8_t> const& text_;
  mutable std::vector<int64_t> err;
  size_t const text_length_in_bytes_;

  std::unique_ptr<stash::pred::index_par<std::vector<sss_type>, sss_type, 7>> ind_;
  string_synchronizing_set_par<kTau, sss_type> sync_set_;
  std::unique_ptr<Lce_rmq_par<sss_type, kTau>> lce_rmq_;
};
}  // namespace lce_test::par
/******************************************************************************/
