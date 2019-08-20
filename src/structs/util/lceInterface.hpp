#ifndef LCEINTERFACE_INCLUDED
#define LCEINTERFACE_INCLUDED

#include <inttypes.h>
#include <string>
#include <fstream>

class LceDataStructure {
	public:
		virtual uint64_t lce(const uint64_t i, const uint64_t j) = 0;
		//virtual char getChar(const uint64_t i) = 0;
		virtual char operator[](const uint64_t i) = 0;
		virtual int isSmallerSuffix(const uint64_t i, const uint64_t j) = 0;
		virtual uint64_t getSizeInBytes() = 0;
};

#endif
