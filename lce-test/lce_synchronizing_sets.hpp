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

class LceSyncSets : public LceDataStructure {
	public:
		/* Loads the full file located at PATH. */
		LceSyncSets(std::string path) {
			buildStruct(path);
		}
		
		/* Loads a prefix of the file located at PATH */ 
		LceSyncSets(std::string path, uint64_t number_of_chars) {
		}
		
		~LceSyncSets() {
		}
		
		/* Answers the lce query for position i and j */
		inline uint64_t lce(const uint64_t i, const uint64_t j) {
			if (i==j) {
				return text_length_in_bytes_ - i;
			}
			/* naive part */
			for(unsigned int k = 0; k < (3*kTau)-1; ++k) {
				if(text_[i+k] != text_[j+k]) {
					return k;
				}
			}

			/* strSync part */
			//std::vector<bool>::iterator si = suc(i);
			uint64_t i_ = suc(i);
			uint64_t j_ = suc(j);
			//std::cout << "i: " << i << "  j: " << j << std::endl;
			//std::cout << "i_: " << i_ << "  j_: " << j_ << std::endl;
			//std::cout << "s_[i_]: " << s_[i_] << "  s_[j]: " << s_[j_] << std::endl;
			uint64_t l = lce_rmq_->lce(i_, j_);
			//std::cout << "l: " << l << std::endl;
			//return s_[i_+l] - i + lce(s_[i_+l], s_[j_+l]);
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
		const uint64_t kTau = 512;
		
		std::string text_;
		size_t text_length_in_bytes_;
		
		std::vector<bool> q_;
		std::vector<bool> r_;
		
		std::vector<uint64_t> s_;
		bit_vector * s_bv_;
		bit_vector_rank * s_bvr_;
		
		Lce_rmq * lce_rmq_;
		
		std::vector<uint64_t> t_fp_;
		const unsigned __int128 kPrime = 18446744073709551557ULL;
		uint64_t two_pow_tau_mod_q_;
		
		std::vector<uint64_t> t_;
		
		

		/* Return the identifier of the text t[i..i+tau] */
		inline uint64_t id(uint64_t i) const{
			return t_fp_[i];
		}
		
		// Returns if the period of T[from..from+tau-1] is greater than tau/3
		inline bool per1tau(const uint64_t from) const{
			unsigned int i_to_compare = 0;
			unsigned int period = 1;
			uint64_t i;
			for(i = 1; i < kTau/3; ++i) {
				if(text_[from + i_to_compare] == text_[from + i]) {
					++i_to_compare;
				} else {
					if(i_to_compare != 0) {
						--i;
					}
					i_to_compare = 0;
					period = i + 1;
				}
			}
			
			for( ; i < kTau; ++i) {
				if(text_[from + i] != text_[from + i - period]) {
					return false;
				}
			}
			return true;
		}
		

		/* Finds the smallest element that is greater or equal to i
		Because s_ is ordered, that is equal to the 
		first element greater than i */
		inline uint64_t suc(uint64_t i) const {
			return s_bvr_->rank1(i);
		}
		
		
		void fillS(const uint64_t from, const uint64_t to) {
			for (uint64_t i = from; i < to; ++i) {
				//if(i%10 == 0) std::cout << i << '\n';
				unsigned int min;
				
				// We first check if i and i+tau are in q. If so, i is not element of s_.
				if(q_[i] == 0) {
					min = 0;
					if(q_[i+kTau] == 0 && id(i) >= id(i+kTau)) {
						min = kTau;
					}
				} else if(q_[i+kTau] == 0) {
					min = kTau;
				} else {
					for (unsigned int j = 1; j < kTau; ++j) {
						if(q_[i+j]==0) {
							min = j;
						}
					}
				}


				// Compare this id with every other index which is not in q
				for (unsigned int j = 1; j < kTau; ++j) {
					if(q_[i+j]==0 && (id(i+j) < id(i+min))) {
						min = j;
					}
				}
				if(min == 0 || min == kTau) {
					//std::cout << i+min << std::endl;
					s_.push_back(i);
				}
				
				
				uint64_t local_min = i + min;
				while(i < local_min && i < to) {
					++i;
					if(q_[i+kTau] == 0 && (id(i+kTau) <= id(local_min))) {
						//std::cout << i+min << std::endl;
						s_.push_back(i);
						local_min = kTau;
					}
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
			t_fp_.push_back((uint64_t) fp);
			
			two_pow_tau_mod_q_ = calculatePowerModulo(9);
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
				t_fp_.push_back((uint64_t) fp);
			}
			
			std::cout << "FP calculated" << std::endl;
			std::cout << "FP Size: " << t_fp_.size() << std::endl;
			
			
			
			// Fill Q
			
			q_ = std::vector<bool>(text_length_in_bytes_, false);
			for(uint64_t i = 0; i < (text_length_in_bytes_ - kTau + 1); ++i) {
				if(per1tau(i)) {  //if the period is smaller than kTau/3, i is element of Q
					q_[i] = 1;
				}
			}
			std::cout << "Q size: " << std::count(q_.begin(), q_.end(), true) << std::endl;
			
			
			
			
			//Calculate S
			fillS(0, (text_length_in_bytes_ - (2*kTau)));
			
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
