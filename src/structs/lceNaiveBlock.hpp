#include "lceInterface.hpp"
#include "util.hpp"
#include <cstdio>

#define unlikely(x)    __builtin_expect(!!(x), 0) 

/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */

class LceNaiveBlock : public LceDataStructure {
	
	
	public:
		LceNaiveBlock() = delete;
		/* Loads the full file located at PATH. */
		LceNaiveBlock(const std::string path) : 
				textLengthInBytes{util::calculateSizeOfInputFile(path)},
				text{new char[util::calculateSizeOfInputFile(path)]} {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			input.seekg(0);
			input.read(text, textLengthInBytes);
		}
		
		
		/* Loads a prefix of the file located at PATH */ 
		
		LceNaiveBlock(const std::string path, const uint64_t numberOfChars) : 
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
		
		~LceNaiveBlock() {
			delete[] text;
		}
		
		
		/* Naive LCE-query */
		uint64_t lce (const uint64_t i, const uint64_t j) {
			uint64_t lce = 0;
			if (unlikely(i == j)) {
				return textLengthInBytes - i;
			}
			
			uint64_t * textBlocks1 = (uint64_t*) (text + i);
			uint64_t * textBlocks2 = (uint64_t*) (text + j);
			
			const uint64_t maxLength = textLengthInBytes - ((i < j) ? j : i);
			
			while (textBlocks1[lce] == textBlocks2[lce]) {
				++lce;
				if (unlikely(lce >= maxLength)) {
					break;
				}
			}
			
			lce *= 8;
			
			while (text[i + lce] == text[j + lce]) {
				++lce;
				if (unlikely(lce >= maxLength)) {
					break;
				}
			}
			return lce;
		}
		
		char getChar(const uint64_t i) {
			return text[i];
		}
		
	private: 
		char * const text;
		const uint64_t textLengthInBytes;
};
