#include "util/lceInterface.hpp"
#include "util/util.hpp"
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
			std::vector<uint64_t>::iterator si = suc(i);
			std::vector<uint64_t>::iterator sj = suc(j);
			uint64_t l = lceT(si, sj);
			return (*(si+l) - i) + lce(*(si+l), *(sj+l));
		}
		
		char operator[](uint64_t i) {
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
		const uint64_t prime = 18446744073709551557ULL;
		uint64_t powOf2modPrime;
		
		std::vector<uint64_t> t_;
		
		
		
		uint64_t lceT(std::vector<uint64_t>::iterator i, std::vector<uint64_t>::iterator j) {
			uint64_t lceInTi = 0;
			while(i != t_.end() && j != t_.end()) {
				for(uint64_t k = 0; k < 3*tau-1; ++k) {
					if(t[*i+k] != t[*j+k]) {
						return lceInTi;
					}
				}
				++lceInTi;
				++i;
				++j;
			}
			return lceInTi;
		}
		
		/* Return the identifier of the text t[i..i+tau] */
		inline std::string id(uint64_t i) {
			return t.substr(i, tau);
		}
		
		/* Calculates the period of the t[from..from + 3*tau-1] */
		inline uint64_t per3tau(const uint64_t from) const{
			if(from > tSize-(3*tau)+1) {
				std::cerr << "ERROR: period3 from " << from << " should not get calculated" << '\n';
			}
			unsigned int iToCompare = 0;
			unsigned int period = 1;
			for(unsigned int i = 1; i < 3*tau - 2; ++i) {
				if (t[from + iToCompare] == t[from + i]) {
					++iToCompare;
				} else {
					iToCompare = 0;
					period = i + 1;
				}
			}
			return period;
		}

		/* Calculates the period of the t[from..from + tau-1] */
		/*
		inline uint64_t per1tau(const uint64_t from) const{
			unsigned int iToCompare = 0;
			unsigned int period = 1;
			for(uint64_t i = 1; i < tau; ++i) {
				if (t[from + iToCompare] == t[from + i]) {
					++iToCompare;
				} else {
					if(iToCompare != 0) {
						--i;
					}
					iToCompare = 0;
					period = i + 1;
				}
			}
			return period;
		}
		*/
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
		inline std::vector<uint64_t>::iterator suc(const uint64_t i) {
			for(auto it = s.begin(); it != s.end(); ++it) {
				if (*it > i) {
					return it;
				}
			}
			return s.end();
			//std::cerr << "ERROR: suc i=" << i << " not found" << '\n';
		}
		
		void fillS(uint64_t from, uint64_t to, std::vector<uint64_t> *v) {
			for (uint64_t i = from; i < to; ++i) {
				if(i%1000000 == 0) std::cout << i << '\n';
				int min = -1;
				
				/* We first check if i and i+tau are in q. If so, i is not element of s.*/
				if(q[i] == 0) {
					min = 0;
				}
				if(q[i+tau] == 0) {
					if(id(i).compare(id(i+tau)) > 0) {
						min = tau;
				}
				if(min == -1)
					continue;
				}

				/* Compare this id with every other index which is not in q */
				for (unsigned int j = 1; j < tau; ++j) {
					if(q[i+j]==0 && id(i+j).compare(id(i+min)) < 0) {
						min = j;
					}
				}
				if(min == 0 || min == tau) {
					v->push_back(i);
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
			
			
			/* Fill Q */
			q = std::vector<bool>(tSize, false);
			for(uint64_t i = 0; i < (tSize - tau + 1); ++i) {
				if(per1tau(i)) {  //if the period is smaller than tau/3, i is element of Q
					q[i] = 1;
				}
			}
			/* Print Q */
			std::cout << "Q size: " << std::count(q.cbegin(), q.cend(), 1) << std::endl;
			unsigned int numberOfI = 0;
			for(unsigned int i = 0; (numberOfI < 5) && (numberOfI < q.size()); ++i) {
				if(q[i]) {
					std::cout << i << std::endl;
					++numberOfI;
				}
			}
			
			/* Calculate R based on Observation 8.1 */
			r = std::vector<bool>(tSize, false);
			uint64_t nElementsInQ = 0; // Counts the number of subsequent indexes in q
			for(uint64_t i = 0; i < (tSize - 3*tau + 2); ++i) {
				//if(i%10000 == 0) {std::cout << i << '\n';}
				if(q[i] == 1) {
					++nElementsInQ;
				} else {
					nElementsInQ = 0;
				}
				
				if(nElementsInQ >= 2*tau) {
					r[1+i-2*tau] = 1;
				}
				
			}
			/* Print R */
			std::cout << "R size: " << std::count(r.cbegin(), r.cend(), 1) << std::endl;
			numberOfI = 0;
			for(unsigned int i = 0; (numberOfI < 5) && (numberOfI < r.size()); ++i) {
				if(r[i]) {
					std::cout << i << std::endl;
					++numberOfI;
				}
			}

			/* Print R */
			//std::cout << "RO size: " << r_or.size() << std::endl;
			
			/* Fill S */
			/*
			std::array<std::thread, 8> sThread;
			std::array<std::vector<uint64_t>, 8> si;
			
			for(unsigned int i = 0; i < 8; ++i) {
				std::thread s1 (&LceSyncSets::fillS, this, (i*8)*(tSize-(2*tau + 1)), ((i+1)*8)*(tSize-(2*tau + 1))-1, s);
			}
			*/
			
			// LOAD S FROM A FILE
			
			std::ifstream sLoad("../res/sss_dna.50MB", std::ios::in);
			for (std::string line; std::getline(sLoad, line); ) {
				s.push_back(stoi(line));
			}
			
			
				
			//fillS(0, (tSize - (2*tau + 1)), &s);
			
			s.shrink_to_fit();
			/* Print S */
			std::cout << "S size: " << s.size() << std::endl;
			/*for( auto i : s ) {
				std::cout << i << std::endl;
				std::cout << id(t, i, tau*2) << std::endl;
			}
			*/
			
			// SAVE S IN A FILE
			/*
			std::ofstream sSet("../res/sss_dna.50MB", std::ios::out|std::ios::trunc);
			for(uint64_t i = 0; i < s.size(); ++i) {
				sSet << s[i] << std::endl;
			}
			*/
			
			/* Calculate fingerprints of the 3*tau long substrings and save fingerprints for every element in s */
			std::cout << "Calculate fingerprints" << std::endl;
			std::vector<uint64_t> fps(s.size());
			unsigned __int128 fp = 0; //current fingerprint
			uint64_t indexS = 0; //index of next element in s
			uint64_t nextS = s[indexS]; //next element in s
			uint64_t i;
			for(i = 0; i < 3*tau; ++i) {
				fp *= 256;
				fp += (unsigned char) t[i];
				fp %= prime;
				if(i == nextS) {
					fps.push_back((uint64_t) fp);
					++indexS;
					nextS = s[indexS];
				}
			}
			
			
			powOf2modPrime = calculatePowerModulo(9);
			
			for( ; i < s[s.size() - 1]; ++i) {
				unsigned __int128 firstCharInfluence = (unsigned int) s[i-1];
				firstCharInfluence *= powOf2modPrime;
				
				if ( (uint64_t) firstCharInfluence < fp ) {
					fp -= firstCharInfluence;
				} else {
					fp = prime - (firstCharInfluence - fp);
				}
				
				
				fp *= 256;
				fp += (unsigned char) t[i];
				fp %= prime;
				if(i == nextS) {
					fps.push_back((uint64_t) i);
					++indexS;
					nextS = s[indexS];
				}
			}
			
			
			std::cout << "Fingerprints calculated" << std::endl;
			

			/* Calculate T' */
			t_ = s;	
			std::sort(t_.begin(), t_.end(), [=](uint64_t a, uint64_t b) {
										uint64_t max = std::min(std::min(tSize - a, tSize - b), 3*tau);
										
										uint64_t i;
										for(i = 0; i < max; ++i) {
											if(t.at(a+i) != t.at(b+i)) {
												return t.at(a+i) < t.at(b+i);
											}
										}
										
										if(i < 3*tau) {
											if(a > b) {
												return false;
											}
											uint64_t max_2 = std::min(tSize - b, 3*tau);
											for(; i < max_2; ++i) {
												if(t.at(b+i) != 0) {
													return true;
												}
											}
										}
										return false;
									});

			std::cout << "T_ Size: " << t_.size() << std::endl;
		}
		
	uint64_t calculatePowerModulo(unsigned int power) {
			//powerTable = new uint64_t[numberOfLevels];
			unsigned __int128 X = 256;
			for (unsigned int i = 1; i < power; i++) {
				X = (X*X) % prime;
			}
			return (uint64_t) X;
		}
		
};
