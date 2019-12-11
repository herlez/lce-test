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

/*
 * bitv.hpp
 *
 *  Created on: Jun 27, 2016
 *      Author: Nicola Prezza
 *
 *  sparse bitvector implemented as a vector of integers. Space: b*w bits (w = memory word size = 64, b = number of 1's)
 *
 *  Supports:
 *
 *  - log(b) time access
 *  - log(b) time rank
 *  - log(b) time 0-predecessor (position of last 0)
 *
 */
#ifndef INTERNAL_BITV_HPP_
#define INTERNAL_BITV_HPP_

#include "includes.hpp"

namespace rklce{

class bitv{

public:

	bitv(){}

	bitv(vector<bool> bv){

		n = bv.size();

		uint64_t b = 0;
		for(auto x:bv) b += x;

		ones = vector<uint64_t>(b);

		uint64_t i = 0; //position in bv
		uint64_t j = 0; //position in ones
		for(auto x:bv){

			if(x) ones[j++] = i;
			i++;

		}

	}

	uint64_t size(){return n;}

	/*
	 * argument: position i in the bitvector
	 * returns: bit in position i
	 * only access! the bitvector is static.
	 */
	bool operator[](uint64_t i){

		//assert(i<n);
		return std::binary_search(ones.begin(),ones.end(),i);

	}

	/*
	 * argument: position i in the bitvector, boolean b
	 * returns: number of bits equal to b before position i excluded
	 */
	uint64_t rank(uint64_t i, bool b=true){

		//assert(i<=n);

		uint64_t n1 = std::lower_bound(ones.begin(),ones.end(),i) - ones.begin();

		return b*n1 + (1-b)*(i-n1);

	}

	/*
	 * argument: position i in the bitvector
	 * returns: rightmost position j <= i such that operator[](j) = false
	 *
	 * WARNING: we require that the first bit in the bitvector is a 0
	 *
	 */
	uint64_t predecessor_0(uint64_t i){

		//assert(i<n);

		//return this position if it contains a 0
		if(not operator[](i)) return i;

		//assert(rank(i) < ones.size());

		//otherwise, the position contains a 1
		auto ps = pred_search(&ones, rank(i));

		//idx = first index of the run of consecutive elements in ones
		uint64_t idx = std::upper_bound(ps.begin(), ps.end(), 0) - ps.begin();

		//assert(idx <= rank(i));

		//pos1 = position of the first 1 in the run
		auto pos1 = ones[idx];

		//assert(operator[](pos1));
		//assert(pos1>0);

		return pos1-1;

	}

	uint64_t bit_size(){

		return 8*sizeof(this) + ones.size()*sizeof(uint64_t)*8;

	}

private:

	vector<uint64_t> ones;//positions of 1's
	uint64_t n = 0;//size


	class pred_search{

		   class ps_iterator  : public std::iterator<random_access_iterator_tag,bool >{

			   	friend class pred_search;

				pred_search *_ps = nullptr;
				uint64_t _index = 0;

				ps_iterator(pred_search *v, uint64_t index)
					: _ps(v), _index(index) { }

			public:

				ps_iterator() = default;
				ps_iterator(ps_iterator const&) = default;

				ps_iterator &operator=(ps_iterator const&) = default;

				// Iterator
				bool operator*() const {
					return (*_ps)[_index];
				}

				ps_iterator &operator++() {
					++_index;
					return *this;
				}

				// EqualityComparable
				bool operator==(ps_iterator it) const {
					return _index == it._index;
				}

				// ForwardIterator
				bool operator!=(ps_iterator it) const {
					return _index != it._index;
				}

				ps_iterator operator++(int) {
					ps_iterator it(*this);
					++_index;
					return it;
				}

				// BidirectionalIterator
				ps_iterator &operator--() {
					//assert(_index>0);
					--_index;
					return *this;
				}

				ps_iterator operator--(int) {
					ps_iterator it(*this);
					//assert(_index>0);
					--_index;
					return it;
				}

				// RandomAccessIterator
				ps_iterator &operator+=(uint64_t n) {
					_index += n;
					return *this;
				}

				ps_iterator operator+(uint64_t n) const {
					ps_iterator it(*this);
					it += n;
					return it;
				}

				friend ps_iterator operator+(uint64_t n, ps_iterator it)
				{
					return it + n;
				}

				ps_iterator &operator-=(uint64_t n) {
					_index -= n;
					return *this;
				}

				ps_iterator operator-(uint64_t n) const {
					ps_iterator it(*this);
					it -= n;
					return it;
				}

				friend ps_iterator operator-(uint64_t n, ps_iterator it) {
					return it - n;
				}

				uint64_t operator-(ps_iterator it) {
					return uint64_t(_index) - uint64_t(it._index);
				}

				bool  operator[](uint64_t i) const {
					//assert(i<_ps->size());
					return (*_ps)[_index + i];
				}

				bool operator<(ps_iterator it) const {
					return _index < it._index;
				}

				bool operator<=(ps_iterator it) const {
					return _index <= it._index;
				}

				bool operator>(ps_iterator it) const {
					return _index > it._index;
				}

				bool operator>=(ps_iterator it) const {
					return _index >= it._index;
				}

			};

		public:

			pred_search(vector<uint64_t> * ones, uint64_t i){

				//assert(i<ones->size());

				this->ones = ones;
				n = i+1;
				this->i = i;

			}

			uint64_t size(){return n;}

			bool operator[](uint64_t j){

				//assert(j<n);
				//assert(j<=i);

				//assert(i<ones->size());
				//assert(j<ones->size());

				//assert(i==j or ones->at(i)>ones->at(j));

				auto b = (ones->at(i)-ones->at(j)) == (i-j);

				return b;

			}

			ps_iterator begin() { return ps_iterator(this, 0); }
			ps_iterator end()   { return ps_iterator(this, n); }

		private:

			vector<uint64_t> * ones;

			//size: from the beginning of bv to i
			uint64_t n;

			//position in bv
			uint64_t i;

		};

};

}


#endif /* INTERNAL_BITV_HPP_ */
