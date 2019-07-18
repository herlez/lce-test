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

#define unlikely(x)    __builtin_expect(!!(x), 0) 

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
			uint64_t si = suc(i);//get s_
			uint64_t sj = suc(j);
			
			/*
			for(unsigned int k = 0; k < (3*tau)-1; ++k) {
				if (t[si+k] != t[sj+k]) {
					return k + (si - i);
				}
			}*/
			return (si - i) + lce(si, sj);
			//return 3*tau + (si - i);
		}
		
		char operator[](uint64_t i) {
			return t[i];
		}
		
		int isSmallerSuffix(const uint64_t i, const uint64_t j) {
			return 0;
		}
		
	private:
		const uint64_t tau = 256;
		
		std::string t;
		uint64_t tSize;
		
		std::vector<bool> q;
		//std::unordered_set<uint64_t> q;
		//std::unordered_set<uint64_t> r;
		std::vector<bool> r;
		std::unordered_set<uint64_t> r_or;
		std::vector<uint64_t> s;
		std::vector<uint64_t> t_;
		
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
		inline uint64_t per1tau(const uint64_t from) const{
			/*
			if(from > tSize-tau) {
				std::cerr << "ERROR: period1 from " << from << " should not get calculated" << '\n';
			}
			*/
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

		/* Finds the smallest element that is greater i
		Because s is ordered, that is equal to the 
		first element greater than i */
		inline uint64_t suc(uint64_t i) const{
			for(uint64_t j = 0; j < s.size(); ++j) {
				if(s[j] > i) {
					return s[j];
				}
			}
			return (tSize - 2*tau + 2);
			//std::cerr << "ERROR: suc i=" << i << " not found" << '\n';
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
				if(per1tau(i) <= tau/3) {
					q[i] = 1;
				}
			}
			/* Print Q */
			std::cout << "Q size: " << std::count(q.cbegin(), q.cend(), 1) << std::endl; 
			
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
			
			/* Calculate R based on the original definition of R */
			/*
			for(uint64_t i = 0; i < (tSize - 3*tau + 2); ++i) {
				if(per3tau(i) <= tau/3) {
					r_or.insert(i);
				}
			}
			*/
			/* Print R */
			//std::cout << "RO size: " << r_or.size() << std::endl;
			
			/* Fill S */
			for (uint64_t i = 0; i < (tSize - 2*tau + 1); ++i) {
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
					s.push_back(i);
				}
			}
			s.shrink_to_fit();
			/* Print S */
			std::cout << "S size: " << s.size() << std::endl; 
			/*for( auto i : s ) {
				std::cout << i << std::endl;
				std::cout << id(t, i, tau*2) << std::endl;
			}
			*/

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
		
};
