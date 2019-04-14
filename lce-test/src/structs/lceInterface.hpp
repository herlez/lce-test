#ifndef LCEINTERFACE_INCLUDED
#define LCEINTERFACE_INCLUDED

#include <inttypes.h>
#include <string>
#include <fstream>

class lceDataStructure {
	public:
		uint64_t textLengthInBytes;
		virtual uint64_t lce(uint64_t i, uint64_t j) = 0;
		virtual char getChar(uint64_t i) = 0;
		double getSizeInBytes() {
			return textLengthInBytes;
		}
	  
	private:
		std::string filePath;
};

#endif
