#include "util/lce_interface.hpp"
#include "util/util.hpp"
#include "util/synchronizing_sets/bit_vector_rank.hpp"
#include "util/synchronizing_sets/lce-rmq.hpp"

#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <unordered_set>
#include <thread>
#include <cmath>
#include <ctgmath>



#define unlikely(x)    __builtin_expect(!!(x), 0) 
#pragma once
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
			if (i==j) {
				return text_length_in_bytes_ - i;
			}
			/* naive part */
			for(unsigned int k = 0; k < (3*kTau-1); ++k) {
				if(text_[i+k] != text_[j+k]) {
					return k;
				}
			}

			/* strSync part */
			uint64_t i_ = suc(i);
			uint64_t j_ = suc(j);
			uint64_t l = lce_rmq_->lce(i_, j_);
			return l + s_[i_] - i;
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
		
		
	private:
		std::string text_;
		size_t text_length_in_bytes_;
		
		const unsigned __int128 kPrime = 18446744073709551557ULL;
		const uint64_t kTau = 1024;
		uint64_t two_pow_tau_mod_q_;

		
		std::vector<uint64_t> s_;
		bit_vector * s_bv_;
		bit_vector_rank * s_bvr_;
		
		Lce_rmq * lce_rmq_;
		

		/* Finds the smallest element that is greater or equal to i
		Because s_ is ordered, that is equal to the 
		first element greater than i */
		inline uint64_t suc(uint64_t i) const {
			return s_bvr_->rank1(i);
		}
		
		
		void fillS(const uint64_t from, const uint64_t to, const vector<uint64_t> fingerprints) {
			
			uint64_t min;
			for (uint64_t i = from; i < to; ) {
				// Compare this id with every other index which is not in q
				min = 0;
				for (unsigned int j = 1; j <= kTau; ++j) {
					if(fingerprints[i+j] < fingerprints[i+min]) {
						min = j;
					}
				}
				if(min == 0 || min == kTau) {
					s_.push_back(i);
				}
				
				
				uint64_t local_min = i + min;
				++i;
				while(i < to && i < local_min) {
					if(fingerprints[i+kTau] <= fingerprints[local_min]) {
						s_.push_back(i);
						local_min = i + kTau; 
					}
					i++;
				}
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
			

			// Calculate fingerprints
			std::cout << "Calculating FP" << std::endl;
			unsigned __int128 fp = 0;
			for(uint64_t i = 0; i < kTau; ++i) {
				fp *= 256;
				fp += (unsigned char) text_[i];
				fp %= kPrime;
			}
			std::vector<uint64_t> t_fp;
			t_fp.push_back((uint64_t) fp);
			two_pow_tau_mod_q_ = calculatePowerModulo(10);
			for(uint64_t i = 0; i < text_length_in_bytes_-kTau; ++i) {
				fp *= 256;
				fp += (unsigned char) text_[kTau+i];
				fp %= kPrime;
				
				unsigned __int128 first_char_influence = text_[i];
				first_char_influence *= two_pow_tau_mod_q_;
				first_char_influence %= kPrime;
				
				if(first_char_influence < fp) {
					fp -= first_char_influence;
				} else {
					fp = kPrime - (first_char_influence - fp);
				}
				t_fp.push_back((uint64_t) fp);
			}
			
			std::cout << "FP calculated" << std::endl;
			std::cout << "FP Size: " << t_fp.size() << std::endl;
		
			//Calculate S
			fillS(0, (text_length_in_bytes_ - (2*kTau)), t_fp);
			
			// LOAD S FROM A FILE
			//std::ifstream sLoad("../res/ecoli.fa", std::ios::in);
			//for (std::string line; std::getline(sLoad, line); ) {
				//s_.push_back(stoi(line));
			//}
			
			std::cout << "S size: " << s_.size() << std::endl;
			s_.shrink_to_fit();
			// SAVE S IN A FILE
			std::ofstream s_set("../res/ecoli.fa", std::ios::out|std::ios::trunc);
			for(uint64_t i = 0; i < s_.size(); ++i) {
				s_set << s_[i] << std::endl;
			}
			
			
			
			s_bv_ = new bit_vector(text_length_in_bytes_);
			for(size_t i = 0; i < s_.size(); ++i) {
				s_bv_->bitset(s_[i], 1);
			}
			s_bvr_ = new bit_vector_rank(*s_bv_);
			lce_rmq_ = new Lce_rmq(&text_, &s_);
			
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
