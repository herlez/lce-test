#include "util/lce_interface.hpp"
#include "util/util.hpp"
#include <cstdio>

#define unlikely(x)    __builtin_expect(!!(x), 0) 

/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */

class LceNaive : public LceDataStructure {
	public:
		LceNaive() = delete;
		/* Loads the full file located at PATH. */
		LceNaive(const std::string path) : 
				text_length_in_bytes_{util::calculateSizeOfInputFile(path)},
				text_{new char[util::calculateSizeOfInputFile(path)]} {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			input.seekg(0);
			input.read(text_, text_length_in_bytes_);
		}
		
		
		/* Loads a prefix of the file located at PATH */
		LceNaive(const std::string path, const uint64_t number_of_chars) : 
					text_length_in_bytes_{util::calculateSizeOfInputFile(path)}, 
					text_{new char[number_of_chars]} {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			uint64_t data_size = util::calculateSizeOfInputFile(&input);
			if(number_of_chars > data_size) {
				std::cerr << "Attemted to load the first " << number_of_chars << " Bytes, but the file " << path << " is only " << data_size << " Bytes big." << std::endl;
				exit(EXIT_FAILURE);
			}
			input.seekg(0);
			input.read(text_, text_length_in_bytes_);
		}
		
		~LceNaive() {
			delete[] text_;
		}
		
		
		/* Naive LCE-query */
		uint64_t lce(const uint64_t i, const uint64_t j) {
			
			if (unlikely(i == j)) {
				return text_length_in_bytes_ - i;
			}
			
			const uint64_t max_length = text_length_in_bytes_ - ((i < j) ? j : i);
			uint64_t lce = 0;
			// First we compare the first few characters. We do this, because in the usual case the lce is low.
			for(; lce < 8; ++lce) {
				if(unlikely(lce >= max_length)) {
					return max_length;
				}
				if(text_[i + lce] != text_[j + lce]) {
					return lce;
				}
			}
			
			// Accelerate search by comparing 16-byte blocks
			lce = 0;
			const unsigned __int128 * text_blocks_i = (unsigned __int128*) (text_ + i);
			const unsigned __int128 * text_blocks_j = (unsigned __int128*) (text_ + j);
			for(; lce < max_length/16; ++lce) {
				if(text_blocks_i[lce] != text_blocks_j[lce]) {
					break;
				}
			}
			lce *= 16;
			// The last block did not match. Here we compare its single characters
			uint64_t lce_end = lce + ((16 < max_length) ? 16 : max_length);
			for (; lce < lce_end; ++lce) {
				if(text_[i + lce] != text_[j + lce]) {
					break;
				}
			}
			return lce;
		}
		
		inline char operator[](const uint64_t i) {
			return text_[i];
		}
		
		int isSmallerSuffix(const uint64_t i, const uint64_t j) {
			uint64_t lce_s = lce(i, j);
			if(unlikely((i + lce_s + 1 == text_length_in_bytes_) || (j + lce_s + 1 == text_length_in_bytes_))) {
				return true;
			}
			return (operator[](i + lce_s) < operator[](j + lce_s));
		}
		
		uint64_t getSizeInBytes() {
			return text_length_in_bytes_;
		}
		
	private: 
		const uint64_t text_length_in_bytes_;
		char * const text_;
		
};
