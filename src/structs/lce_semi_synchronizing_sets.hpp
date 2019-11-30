#pragma once

#include "util/lce_interface.hpp"
#include "util/util.hpp"
#include "util/synchronizing_sets/bit_vector_rank.hpp"
#include "util/synchronizing_sets/lce-rmq.hpp"

#include <tlx/math/round_to_power_of_two.hpp>

#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <cmath>
#include <ctgmath>

#define unlikely(x)    __builtin_expect(!!(x), 0) 

template <typename DataType>
class ring_buffer {
public:
  ring_buffer(uint64_t const buffer_size)
    : buffer_size_(tlx::round_up_to_power_of_two(buffer_size)),
      mod_mask_(buffer_size_ - 1), size_(0), data_(buffer_size_) { }

  void push_back(DataType const data) {
    uint64_t const insert_pos = size_++ & mod_mask_;
    data_[insert_pos] = data;
  }

  size_t size() const {
    return size_;
  }

  DataType operator[](size_t const index) {
    return data_[index & mod_mask_];
  }

  DataType operator[](size_t const index) const {
    return data_[index & mod_mask_];
  }

private:
  uint64_t const buffer_size_;
  uint64_t const mod_mask_;

  uint64_t size_;

  std::vector<DataType> data_;

}; // class ring_buffer

/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */

class LceSemiSyncSets : public LceDataStructure {
public:
  /* Loads the full file located at PATH. */
  LceSemiSyncSets(std::string path) {
    buildStruct(path);
  }
		
  /* Loads a prefix of the file located at PATH */ 
  LceSemiSyncSets(std::string path, uint64_t number_of_chars) {
  }
		
  ~LceSemiSyncSets() {
  }
		
  /* Answers the lce query for position i and j */
  inline uint64_t lce(const uint64_t i, const uint64_t j) {
			
    timer.reset();
    if (i==j) {
      return text_length_in_bytes_ - i;
    }
    /* naive part */
    for(unsigned int k = 0; k < (3*kTau-1); ++k) {
      if(text_[i+k] != text_[j+k]) {
        return k;
      }
    }
			
    ts_naive += timer.elapsed();
			
    timer.reset();
    /* strSync part */
    uint64_t i_ = suc(i);
    uint64_t j_ = suc(j);
    ts_rank += timer.elapsed();
			
    timer.reset();
    uint64_t l = lce_rmq_->lce(i_, j_);
    ts_rmq_lce += timer.elapsed();
			
    return l + sync_set_[i_] - i;
  }
		
  char operator[](uint64_t i) {
    if(i > text_length_in_bytes_) {return '\00';}
    return text_[i];
  }
		
  int isSmallerSuffix(const uint64_t i, const uint64_t j) {
    return 0;
  }
		
  size_t getSizeInBytes() {
    return text_length_in_bytes_;
  }
		
		
  double getTimeNaive() {
    return ts_naive;
  }
  double getTimeRank() {
    return ts_rank;
  }
  double getTimeRmq() {
    return ts_rmq_lce;
  }
  void resetTimer() {
    ts_naive = 0.0;
    ts_rank = 0.0;
    ts_rmq_lce = 0.0;
  }
		
private:
  std::string text_;
  size_t text_length_in_bytes_;
		
  const unsigned __int128 kPrime = 18446744073709551557ULL;
  const uint64_t kTau = 1024;
  uint64_t two_pow_tau_mod_q_;

		
  std::vector<uint64_t> s_;
  std::vector<uint64_t> sync_set_;
  bit_vector * s_bv_;
  bit_vector_rank * s_bvr_;
		
  Lce_rmq * lce_rmq_;
		
  double ts_naive, ts_rank, ts_rmq_lce;
  util::Timer timer;

  /* Finds the smallest element that is greater or equal to i
     Because s_ is ordered, that is equal to the 
     first element greater than i */
  inline uint64_t suc(uint64_t i) const {
    return s_bvr_->rank1(i);
  }

  void fill_synchronizing_set(const uint64_t from, const uint64_t to,
                              unsigned __int128& fp,
                              ring_buffer<uint64_t>& fingerprints) {

    uint64_t min;
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
      }
				
				
      uint64_t local_min = i + min;
      ++i;
      calculate_fingerprints(1, fp, fingerprints);
      while(i < to && i < local_min) {
        if(fingerprints[i+kTau] <= fingerprints[local_min]) {
          sync_set_.push_back(i);
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
      first_char_influence *= two_pow_tau_mod_q_;
      first_char_influence %= kPrime;
				
      if(first_char_influence < fp) {
        fp -= first_char_influence;
      } else {
        fp = kPrime - (first_char_influence - fp);
      }
      fingerprints.push_back(static_cast<uint64_t>(fp));
    }
  }
		
		
  void buildStruct(std::string path) {
    std::ifstream input(path);
    input.seekg(0);
    util::inputErrorHandling(&input);
    std::stringstream buffer;
    buffer << input.rdbuf();
    text_ = buffer.str();
    text_length_in_bytes_ = text_.size();
    std::cout << "T size: " << text_.size() << std::endl;
			
    // Timer for benchmark
    util::Timer timer{};

    // Calculate fingerprints
    std::cout << "Calculating FP" << std::endl;
    unsigned __int128 fp = 0;
    for(uint64_t i = 0; i < kTau; ++i) {
      fp *= 256;
      fp += (unsigned char) text_[i];
      fp %= kPrime;
    }
    ring_buffer<uint64_t> fingerprints(3*kTau);
    fingerprints.push_back(static_cast<uint64_t>(fp));
    two_pow_tau_mod_q_ = calculatePowerModulo(10);
    fill_synchronizing_set(0, (text_length_in_bytes_ - (2*kTau)), fp, fingerprints);
    sync_set_.shrink_to_fit();
			
    s_bv_ = new bit_vector(text_length_in_bytes_);
    for(size_t i = 0; i < sync_set_.size(); ++i) {
      s_bv_->bitset(sync_set_[i], 1);
    }
			
    s_bvr_ = new bit_vector_rank(*s_bv_);
    lce_rmq_ = new Lce_rmq(&text_, &sync_set_);
  }
		
  uint64_t calculatePowerModulo(unsigned int power) {
    //powerTable = new uint64_t[numberOfLevels];
    unsigned __int128 x = 256;
    for (unsigned int i = 0; i < power; i++) {
      x = (x*x) % kPrime;
    }
    return (uint64_t) x;
  }
};
