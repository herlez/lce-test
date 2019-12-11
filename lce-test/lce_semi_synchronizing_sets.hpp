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
#include <stash/pred/index.hpp>

#include <vector>
#include <memory>

static constexpr uint64_t calculatePowerModulo(unsigned int const power,
                                               __int128 const kPrime) {
  unsigned __int128 x = 256;
  for (unsigned int i = 0; i < power; i++) {
    x = (x*x) % kPrime;
  }
  return static_cast<uint64_t>(x);
}

/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */

class LceSemiSyncSets : public LceDataStructure {

public:
  static constexpr __int128 kPrime = 18446744073709551557ULL;
  static constexpr uint64_t kTau = 1024;
  static constexpr uint64_t TwoPowTauModQ = calculatePowerModulo(10, kPrime);

public:
  LceSemiSyncSets(std::vector<uint8_t> const& text)
    : text_(text), text_length_in_bytes_(text_.size()),
      s_bv_(std::make_unique<bit_vector>(text_length_in_bytes_)) {

    unsigned __int128 fp = 0;
    for(uint64_t i = 0; i < kTau; ++i) {
      fp *= 256;
      fp += (unsigned char) text_[i];
      fp %= kPrime;
    }
    ring_buffer<uint64_t> fingerprints(3*kTau);
    std::vector<uint64_t> s_fingerprints;
    fingerprints.push_back(static_cast<uint64_t>(fp));
    fill_synchronizing_set(0, (text_length_in_bytes_ - (2*kTau)), fp,
                           fingerprints, s_fingerprints);
                      

    for(size_t i = 0; i < sync_set_.size(); ++i) {
      s_bv_->bitset(sync_set_[i], 1);
    }
			
    s_bvr_ = std::make_unique<bit_vector_rank>(*s_bv_);

    lce_rmq_ = std::make_unique<Lce_rmq>(text_.data(), text_length_in_bytes_,
                                         sync_set_, s_fingerprints);
  }

  /* Answers the lce query for position i and j */
  inline uint64_t lce(const uint64_t i, const uint64_t j) {
    if (i == j) {
      return text_length_in_bytes_ - i;
    }
    /* naive part */
    for(unsigned int k = 0; k < (3*kTau-1); ++k) {
      if(text_[i+k] != text_[j+k]) {
        return k;
      }
    }
			
    /* strSync part */
    uint64_t const i_ = suc(i);
    uint64_t const j_ = suc(j);
    uint64_t const l = lce_rmq_->lce(i_, j_);			
    return l + sync_set_[i_] - i;
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
		
private:

  /* Finds the smallest element that is greater or equal to i
     Because s_ is ordered, that is equal to the 
     first element greater than i */
  inline uint64_t suc(uint64_t i) const {
    return s_bvr_->rank1(i);
  }

  void fill_synchronizing_set(const uint64_t from, const uint64_t to,
                              unsigned __int128& fp,
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
          local_min = i + kTau; 
        }
        i++;
        calculate_fingerprints(1, fp, fingerprints);
      }
    }
  }

  void calculate_fingerprints(size_t const count, unsigned __int128& fp,
                              ring_buffer<uint64_t>& fingerprints) {
    for(uint64_t i = 0; i < count; ++i) {
      fp *= 256;
      fp += (unsigned char) text_[kTau+fingerprints.size() - 1];
      fp %= kPrime;
				
      unsigned __int128 first_char_influence = text_[fingerprints.size() - 1];
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
  std::vector<uint8_t> const& text_;
  size_t const text_length_in_bytes_;

  std::vector<uint64_t> sync_set_;
  std::unique_ptr<bit_vector> s_bv_;
  std::unique_ptr<bit_vector_rank> s_bvr_;
  std::unique_ptr<Lce_rmq> lce_rmq_;
};

/******************************************************************************/
