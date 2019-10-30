#include "util/lce_interface.hpp"
#include "util/util.hpp"
#include <cstdio>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)    __builtin_expect(!!(x), 0) 

/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */

class LceUltraNaive : public LceDataStructure {
	public:
		LceUltraNaive() = delete;
		/* Loads the full file located at PATH. */
		LceUltraNaive(const std::string path) : 
				text_length_in_bytes_{util::calculateSizeOfInputFile(path)},
				text_{new char[util::calculateSizeOfInputFile(path)]} {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			input.seekg(0);
			input.read(text_, text_length_in_bytes_);
		}
		
		
		/* Loads a prefix of the file located at PATH */
		LceUltraNaive(const std::string path, const uint64_t number_of_chars) : 
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
		
		~LceUltraNaive() {
			delete[] text_;
		}
		
		
		/* Naive LCE-query */
		uint64_t lce(const uint64_t i, const uint64_t j) {
			
			if (unlikely(i == j)) {
				return text_length_in_bytes_ - i;
			}
			
			const uint64_t max_length = text_length_in_bytes_ - ((i < j) ? j : i);
			uint64_t lce = 0;
			while(likely(lce < max_length) && text_[i + lce] == text_[j + lce]) {
				lce++;
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
