#include "lceInterface.hpp"
#include "util.hpp"
#include <vector>
#include <cstdio>

#define unlikely(x)    __builtin_expect(!!(x), 0) 

/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */

class lceSyncSets : public lceDataStructure {
	public:
		/* Loads the full file located at PATH. */
		lceSyncSets(std::string path) {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			textLengthInBytes = util::calculateSizeOfInputFile(&input);
			text = new char[textLengthInBytes];
			input.seekg(0);
			input.read(text, textLengthInBytes);
		}
		
		/* Loads a prefix of the file located at PATH */ 
		lceSyncSets(std::string path, uint64_t numberOfChars) {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			uint64_t dataSize = util::calculateSizeOfInputFile(&input);
			if(numberOfChars > dataSize) {
				std::cerr << "Attemted to load the first " << numberOfChars << " Bytes, but the file " << path << " is only " << dataSize << " Bytes big." << std::endl;
				exit(EXIT_FAILURE);
			}
			textLengthInBytes = numberOfChars;
			text = new char[textLengthInBytes];
			input.seekg(0);
			input.read(text, textLengthInBytes);
		}
		
		
		
		~lceSyncSets() {
			delete[] text;
		}
		
		/* Naive LCE-query */
		uint64_t lce(uint64_t i, uint64_t j) {
			uint64_t lce = 0;
			return lce;
		}
		
		char getChar(uint64_t i) {
			return text[i];
		}
		
	private:
		char * text;
		
		// For up to 4,29 GB of data
		std::vector<uint32_t> stringSynchronizingSet;
		
		int id (int tauBlock) {
			return tauBlock;
		}
};
