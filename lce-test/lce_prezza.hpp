/*******************************************************************************
 * lce-test/lce_prezza.hpp
 *
 * Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <algorithm>

#include <tlx/define/likely.hpp>

#include "util/lce_interface.hpp"
#include "util/util.hpp"

/* This class builds Prezza's in-place LCE data structure and
 * answers LCE-queries in O(log(n)). */
template <uint64_t t_naive_scan = 128>
class LcePrezza : public LceDataStructure {
public:
  LcePrezza() = delete;
  /* Loads the full file located at PATH and builds Prezza's LCE data structure */
  LcePrezza(uint64_t * const text, size_t const size)
  : text_length_in_bytes_(size),
    text_length_in_blocks_(size / 8 + (size % 8 == 0 ? 0 : 1)),
    fingerprints_(text),
    power_table_(new uint64_t[((int) std::log2(text_length_in_blocks_)) + 6]) {
      calculateFingerprints();
      calculatePowers();
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
      unsigned int const max_stub = (max_lce - lce) < 8 ? (max_lce - lce) : 8;
      return lce + std::min<uint64_t>(((__builtin_clzl(comp_block_i ^ comp_block_j)) / 8), max_stub);
    }
    return t_naive_scan;
  }
  
  
  
  /* Fast LCE-query in O(log(n)) time */
  uint64_t lce(const uint64_t i, const uint64_t j) {
    if (TLX_UNLIKELY(i == j)) {
      return text_length_in_bytes_ - i;
    }
    uint64_t max_lce = text_length_in_bytes_ - ((i < j) ? j : i);
    uint64_t lce = lce_scan(i, j, max_lce);
    if(lce < t_naive_scan) {
      return lce;
    }
    /* exponential search */    
    uint64_t dist = t_naive_scan * 2;
    int exp = __builtin_ctz(t_naive_scan)+1;

    unsigned const __int128 fingerprint_to_i = (i != 0) ? fingerprintTo(i - 1) : 0;
    unsigned const __int128 fingerprint_to_j = (j != 0) ? fingerprintTo(j - 1) : 0;

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

    while(exp != __builtin_ctz(t_naive_scan)) {
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
  unsigned __int128 getPrime() const {
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
    if(TLX_UNLIKELY((i + lce_s + 1 == text_length_in_bytes_) ||
                    (j + lce_s + 1 == text_length_in_bytes_))) {
      return true;
    }
    return (operator[](i + lce_s) < operator[](j + lce_s));
  }

  uint64_t getSizeInBytes() {
    return text_length_in_bytes_;
  }

private:
  const uint64_t text_length_in_bytes_;
  const uint64_t text_length_in_blocks_;
  static constexpr unsigned __int128 prime_ = 0x800000000000001dULL;
  uint64_t * const fingerprints_;
  uint64_t * const power_table_;

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
   
      unsigned int const max_stub = (max_lce - lce) < 8 ? (max_lce - lce) : 8;
      return lce + std::min<uint64_t>(((__builtin_clzl(comp_block_i ^ comp_block_j)) / 8), max_stub);
  }


  /* Returns the i'th block. A block contains 8 character. */
  uint64_t getBlock(const uint64_t i) const {
    unsigned __int128 x = (i != 0) ? fingerprints_[i - 1] & 0x7FFFFFFFFFFFFFFFULL : 0;
    x <<= 64;
    x %= prime_;

    uint64_t current_fingerprint = fingerprints_[i];
    uint64_t s_bit = current_fingerprint >> 63;
    current_fingerprint &= 0x7FFFFFFFFFFFFFFFULL;

    uint64_t y = (uint64_t) x;

    y = y <= current_fingerprint ?
      current_fingerprint - y : prime_ - (y - current_fingerprint);
    return y + s_bit*static_cast<uint64_t>(prime_);
  }

  /* Retruns the i'th block for i > 0. */
  uint64_t getBlockGuaranteeIgeqOne(const uint64_t i) const {
    unsigned __int128 x = fingerprints_[i - 1] & 0x7FFFFFFFFFFFFFFFULL;
    x <<= 64;
    x %= prime_;

    uint64_t current_fingerprint = fingerprints_[i];
    uint64_t s_bit = current_fingerprint >> 63;
    current_fingerprint &= 0x7FFFFFFFFFFFFFFFULL;

    uint64_t y = (uint64_t) x;

    y = y <= current_fingerprint ?
      current_fingerprint - y : prime_ - (y - current_fingerprint);
    return y + s_bit*static_cast<uint64_t>(prime_);
  }

  /* Calculates the fingerprint of T[from, from + 2^exp) when the fingerprint
     of T[1, from) is already knwon */
  uint64_t fingerprintExp(unsigned __int128 fingerprint_to_i,
                          const uint64_t from, const int exp) const {
    unsigned __int128 fingerprint_to_j = fingerprintTo(from + (1 << exp) - 1);
    fingerprint_to_i *= power_table_[exp];
    fingerprint_to_i %= prime_;

    return fingerprint_to_j >= fingerprint_to_i ?
      static_cast<uint64_t>(fingerprint_to_j - fingerprint_to_i) :
      static_cast<uint64_t>(prime_ - (fingerprint_to_i - fingerprint_to_j));
  }

  /* Calculates the fingerprint of T[from, from + 2^exp) */
  uint64_t fingerprintExp(const uint64_t from, const int exp) const {
    unsigned __int128 fingerprint_to_i = (from != 0) ? fingerprintTo(from - 1) : 0;
    unsigned __int128 fingerprint_to_j = fingerprintTo(from + (1 << exp) - 1);
    fingerprint_to_i *= power_table_[exp];
    fingerprint_to_i %= prime_;

    return fingerprint_to_j >= fingerprint_to_i ?
      static_cast<uint64_t>(fingerprint_to_j - fingerprint_to_i) :
      static_cast<uint64_t>(prime_ - (fingerprint_to_i - fingerprint_to_j));
  }

  /* Calculates the fingerprint of T[0..i] */
  uint64_t fingerprintTo(const uint64_t i) const {
    unsigned __int128 fingerprint = 0;
    int pad = ((i+1) & 7); 
    if(TLX_UNLIKELY(pad == 0)) {
      // This fingerprints is already saved.
      // We only have to remove the helping bit.
      return fingerprints_[i/8] & 0x7FFFFFFFFFFFFFFFULL; 
    }
    pad *= 8;
    /* Add fingerprint from previous block */
    if (TLX_LIKELY(i > 7)) {
      fingerprint = fingerprints_[((i/8) - 1)] & 0x7FFFFFFFFFFFFFFFULL;
      fingerprint <<= pad;
      {
        unsigned __int128 x = fingerprints_[(i / 8) - 1] & 0x7FFFFFFFFFFFFFFFULL;
        x <<= 64;
        x %= prime_;

        uint64_t current_fingerprint = fingerprints_[i / 8];
        uint64_t s_bit = current_fingerprint >> 63;
        current_fingerprint &= 0x7FFFFFFFFFFFFFFFULL;

        uint64_t y = (uint64_t) x;

        y = y <= current_fingerprint ? 
          current_fingerprint - y : prime_ - (y - current_fingerprint);
        fingerprint += ((y + s_bit*static_cast<uint64_t>(prime_)) >> (64 - pad));
      }
    } else {
      uint64_t current_fingerprint = fingerprints_[0];
      uint64_t s_bit = current_fingerprint >> 63;
      current_fingerprint &= 0x7FFFFFFFFFFFFFFFULL;
  
      uint64_t y = (uint64_t) 0;

      y = y <= current_fingerprint ?
        current_fingerprint - y : prime_ - (y - current_fingerprint);
      fingerprint += ((y + s_bit*static_cast<uint64_t>(prime_)) >> (64 - pad));
    }

    /* Add relevant part of block */
    //fingerprint += (getBlock(i/8) >> (64-pad));
    fingerprint %= prime_;
    return static_cast<uint64_t>(fingerprint);
  }

  /* Overwrites the n'th block with the fingerprint of the first n blocks.
     Because the Rabin-Karp fingerprint uses a rolling hash function,
     this is done in O(n) time. */
  void calculateFingerprints() {
    /* We run into problems with small endian systems, if we do not reverse the
       order of the characters. I could not find a way to calculate fingerprints
       without this "endian reversal". Luckily this step is not that slow. */
    char * input = (char*) fingerprints_;
    for(uint64_t i = 0; i < text_length_in_blocks_; i++) {
      uint64_t paquet = *(uint64_t*)"\x1\x0\x0\x0\x0\x0\x0\x0";
      if(paquet == 1){
        //reverse
        char *f=&input[0], *b=&input[7];
        while(f<b){
          char tmp = *f;
          *f++ = *b;
          *b-- = tmp;
        }
      }
      input += 8;
    }
    uint64_t previous_fingerprint = 0;
    uint64_t current_block = fingerprints_[0];

    for (uint64_t i = 0; i < text_length_in_blocks_; ++i) {
      current_block = fingerprints_[i];
      unsigned __int128 x = previous_fingerprint;
      x = x << 64;
      x = x + current_block;
      x = x % prime_;
      previous_fingerprint = (uint64_t) x;

      /* Additionally store if block > prime */
      if(current_block > prime_) {
        x = x + 0x8000000000000000ULL;
      }
      fingerprints_[i] = (uint64_t) x;
    }
  }

  /* Calculates the powers of 2. This supports LCE queries and reduces the time
     from polylogarithmic to logarithmic. */
  void calculatePowers() {
    // +1 to round up and +4 because we need bit shifts by 1byte, 2byte, 4byte,
    // and 8byte
    unsigned int number_of_levels = ((int) std::log2(text_length_in_blocks_)) + 6;
    //power_table_ = new uint64_t[number_of_levels];
    unsigned __int128 x = 256;
    power_table_[0] = (uint64_t) x;
    for (unsigned int i = 1; i < number_of_levels; ++i) {
      x = (x*x) % prime_;
      power_table_[i] = (uint64_t) x;
    }
  }
};

/******************************************************************************/
