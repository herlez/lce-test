#include "util/lceInterface.hpp"
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
				textLengthInBytes{util::calculateSizeOfInputFile(path)},
				text{new char[util::calculateSizeOfInputFile(path)]} {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			input.seekg(0);
			input.read(text, textLengthInBytes);
		}
		
		
		/* Loads a prefix of the file located at PATH */
		LceNaive(const std::string path, const uint64_t numberOfChars) : 
					textLengthInBytes{util::calculateSizeOfInputFile(path)}, 
					text{new char[numberOfChars]} {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			uint64_t dataSize = util::calculateSizeOfInputFile(&input);
			if(numberOfChars > dataSize) {
				std::cerr << "Attemted to load the first " << numberOfChars << " Bytes, but the file " << path << " is only " << dataSize << " Bytes big." << std::endl;
				exit(EXIT_FAILURE);
			}
			input.seekg(0);
			input.read(text, textLengthInBytes);
		}
		
		~LceNaive() {
			delete[] text;
		}
		
		
		/* Naive LCE-query */
		uint64_t lce(const uint64_t i, const uint64_t j) {
			uint64_t lce = 0;
			if (unlikely(i == j)) {
				return textLengthInBytes - i;
			}
			
			const uint64_t maxLength = textLengthInBytes - ((i < j) ? j : i);
			
			// First we compare the first few characters. We do this, because in the usual case the lce is low.
			for(; lce < 8; ++lce) {
				if(unlikely(lce >= maxLength)) {
					return maxLength;
				}
				if(text[i + lce] != text[j + lce]) {
					return lce;
				}
			}
			
			// Accelerate search by comparing 16-byte blocks
			lce = 0;
			unsigned __int128 * textBlocks1 = (unsigned __int128*) (text + i);
			unsigned __int128 * textBlocks2 = (unsigned __int128*) (text + j);
			for(; lce < maxLength/16; ++lce) {
				if(textBlocks1[lce] != textBlocks2[lce]) {
					break;
				}
			}
			lce *= 16;
			// The last block did not match. Here we compare its single characters
			uint64_t lceEnd = lce + ((16 < maxLength) ? 16 : maxLength);
			for (; lce < lceEnd; ++lce) {
				if(text[i + lce] != text[j + lce]) {
					break;
				}
			}
			return lce;
		}
		
		inline char operator[](const uint64_t i) {
			return text[i];
		}
		
		int isSmallerSuffix(const uint64_t i, const uint64_t j) {
			uint64_t lceS = lce(i, j);
			if(unlikely((i + lceS + 1 == textLengthInBytes) || (j + lceS + 1 == textLengthInBytes))) {
				return true;
			}
			return (operator[](i + lceS) < operator[](j + lceS));
		}
		
		uint64_t getSizeInBytes() {
			return textLengthInBytes;
		}
		
	private: 
		const uint64_t textLengthInBytes;
		char * const text;
		
};
