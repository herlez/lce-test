#include "util/lceInterface.hpp"
#include "util/util.hpp"
#include "util/bit_vector_rank.hpp"

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
		LceSyncSets(std::string path, uint64_t numberOfChars) {
		}
		
		~LceSyncSets() {
		}
		
		/* Answers the lce query for position i and j */
		inline uint64_t lce(const uint64_t i, const uint64_t j) {
			if (i==j) {
				return tSize - i;
			}
			/* naive part */
			for(unsigned int k = 0; k < (3*tau)-1; ++k) {
				if(t[i+k] != t[j+k]) {
					return k;
				}
			}

			/* strSync part */
			//std::vector<bool>::iterator si = suc(i);
			uint64_t si = suc(i);
			//std::cout << i << std::endl;
			//std::cout << *si << std::endl;
			//std::vector<bool>::iterator sj = suc(j);
			uint64_t sj = suc(j);
			//std::cout << j << std::endl;
			//std::cout << *sj << std::endl;
			
			//std::cout << tFP[i] << std::endl;
			//std::cout << tFP[j] << std::endl;
			//std::cout << t_[i] << std::endl;
			//std::cout << t_[j] << std::endl;
			
			uint64_t l = i - lceT(si, sj);
			return l + lce(i + l, j + l);
		}
		
		char operator[](uint64_t i) {
			if(i > tSize) {return '\00';}
			return t[i];
		}
		
		int isSmallerSuffix(const uint64_t i, const uint64_t j) {
			return 0;
		}
		
		size_t getSizeInBytes() {
			return tSize;
		}
		
		
	private:
		const uint64_t tau = 512;
		
		std::string t;
		size_t tSize;
		
		std::vector<bool> q;
		std::vector<bool> r;
		
		std::vector<uint64_t> s;
		std::vector<bool> sBit;
		
		std::vector<uint64_t> tFP;
		const unsigned __int128 prime = 18446744073709551557ULL;
		uint64_t powOf2modPrime;
		
		std::vector<uint64_t> t_;
		
		std::vector<uint64_t> ISA, LCP;
		std::vector<std::vector<uint64_t> RMQ;
		
		
		//naive lce in T
		uint64_t lceT(uint64_t i, uint64_t j) {
			const uint64_t startI = i;
			while(i < t.size() && j < s.size() && t[i] == t[j]) {
				i++;
				j++;
			}
			return (i - startI)/(3*tau);
		}
		
		//lce with lce data structure in T'
		uint64_t lceT_(uint64_t i, uint64_t j) {
			return LCP[rmq(ISA[i] + 1, ISA[j])];
		}
		
		uint64_t rmq(uint64_t i, uint64_t j) {
			if(i > j) {
				uint64_t tempi = i;
				i = j;
				j = tempi
			}
			
			
		}
		
		/* Return the identifier of the text t[i..i+tau] */
		inline uint64_t id(uint64_t i) {
			return tFP[i];
		}
		
		// Returns if the period of T[from..from+tau-1] is greater than tau/3
		inline bool per1tau(const uint64_t from) const{
			unsigned int iToCompare = 0;
			unsigned int period = 1;
			uint64_t i;
			for(i = 1; i < tau/3; ++i) {
				if(t[from + iToCompare] == t[from + i]) {
					++iToCompare;
				} else {
					if(iToCompare != 0) {
						--i;
					}
					iToCompare = 0;
					period = i + 1;
				}
			}
			
			for( ; i < tau; ++i) {
				if(t[from + i] != t[from + i - period]) {
					return false;
				}
			}
			return true;
		}
		

		/* Finds the smallest element that is greater or equal to i
		Because s is ordered, that is equal to the 
		first element greater than i */
		inline uint64_t suc(uint64_t i) {
			while(sBit[i] == 0 && i < s.size()) {
				++i;
			}
			return i;
			//std::cerr << "ERROR: suc i=" << i << " not found" << '\n';
		}
		
		
		void fillS(uint64_t from, uint64_t to) {
			for (uint64_t i = from; i < to; ++i) {
				if(i%1000000 == 0) std::cout << i << '\n';
				unsigned int min = tau+1;
				
				// We first check if i and i+tau are in q. If so, i is not element of s.
				if(q[i] == 0) {
					min = 0;
				}
				if(q[i+tau] == 0) {
					if(id(i) > id(i+tau)) {
						min = tau;
				}
				if(min == tau+1)
					continue;
				}

				// Compare this id with every other index which is not in q
				for (unsigned int j = 1; j < tau; ++j) {
					if(q[i+j]==0 && id(i+j) < id(i+min)) {
						min = j;
					}
				}
				if(min == 0 || min == tau) {
					sBit[i] = true;
					s.push_back(i);
				}
			}
		}
		
		
		void buildStruct(std::string path) {
			std::ifstream input(path);
			input.seekg(0);
			util::inputErrorHandling(&input);
			std::stringstream buffer;
			buffer << input.rdbuf();
			t = buffer.str();
			tSize = t.size();
			std::cout << "T size: " << t.size() << std::endl;
			
			// Calculate fingerprints
			std::cout << "Calculating FP" << std::endl;
			unsigned __int128 fp = 0;
			for(uint64_t i = 0; i < tau; ++i) {
				fp *= 256;
				fp += (unsigned char) t[i];
				fp %= prime;
			}
			tFP.push_back((uint64_t) fp);
			
			powOf2modPrime = calculatePowerModulo(9);
			for(uint64_t i = 0; i < tSize-tau; ++i) {
				fp *= 256;
				fp += (unsigned char) t[tau+i];
				fp %= prime;
				
				unsigned __int128 firstCharInfluence = t[i];
				firstCharInfluence *= powOf2modPrime;
				firstCharInfluence %= prime;
				
				if(firstCharInfluence < fp) {
					fp -= firstCharInfluence;
				} else {
					fp = prime - (firstCharInfluence - fp);
				}
				tFP.push_back((uint64_t) fp);
			}
			
			std::cout << "FP calculated" << std::endl;
			std::cout << "FP Size: " << tFP.size() << std::endl;
			
			
			
			// Fill Q
			/*
			q = std::vector<bool>(tSize, false);
			for(uint64_t i = 0; i < (tSize - tau + 1); ++i) {
				if(per1tau(i)) {  //if the period is smaller than tau/3, i is element of Q
					q[i] = 1;
				}
			}
			std::cout << "Q size: " << std::count(q.begin(), q.end(), true) << std::endl;
			*/
			
			
			
			sBit.resize(tSize);
			// Calculate S
			//fillS(0, (tSize - (2*tau + 1)));
			

			// LOAD S FROM A FILE
			
			std::ifstream sLoad("../res/sss_dna.50MB", std::ios::in);
			for (std::string line; std::getline(sLoad, line); ) {
				s.push_back(stoi(line));
				sBit[stoi(line)] = 1;
			}
			
			s.shrink_to_fit();
			//std::cout << "S size: " << s.size() << std::endl;
			// SAVE S IN A FILE
			/*
			std::ofstream sSet("../res/sss_dna.50MB", std::ios::out|std::ios::trunc);
			for(uint64_t i = 0; i < s.size(); ++i) {
				sSet << s[i] << std::endl;
			}
			*/
			
			
			// Construct SA for T_
			std::vector<uint64_t> SA;
			SA.resize(s.size());
			for(uint64_t i = 0; i < s.size(); ++i) {
				SA[i] = i;
			}
			std::sort(SA.begin(), SA.end(), [=](uint64_t i, uint64_t j) {
								while(true) {
									if(i > s.size()) {return true;}
									if(j > s.size()) {return false;}
									for(uint64_t k = 0; k < 3*tau; ++k) {
										if(operator[](i+k) != operator[](j+k)) {
											return operator[](i+k) < operator[](j+k);
										}
									}
									i = suc(i);
									j = suc(j);
								}
								return false;
							});
			std::cout << "SA calculated" << std::endl;
			
			// Constuct ISA for T_
			ISA.resize(s.size());
			for(uint64_t i = 0; i < s.size(); ++i) {
				ISA[SA[i]] = i;
			}
			std::cout << "ISA calculated" << std::endl;
			
			
			// Construct LCP for T_
			LCP.resize(s.size());
			for(uint64_t i = 0; i < (s.size()-1); ++i) {
				LCP[i] = lceT(s[SA[i]], s[SA[i+1]]);
			}
			std::cout << "LCP calculated" << std::endl;
			// Constuct RMQ
			RMQ.resize

		}
		
	uint64_t calculatePowerModulo(unsigned int power) {
			//powerTable = new uint64_t[numberOfLevels];
			unsigned __int128 X = 256;
			for (unsigned int i = 0; i < power; i++) {
				X = (X*X) % prime;
			}
			return (uint64_t) X;
		}
		
};
