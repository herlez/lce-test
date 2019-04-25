#include "lceInterface.hpp"
#include "util.hpp"

/* This class stores a text as an array of characters and 
 * answers LCE-queries with the naive method. */

class lceUltraNaive : public lceDataStructure {
	public:
		/* Loads the full file located at PATH. */
		lceUltraNaive(std::string path) {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			textLengthInBytes = util::calculateSizeOfInputFile(&input);
			text = new char[textLengthInBytes];
			input.seekg(0);
			input.read(text, textLengthInBytes);
		}
		
		/* Loads a prefix of the file located at PATH */ 
		lceUltraNaive(std::string path, uint64_t numberOfChars) {
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
		
		~lceUltraNaive() {
			delete[] text;
		}
		
		/* Naive LCE-query */
		uint64_t lce(uint64_t i, uint64_t j) {
			if (i == j) {
				return textLengthInBytes - i;
			}
			uint64_t lce = 0;
			while(i + lce < textLengthInBytes &&
			j + lce < textLengthInBytes && 
			text[i+lce] == text[j+lce]) {
				lce++;
			}
			return lce;
		}
		
		char getChar(uint64_t i) {
			return text[i];
		}
		
	private:
		char * text;
};
