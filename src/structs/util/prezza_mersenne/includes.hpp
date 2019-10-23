/*
 *  This file is part of rk-lce.
 *  Copyright (c) by
 *  Nicola Prezza <nicolapr@gmail.com>
 *
 *   rk-lce is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   rk-lce is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details (<http://www.gnu.org/licenses/>).
 */

#ifndef INCLUDES_HPP_
#define INCLUDES_HPP_

#include <algorithm>
#include <functional>
#include "stdint.h"
#include <string>
#include <iostream>
#include <set>
#include <vector>
#include <fstream>
#include <sstream>
#include <cassert>
#include <math.h>
#include <chrono>
#include <cstdint>

using namespace std;

namespace rklce{

typedef unsigned char uchar;

typedef __uint128_t uint128;

//x-y mod 2^w-1
template<uint64_t w>
inline uint128 sub(uint128 x, uint128 y){

	constexpr uint128 q = (uint128(1)<<w)-1;

	assert(x<q);
	assert(y<q);

	return x==y ? 0 : ( y<x ? x-y : q - (y-x) );

}

/*
 * returns x * 2^e mod 2^w-1
 *
 * complexity: O(1) (since modulus is a Mersenne prime, this corresponds to a
 * left cyclic permutation of e mod w positions)
 *
 */
template<uint64_t w>
inline uint128 mul_pow2(uint128 x, uint64_t e){

	static constexpr uint128 q = (uint128(1)<<w)-1;

	assert( x <= (uint128(1)<<w)-1 );

	if(e==0) return x;

	e = e % w;

	uint64_t l_bits = e;		//how many bits exit from left and enter from right
	uint64_t r_bits = w-l_bits;

	uint128 MASK = (uint128(1) << r_bits)-1;

	uint128 R = x&MASK;	//right part

	x = x >> r_bits;
	R = R << l_bits;

	auto result = x | R;

	return result%q;

}


/*
 * returns x / 2^e mod 2^w-1
 *
 * complexity: O(1) (since modulus is a Mersenne prime, this corresponds to a
 * right cyclic permutation of e mod w positions)
 *
 */
template<uint64_t w>
inline uint128 div_pow2(uint128 x, uint64_t e){

	static constexpr uint128 q = (uint128(1)<<w)-1;

	assert( x < (uint128(1)<<w)-1 );

	if(e==0) return x;

	e = e % w;

	uint64_t r_bits = e;		//how many bits exit from right and enter from left
	uint64_t l_bits = w-r_bits;

	uint128 MASK = (uint128(1) << r_bits)-1;

	uint128 R = x&MASK;	//right part

	x = x >> r_bits;
	R = R << l_bits;

	auto result = x | R;

	return result%q;

}

inline int clz_u128 (uint128 u) {
  uint64_t hi = u>>64;
  uint64_t lo = u;
  int retval[3]={
    __builtin_clzll(hi),
    __builtin_clzll(lo)+64,
    128
  };
  int idx = !hi + ((!lo)&(!hi));
  return retval[idx];
}


/*
 * simulates a vector of 127-bits integers
 */
class packed_vector_127{

public:

	packed_vector_127(){}

	/*
	 * build packed vector with the content of the input vector
	 *
	 * input: a vector of 128-bits integers, each storing a 127-bits value aligned on the right
	 *
	 * this class compacts the 127-bits values in a virtual array of 127-bits integers (simulated using
	 * 128-bits integers)
	 *
	 */
	packed_vector_127(vector<uint128> & B){

		n_bits = B.size() * BL;
		n = B.size();

		//number of 128-bits integers
		uint64_t n_bl = (n_bits / W) + (n_bits % W != 0);

		//blocks are left-aligned
		blocks = vector<uint128>(n_bl,0);

		uint64_t i = 0;
		for(auto block:B){

			//the vector must contain 127-bits integers
			assert( block < uint128(1)<<BL );

			for(uint64_t j = 0; j < BL; ++j){

				uint128 b = (block >> (BL - (j+1))) & uint128(1);

				blocks[i/W] |= (b << (W-(i%W+1)) );
				i++;

			}

		}

	}

	uint128 operator[](uint64_t i){

		assert(i<n);

		//initial position of the 127-bits block in the bitvector
		auto l = i*BL;

		//number of bits falling in the left word
		auto l_bits = W - l%W;
		l_bits = l_bits > BL ? BL : l_bits;

		//number of bits falling in the right word
		auto r_bits = BL - l_bits;

		uint128 l_mask = (uint128(1)<<l_bits)-1;

		uint128 L = l_bits < BL ? (blocks[l/W] & l_mask) << r_bits : (blocks[l/W] >> (l%W==0 ? 1 : 0)) & l_mask;

		uint128 R = r_bits == 0 ? 0 : blocks[l/W+1==n?0:l/W+1] >> (W-r_bits);

		return L|R;

	}

	uint64_t size(){
		return n;
	}
	uint64_t length(){
		return n;
	}

	uint64_t bit_size(){

		return 8*sizeof(this) + blocks.size()*sizeof(uint128)*8;

	}

private:

	//word size: we use 128-bits integers
	static const uint64_t W = 128;

	//block length
	static const uint64_t BL = 127;

	//number of 127-bits integers
	uint64_t n = 0;

	//number of bits
	uint64_t n_bits = 0;

	vector<uint128> blocks;

};

}

#endif /* INCLUDES_HPP_ */
