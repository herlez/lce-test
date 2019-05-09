#include "lceInterface.hpp"
#include "util.hpp"
#include <cstdio>

#define unlikely(x)    __builtin_expect(!!(x), 0) 

/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */

class LceNaiveBlock128 : public LceDataStructure {
	
	
	public:
		LceNaiveBlock128() = delete;
		/* Loads the full file located at PATH. */
		LceNaiveBlock128(const std::string path) : 
				textLengthInBytes{util::calculateSizeOfInputFile(path)},
				text{new char[util::calculateSizeOfInputFile(path)]} {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			input.seekg(0);
			input.read(text, textLengthInBytes);
		}
		
		
		/* Loads a prefix of the file located at PATH */ 
		
		LceNaiveBlock128(const std::string path, const uint64_t numberOfChars) : 
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
		
		~LceNaiveBlock128() {
			delete[] text;
		}
		
		
		/* Naive LCE-query */
		uint64_t lce (const uint64_t i, const uint64_t j) {
			uint64_t lce = 0;
			if (unlikely(i == j)) {
				return textLengthInBytes - i;
			}
			
			unsigned __int128 * textBlocks1 = (unsigned __int128*) (text + i);
			unsigned __int128 * textBlocks2 = (unsigned __int128*) (text + j);
			
			const uint64_t maxLength = textLengthInBytes - ((i < j) ? j : i);
			
			while (textBlocks1[lce] == textBlocks2[lce]) {
				++lce;
				if (unlikely(lce >= maxLength)) {
					break;
				}
			}
			
			lce *= 16;
			
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
