/*******************************************************************************
 * structs/lce_semi_synchronizing_sets.hpp
 *
 * Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include "util/lce_interface.hpp"
#include "util/synchronizing_sets/bit_vector_rank.hpp"
#include "util/synchronizing_sets/ring_buffer.hpp"
#include "util/synchronizing_sets/lce-rmq.hpp"
#include "util/util.hpp"
#include "util/successor/index.hpp"

#include <tlx/define/likely.hpp>
#include <chrono>
#include <cmath>
#include <vector>
#include <memory>

#ifdef DETAILED_TIME
#include <malloc_count.h>
#endif



/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */

template <uint64_t kTau = 1024, bool prefer_long = true>
class LceSemiSyncSets : public LceDataStructure {

public:
  __extension__ typedef unsigned __int128 uint128_t;

  // static constexpr uint128_t kPrime = 562949953421231ULL;
  // static constexpr uint128_t kPrime = 1125899906842597ULL;
  // static constexpr uint128_t kPrime = 2251799813685119ULL;
  // static constexpr uint128_t kPrime = 4503599627370449ULL;
  // static constexpr uint128_t kPrime = 9007199254740881ULL;
  // static constexpr uint128_t kPrime = 18014398509481951ULL;
  // static constexpr uint128_t kPrime = 36028797018963913ULL;
  // static constexpr uint128_t kPrime = 72057594037927931ULL;
  // static constexpr uint128_t kPrime = 144115188075855859ULL;
  // static constexpr uint128_t kPrime = 288230376151711717ULL;
  // static constexpr uint128_t kPrime = 576460752303423433ULL;
  // static constexpr uint128_t kPrime = 1152921504606846883ULL;
  // static constexpr uint128_t kPrime = 2305843009213693951ULL;

  using sss_type = uint32_t;

public:
  LceSemiSyncSets(std::vector<uint8_t> const& text, bool const print_ss_size)
    : text_(text), text_length_in_bytes_(text_.size()) {

    uint128_t fp = { 0ULL };
    for(uint64_t i = 0; i < kTau; ++i) {
      fp *= 256;
      fp += (unsigned char) text_[i];
      fp %= kPrime;
    }
    ring_buffer<uint64_t> fingerprints(4*kTau);

#ifdef DETAILED_TIME
    size_t mem_before = malloc_count_current();
    malloc_count_reset_peak();
    std::chrono::system_clock::time_point begin = std::chrono::system_clock::now();
#endif

    std::vector<uint64_t> s_fingerprints;
    fingerprints.push_back(static_cast<uint64_t>(fp));
    fill_synchronizing_set(0, (text_length_in_bytes_ - (2*kTau)), fp,
                           fingerprints, s_fingerprints);
    
#ifdef DETAILED_TIME
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    if (print_ss_size) {
      std::cout << "sss_construct_time=" 
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
                << "sss_construct_mem=" << (malloc_count_peak() - mem_before) << " "
                << "sss_size=" << sync_set_.size() << " ";
    }
#endif

#ifdef DETAILED_TIME
    mem_before = malloc_count_current();
    malloc_count_reset_peak();
    begin = std::chrono::system_clock::now();
#endif

    ind_ = std::make_unique<stash::pred::index<std::vector<sss_type>, sss_type, 7>>(sync_set_);

#ifdef DETAILED_TIME
    end = std::chrono::system_clock::now();
    if (print_ss_size) {
      std::cout << "pred_construct_time=" 
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " "
                << "pred_construct_mem=" << (malloc_count_peak() - mem_before) << " ";
    }
#endif

    lce_rmq_ = std::make_unique<Lce_rmq<sss_type, kTau>>(text_.data(),
                                                         text_length_in_bytes_,
                                                         sync_set_,
                                                         print_ss_size);
    if (print_ss_size) {
      std::cout << "sync_set_size=" << getSyncSetSize() << " ";
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
        max_length = (i > j) ?
          ((i + dist_i > text_length_in_bytes_) ?
           i + dist_i - text_length_in_bytes_ : dist_i) :
          ((j + dist_i > text_length_in_bytes_) ?
           j + dist_i - text_length_in_bytes_ : dist_i);
      } else {
        max_length = 2 * kTau + std::min(dist_i, dist_j);
      }

      for (; lce < 8; ++lce) {
        if(text_[i + lce] != text_[j + lce]) {
          return lce;
        }
      }

      lce = 0;
      uint128_t const* const text_blocks_i =
        reinterpret_cast<uint128_t const*>(text_.data() + i);
      uint128_t const * const text_blocks_j =
        reinterpret_cast<uint128_t const *>(text_.data() + j);
      for(; lce < max_length/16; ++lce) {
        if(text_blocks_i[lce] != text_blocks_j[lce]) {
          break;
        }
      }
      lce *= 16;
      // The last block did not match. Here we compare its single characters
      uint64_t lce_end = lce + ((16 < max_length) ? 16 : max_length);
      for (; lce < lce_end; ++lce) {
        if(text_[i + lce] != text_[j + lce]) {
          return lce;
        }
      }

      uint64_t const l = lce_rmq_->lce(i_, j_);
      return l + sync_set_[i_] - i;
    } else {
      /* naive part */
      uint64_t const sync_length = 3 * kTau;
      uint64_t const max_length = (i < j) ?
        ((sync_length + j > text_length_in_bytes_) ?
         text_length_in_bytes_ - j  :
         sync_length) :
        ((sync_length + i > text_length_in_bytes_) ?
         text_length_in_bytes_ - i  :
         sync_length);

      uint64_t lce = 0;
      for (; lce < 8; ++lce) {
        if (TLX_UNLIKELY(lce >= max_length)) {
          return max_length;
        }
        if(text_[i + lce] != text_[j + lce]) {
          return lce;
        }
      }

      lce = 0;
      uint128_t const* const text_blocks_i =
        reinterpret_cast<uint128_t const*>(text_.data() + i);
      uint128_t const * const text_blocks_j =
        reinterpret_cast<uint128_t const *>(text_.data() + j);
      for(; lce < max_length/16; ++lce) {
        if(text_blocks_i[lce] != text_blocks_j[lce]) {
          break;
        }
      }
      lce *= 16;
      // The last block did not match. Here we compare its single characters
      uint64_t lce_end = lce + ((16 < max_length) ? 16 : max_length);
      for (; lce < lce_end; ++lce) {
        if(text_[i + lce] != text_[j + lce]) {
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
    if(i > text_length_in_bytes_) {return '\00';}
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
    //return s_bvr_->rank1(i);
    return ind_->successor(i).pos;
  }

  void fill_synchronizing_set(const uint64_t from, const uint64_t to,
                              uint128_t& fp,
                              ring_buffer<uint64_t>& fingerprints,
                              std::vector<uint64_t>& out_s_fingerprints) {

    uint64_t min = 0;
    for (uint64_t i = from; i < to; ) {
      // Compare this id with every other index which is not in q
      min = 0;
      size_t required = i + kTau - (fingerprints.size() - 1);
      calculate_fingerprints(required, fp, fingerprints);
      for (unsigned int j = 1; j <= kTau; ++j) {
        if(fingerprints[i+j] < fingerprints[i+min]) {
          min = j;
        }
      }
      if(min == 0 || min == kTau) {
        sync_set_.push_back(i);
        out_s_fingerprints.push_back(fingerprints[i]);
      }
            
            
      uint64_t local_min = i + min;
      ++i;
      calculate_fingerprints(1, fp, fingerprints);
      while(i < to && i < local_min) {
        if(fingerprints[i+kTau] <= fingerprints[local_min]) {
          sync_set_.push_back(i);
          out_s_fingerprints.push_back(fingerprints[i]);
          if(fingerprints[i+kTau] != fingerprints[local_min]) {
            local_min = i + kTau; 
          }
        }
        i++;
        calculate_fingerprints(1, fp, fingerprints);
      }
    }
  }

  void calculate_fingerprints(size_t const count, uint128_t& fp,
                              ring_buffer<uint64_t>& fingerprints) {
    for(uint64_t i = 0; i < count; ++i) {
      fp *= 256;
      fp += (unsigned char) text_[kTau+fingerprints.size() - 1];
      fp %= kPrime;
                
      uint128_t first_char_influence = text_[fingerprints.size() - 1];
      first_char_influence *= TwoPowTauModQ;
      first_char_influence %= kPrime;
                
      if(first_char_influence < fp) {
        fp -= first_char_influence;
      } else {
        fp = kPrime - (first_char_influence - fp);
      }
      fingerprints.push_back(static_cast<uint64_t>(fp));
    }
  }

private:
  static constexpr uint64_t calculatePowerModulo(unsigned int const power,
                                               uint128_t const kPrime) {
  uint128_t x = 256;
  for (unsigned int i = 0; i < power; i++) {
    x = (x*x) % kPrime;
  }
  return static_cast<uint64_t>(x);
  }
  
  static constexpr uint128_t kPrime = 18446744073709551253ULL;
  static constexpr uint64_t TwoPowTauModQ = calculatePowerModulo(std::log2(kTau), kPrime);

  std::vector<uint8_t> const& text_;
  size_t const text_length_in_bytes_;
  
  std::unique_ptr<stash::pred::index<std::vector<uint32_t>, uint32_t, 7>> ind_;
  std::vector<sss_type> sync_set_;
  std::unique_ptr<Lce_rmq<sss_type, kTau>> lce_rmq_;
};

/******************************************************************************/
