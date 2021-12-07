/*******************************************************************************
 * lce-test/lce_prezza.hpp
 *
 * Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <algorithm>

#include "util/lce_interface.hpp"
#include "util/util.hpp"
#include <cmath>
#include <bit>
#include <assert.h>

/* This class builds Prezza's in-place LCE data structure and
 * answers LCE-queries in O(log(n)). */
template <uint64_t t_naive_scan = 128>
class LcePrezza : public LceDataStructure {

/* Calculates the powers of 2. This supports LCE queries and reduces the time
     from polylogarithmic to logarithmic. */
  static constexpr std::array<uint64_t, 70> calculatePowers() {
    std::array<uint64_t, 70> powers;
    uint128_t x = 256;
    powers[0] = static_cast<uint64_t>(x);
    for (size_t i = 1; i < powers.size(); ++i) {
      x = (x*x) % prime_;
      powers[i] = static_cast<uint64_t>(x);
    }
    return powers;
  }

public:
  __extension__ typedef unsigned __int128 uint128_t;
  LcePrezza() = delete;
  /* Loads the full file located at PATH and builds Prezza's LCE data structure */
  LcePrezza(uint64_t * const text, size_t const size)
  : text_length_in_bytes_(size),
    text_length_in_blocks_(size / 8 + (size % 8 == 0 ? 0 : 1)),
    fingerprints_(text) {
      calculateFingerprints();
  }


  uint64_t lce_scan(const uint64_t i, const uint64_t j, uint64_t max_lce) {
    uint64_t lce = 0;
    /* naive part of lce query */
    /* compare blockwise */
    const int offset_lce1 = (i % 8) * 8;
    const int offset_lce2 = (j % 8) * 8;
    uint64_t block_i = getBlock(i/8);
    uint64_t block_i2 = getBlockGuaranteeIgeqOne(i/8 + 1);
    uint64_t block_j = getBlock(j/8);
    uint64_t block_j2 = getBlockGuaranteeIgeqOne(j/8 + 1);
    uint64_t comp_block_i = (block_i << offset_lce1) +
      ((block_i2 >> 1) >> (63-offset_lce1));
    uint64_t comp_block_j = (block_j << offset_lce2) +
      ((block_j2 >> 1) >> (63-offset_lce2));

    const uint64_t max_block_naive = max_lce < t_naive_scan ? max_lce/8 : t_naive_scan/8;
    while(lce < max_block_naive) {
      if(comp_block_i != comp_block_j) {
        break;
      }
      ++lce;
      block_i = block_i2;
      block_i2 = getBlockGuaranteeIgeqOne((i/8)+lce+1);
      block_j = block_j2;
      block_j2 = getBlockGuaranteeIgeqOne((j/8)+lce+1);
      comp_block_i = (block_i << offset_lce1) +
        ((block_i2 >> 1) >> (63-offset_lce1));
      comp_block_j = (block_j << offset_lce2) +
        ((block_j2 >> 1) >> (63-offset_lce2));
    }
    lce *= 8;
    /* If everything except the stub matches, we compare the stub character-wise
       and return the result */
    if(lce != t_naive_scan) {
      uint64_t max_stub = std::min((max_lce - lce), uint64_t{8});
      return lce + std::min<uint64_t>(((std::countl_zero(comp_block_i ^ comp_block_j)) / 8), max_stub);
    }
    return t_naive_scan;
  }
  
  
  
  /* Fast LCE-query in O(log(n)) time */
  uint64_t lce(const uint64_t i, const uint64_t j) {
    if (i == j) [[unlikely]] {
      return text_length_in_bytes_ - i;
    }
    uint64_t max_lce = text_length_in_bytes_ - ((i < j) ? j : i);
    uint64_t lce = lce_scan(i, j, max_lce);
    if(lce < t_naive_scan) {
      return lce;
    }
    /* exponential search */    
    uint64_t dist = t_naive_scan * 2;
    int exp = std::countr_zero(dist);

    const uint128_t fingerprint_to_i = (i != 0) ? fingerprintTo(i - 1) : 0;
    const uint128_t fingerprint_to_j = (j != 0) ? fingerprintTo(j - 1) : 0;

    while (dist <= max_lce &&
           fingerprintExp(fingerprint_to_i, i, exp) == fingerprintExp(fingerprint_to_j, j, exp)) {
      ++exp;
      dist *= 2;
    }

    /* binary search , we start it at i2 and j2, because we know that 
     * up until i2 and j2 everything matched */
    --exp;
    dist /= 2;
    uint64_t add = dist;

    while(dist > t_naive_scan) {
      --exp;
      dist /= 2;
      if(fingerprintExp(i + add, exp) == fingerprintExp(j + add, exp)) {
        add += dist;
      }
    }
    max_lce -= add;
    return add + lce_scan_to_end(i + add, j + add, max_lce);
  }

  /* Returns the prime*/
  uint128_t getPrime() const {
    return prime_;
  }

  /* Returns the character at index i */ 
  char operator[] (const uint64_t i) {
    uint64_t block_number = i / 8;
    uint64_t offset = 7 - (i % 8);
    return (getBlock(block_number)) >> (8*offset) & 0xff;
  }

  int isSmallerSuffix(const uint64_t i, const uint64_t j) {
    uint64_t lce_s = lce(i, j);
    if(i + lce_s + 1 == text_length_in_bytes_) [[unlikely]] { return true;}
    if(j + lce_s + 1 == text_length_in_bytes_) [[unlikely]] { return false;}
    return (operator[](i + lce_s) < operator[](j + lce_s));
  }

  uint64_t getSizeInBytes() {
    return text_length_in_bytes_;
  }

  void retransform_text() {
    for(size_t i{text_length_in_blocks_}; i > 0; --i) {
      fingerprints_[i] = getBlockGuaranteeIgeqOne(i);
      fingerprints_[i] = __builtin_bswap64(fingerprints_[i]);
    }
    fingerprints_[0] &= 0x7FFFFFFFFFFFFFFFULL;
  }

private:
  uint64_t text_length_in_bytes_;
  uint64_t text_length_in_blocks_;
  static constexpr uint128_t prime_{0x800000000000001d};


  uint64_t * fingerprints_; //We overwrite the text and store the pointer here;
  static constexpr std::array<uint64_t, 70> power_table_ = calculatePowers();

  uint64_t lce_scan_to_end(const uint64_t i, const uint64_t j, uint64_t max_lce) {
    uint64_t lce = 0;
    /* naive part of lce query */
    /* compare blockwise */
    const int offset_lce1 = (i % 8) * 8;
    const int offset_lce2 = (j % 8) * 8;
    uint64_t block_i = getBlock(i/8);
    uint64_t block_i2 = getBlockGuaranteeIgeqOne(i/8 + 1);
    uint64_t block_j = getBlock(j/8);
    uint64_t block_j2 = getBlockGuaranteeIgeqOne(j/8 + 1);
    uint64_t comp_block_i = (block_i << offset_lce1) +
      ((block_i2 >> 1) >> (63-offset_lce1));
    uint64_t comp_block_j = (block_j << offset_lce2) +
      ((block_j2 >> 1) >> (63-offset_lce2));

    while(lce < max_lce) {
      if(comp_block_i != comp_block_j) {
        break;
      }
      ++lce;
      block_i = block_i2;
      block_i2 = getBlockGuaranteeIgeqOne((i/8)+lce+1);
      block_j = block_j2;
      block_j2 = getBlockGuaranteeIgeqOne((j/8)+lce+1);
      comp_block_i = (block_i << offset_lce1) +
        ((block_i2 >> 1) >> (63-offset_lce1));
      comp_block_j = (block_j << offset_lce2) +
        ((block_j2 >> 1) >> (63-offset_lce2));
    }
    lce *= 8;
    /* If everything except the stub matches, we compare the stub character-wise
 *        and return the result */
   
      uint64_t max_stub = std::min((max_lce - lce), uint64_t{8});
      return lce + std::min<uint64_t>(((std::countl_zero(comp_block_i ^ comp_block_j)) / 8), max_stub);
  }


  /* Returns the i'th block. A block contains 8 character. */
  uint64_t getBlock(const uint64_t i) const {
    uint128_t x = (i != 0) ? fingerprints_[i - 1] & 0x7FFFFFFFFFFFFFFFULL : 0;
    x <<= 64;
    x %= prime_;

    uint64_t current_fingerprint = fingerprints_[i];
    uint64_t s_bit = current_fingerprint >> 63;
    current_fingerprint &= 0x7FFFFFFFFFFFFFFFULL;

    uint64_t y = static_cast<uint64_t>(x);

    y = y <= current_fingerprint ?
      current_fingerprint - y : prime_ - (y - current_fingerprint);
    return y + s_bit*static_cast<uint64_t>(prime_);
  }

  /* Retruns the i'th block for i > 0. */
  uint64_t getBlockGuaranteeIgeqOne(const uint64_t i) const {
    assert(i >= 1);
    uint128_t x = fingerprints_[i - 1] & 0x7FFFFFFFFFFFFFFFULL;
    x <<= 64;
    x %= prime_;

    uint64_t current_fingerprint = fingerprints_[i];
    uint64_t s_bit = current_fingerprint >> 63;
    current_fingerprint &= 0x7FFFFFFFFFFFFFFFULL;

    uint64_t y = static_cast<uint64_t>(x);

    y = y <= current_fingerprint ?
      current_fingerprint - y : prime_ - (y - current_fingerprint);
    return y + s_bit*static_cast<uint64_t>(prime_);
  }

  /* Calculates the fingerprint of T[from, from + 2^exp) when the fingerprint
     of T[1, from) is already knwon */
  uint64_t fingerprintExp(uint128_t fingerprint_to_i,
                          const uint64_t from, const int exp) const {
    uint128_t fingerprint_to_j = fingerprintTo(from + (1 << exp) - 1);
    fingerprint_to_i *= power_table_[exp];
    fingerprint_to_i %= prime_;

    return fingerprint_to_j >= fingerprint_to_i ?
      static_cast<uint64_t>(fingerprint_to_j - fingerprint_to_i) :
      static_cast<uint64_t>(prime_ - (fingerprint_to_i - fingerprint_to_j));
  }

  /* Calculates the fingerprint of T[from, from + 2^exp) */
  uint64_t fingerprintExp(const uint64_t from, const int exp) const {
    uint128_t fingerprint_to_i = (from != 0) ? fingerprintTo(from - 1) : 0;
    uint128_t fingerprint_to_j = fingerprintTo(from + (1 << exp) - 1);
    fingerprint_to_i *= power_table_[exp];
    fingerprint_to_i %= prime_;

    return fingerprint_to_j >= fingerprint_to_i ?
      static_cast<uint64_t>(fingerprint_to_j - fingerprint_to_i) :
      static_cast<uint64_t>(prime_ - (fingerprint_to_i - fingerprint_to_j));
  }

  /* Calculates the fingerprint of T[0..i] */
  uint64_t fingerprintTo(const uint64_t i) const {
    uint128_t fingerprint = 0;
    int pad = ((i+1) & 7) * 8; 
    if(pad == 0) [[unlikely]] {
      // This fingerprints is already saved.
      // We only have to remove the helping bit.
      return fingerprints_[i/8] & 0x7FFFFFFFFFFFFFFFULL; 
    }
    /* Add fingerprint from previous block */
    if (i > 7) [[likely]] {
      fingerprint = fingerprints_[(i/8) - 1] & 0x7FFFFFFFFFFFFFFFULL;
      fingerprint <<= pad;    
      uint64_t y = getBlockGuaranteeIgeqOne(i/8);
      fingerprint += (y >> (64 - pad));
      
    } else {
      const uint64_t current_fingerprint = fingerprints_[0];
      const uint64_t s_bit = current_fingerprint >> 63;
      
      fingerprint = current_fingerprint & 0x7FFFFFFFFFFFFFFFULL;
      fingerprint += (s_bit*prime_);
      fingerprint >>= (64 - pad);
    }

    fingerprint %= prime_;
    return static_cast<uint64_t>(fingerprint);
  }

  /* Overwrites the n'th block with the fingerprint of the first n blocks.
     Because the Rabin-Karp fingerprint uses a rolling hash function,
     this is done in O(n) time. */
  void calculateFingerprints() {
    /* For small endian systems we need to swap the order of bytes in order to
      calculate fingerprints. Luckily this step is fast. */
    if constexpr (std::endian::native == std::endian::little) {
      for(size_t i = 0; i < text_length_in_blocks_; ++i) {
        fingerprints_[i] = __builtin_bswap64(fingerprints_[i]); //C++23 std::byteswap!
      }
    }
    uint128_t previous_fingerprint = 0;
    uint64_t current_block = fingerprints_[0];

    for (uint64_t i = 0; i < text_length_in_blocks_; ++i) {
      current_block = fingerprints_[i];
      uint128_t x = previous_fingerprint;
      x <<= 64;
      x += current_block;
      x = x % prime_;
      previous_fingerprint = static_cast<uint64_t>(x);

      /* Additionally store if block > prime */
      if(current_block > prime_) {
        x = x + 0x8000000000000000ULL;
      }
      fingerprints_[i] = (uint64_t) x;
    }
  }
};

/******************************************************************************/
