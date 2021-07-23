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

#include "util_ssss_par/ssss_par.hpp"
#include "util/lce_interface.hpp"
#include "util/successor/index_par.hpp"

#include "util_ssss_par/lce-rmq.hpp"
#include "util/util.hpp"

#ifdef DETAILED_TIME
#include <malloc_count.h>
#endif

namespace lce_test::par {

/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */
template <uint64_t kTau = 1024, bool prefer_long = true>
class LceSemiSyncSetsPar : public LceDataStructure {
 public:
  using sss_type = uint32_t;

 public:
  LceSemiSyncSetsPar(std::vector<uint8_t> const& text, bool const print_ss_size)
      : text_(text), text_length_in_bytes_(text_.size()) {
#ifdef DETAILED_TIME
    size_t mem_before = malloc_count_current();
    malloc_count_reset_peak();
    std::chrono::system_clock::time_point begin = std::chrono::system_clock::now();
#endif

    sync_set_ = string_synchronizing_set_par<kTau, sss_type>(text_).get_sss();

#ifdef DETAILED_TIME
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    if (print_ss_size) {
      std::cout << "sss_construct_time="
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
                << "sss_construct_mem=" << (malloc_count_peak() - mem_before) << " ";
    }
#endif

#ifdef DETAILED_TIME
    mem_before = malloc_count_current();
    malloc_count_reset_peak();
    begin = std::chrono::system_clock::now();
#endif

    ind_ = std::make_unique<stash::pred::index_par<std::vector<sss_type>, sss_type, 7>>(sync_set_);

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
                                                         sync_set_,
                                                         print_ss_size);
    if (print_ss_size) {
      std::cout << "sync_set_size=" << sync_set_.size() << " ";
    }
  }

  /* Answers the lce query for position i and j */
  inline uint64_t lce(const uint64_t i, const uint64_t j) {
    if (TLX_UNLIKELY(i == j)) {
      return text_length_in_bytes_ - i;
    }

    if constexpr (prefer_long) {
      uint64_t const i_ = suc(i + 1);
      uint64_t const j_ = suc(j + 1);
      uint64_t const dist_i = sync_set_[i_] - i;
      uint64_t const dist_j = sync_set_[j_] - j;

      uint64_t max_length = 0;
      uint64_t lce = 0;
      if (dist_i == dist_j) {
        max_length = (i > j) ? ((i + dist_i > text_length_in_bytes_) ? i + dist_i - text_length_in_bytes_ : dist_i) : ((j + dist_i > text_length_in_bytes_) ? j + dist_i - text_length_in_bytes_ : dist_i);
      } else {
        max_length = 2 * kTau + std::min(dist_i, dist_j);
      }

      for (; lce < 8; ++lce) {
        if (text_[i + lce] != text_[j + lce]) {
          return lce;
        }
      }

      lce = 0;
      unsigned __int128 const* const text_blocks_i =
          reinterpret_cast<unsigned __int128 const*>(text_.data() + i);
      unsigned __int128 const* const text_blocks_j =
          reinterpret_cast<unsigned __int128 const*>(text_.data() + j);
      for (; lce < max_length / 16; ++lce) {
        if (text_blocks_i[lce] != text_blocks_j[lce]) {
          break;
        }
      }
      lce *= 16;
      // The last block did not match. Here we compare its single characters
      uint64_t lce_end = lce + ((16 < max_length) ? 16 : max_length);
      for (; lce < lce_end; ++lce) {
        if (text_[i + lce] != text_[j + lce]) {
          return lce;
        }
      }

      uint64_t const l = lce_rmq_->lce(i_, j_);
      return l + sync_set_[i_] - i;
    } else {
      /* naive part */
      uint64_t const sync_length = 3 * kTau;
      uint64_t const max_length = (i < j) ? ((sync_length + j > text_length_in_bytes_) ? text_length_in_bytes_ - j : sync_length) : ((sync_length + i > text_length_in_bytes_) ? text_length_in_bytes_ - i : sync_length);

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
      unsigned __int128 const* const text_blocks_i =
          reinterpret_cast<unsigned __int128 const*>(text_.data() + i);
      unsigned __int128 const* const text_blocks_j =
          reinterpret_cast<unsigned __int128 const*>(text_.data() + j);
      for (; lce < max_length / 16; ++lce) {
        if (text_blocks_i[lce] != text_blocks_j[lce]) {
          break;
        }
      }
      lce *= 16;
      // The last block did not match. Here we compare its single characters
      uint64_t lce_end = lce + ((16 < max_length) ? 16 : max_length);
      for (; lce < lce_end; ++lce) {
        if (text_[i + lce] != text_[j + lce]) {
          return lce;
        }
      }
      /* strSync part */
      uint64_t const i_ = suc(i + 1);
      uint64_t const j_ = suc(j + 1);

      uint64_t const l = lce_rmq_->lce(i_, j_);

      return l + sync_set_[i_] - i;
    }
  }

  char operator[](uint64_t i) {
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

 private:
  /* Finds the smallest element that is greater or equal to i
     Because s_ is ordered, that is equal to the 
     first element greater than i */
  inline uint64_t suc(uint64_t i) const {
    return ind_->successor(i).pos;
  }

 private:
  std::vector<uint8_t> const& text_;
  size_t const text_length_in_bytes_;

  std::unique_ptr<stash::pred::index_par<std::vector<uint32_t>, uint32_t, 7>> ind_;
  std::vector<sss_type> sync_set_;
  std::unique_ptr<Lce_rmq_par<sss_type, kTau>> lce_rmq_;
};
}  // namespace sss_par
/******************************************************************************/
