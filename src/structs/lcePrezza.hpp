#include <math.h>
#include <algorithm> 
#include "lceInterface.hpp"
#include "util.hpp"

#define unlikely(x)    __builtin_expect(!!(x), 0)
/* This class builds Prezza's in-place LCE data structure and
 * answers LCE-queries in O(log(n)). */

class LcePrezza : public LceDataStructure {
	public:
		LcePrezza() = delete;
		/* Loads the full file located at PATH and builds Prezza's LCE data structure */
		LcePrezza(const std::string path) :
									textLengthInBytes{util::calculateSizeOfInputFile(path)},
									textLengthInBlocks{textLengthInBytes/8 + (textLengthInBytes % 8 == 0 ? 0 : 1)},
									prime{util::getLow64BitPrime()},
									fingerprints{new uint64_t[textLengthInBlocks]},
									powerTable{new uint64_t[((int) std::log2(textLengthInBlocks)) + 6]} {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			input.seekg(0);
			input.read((char*)fingerprints, textLengthInBytes);
			calculateFingerprints();
			calculatePowers();
		}
		
		/* Loads a prefix of the file located at PATH and build Prezza's LCE data structure */
		LcePrezza(const std::string path, const uint64_t numberOfChars) :
									textLengthInBytes{numberOfChars},
									textLengthInBlocks{textLengthInBytes/8 + (textLengthInBytes % 8 == 0 ? 0 : 1)},
									prime{util::getLow64BitPrime()},
									fingerprints{new uint64_t[textLengthInBlocks]},
									powerTable{new uint64_t[((int) std::log2(textLengthInBlocks)) + 6]} {

			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			uint64_t dataSize = util::calculateSizeOfInputFile(&input);
			if(numberOfChars > dataSize) {
				std::cerr << "Attemted to load the first " << numberOfChars << " Bytes, but the file " << path << " is only " << dataSize << " Bytes big." << std::endl;
				exit(EXIT_FAILURE);
			}
			input.seekg(0);
			input.read((char*)fingerprints, textLengthInBytes);
			calculateFingerprints();
			calculatePowers();
		}
		
		/* Fast LCE-query in O(log(n)) time */
		uint64_t lce(const uint64_t i, const uint64_t j) {
			if (unlikely(i == j)) {
				return textLengthInBytes - i;
			}
			
			uint64_t maxLength = textLengthInBytes - ((i < j) ? j : i);
			const uint64_t nCheck = maxLength > 1024 ? 1024 : maxLength;

			/* Compare single characters */
			for(int lceN = 0; lceN <= nCheck; lceN++) {
				if (getChar(i + lceN) != getChar2(j + lceN)) {
					return lceN;
				}
			}
			
			/* exponential search */
			int k = 11;
			uint64_t dist = 2048;
			while(fingerprintExp(i, k) == fingerprintExp(j, k)) {
				++k;
				dist *= 2;
				if(unlikely(dist > maxLength)) {
					break;
				}
			}
			if (k == 0) {return 0;}
			if (k == 1) {return 1;}
			
			/* binary search */
			k--;
			dist /= 2;
			uint64_t i2 = i + dist;
			uint64_t j2 = j + dist;
			maxLength = textLengthInBytes - ((i2 < j2) ? j2 : i2);
			
			while(k != 0) {
				k--;
				dist /= 2;
				if (unlikely(dist > maxLength)) {
					continue;
				}
				if(fingerprintExp(i2, k) == fingerprintExp(j2, k)) {
					i2 += dist;
					j2 += dist;
				}
			}
			return i2-i;
		}
		
		/* Returns the prime*/
		unsigned __int128 getPrime() {
			return prime;
		}
		
		/* Returns the character at index i */ 
		char getChar(const uint64_t i) {
			uint64_t blockNumber = i / 8;
			uint64_t offset = 7 - (i % 8);
			
			if (blockNumber != cachedBlockIndex1) {
				cachedBlockIndex1 = blockNumber;
				cachedBlock1 = getBlock(blockNumber);
			}
			return ((char*)&cachedBlock1)[offset];
		}
		
		char getChar2(const uint64_t i) {
			uint64_t blockNumber = i / 8;
			uint64_t offset = 7 - (i % 8);
			
			uint64_t block;
			if (blockNumber != cachedBlockIndex2) {
				cachedBlockIndex2 = blockNumber;
				cachedBlock2 = getBlock(blockNumber);
			}
			return ((char*)&cachedBlock2)[offset];
		}
	
	private:
		
		const uint64_t textLengthInBytes;
		const uint64_t textLengthInBlocks;
		const unsigned __int128 prime;
		uint64_t * const fingerprints;
		uint64_t * const powerTable;
		
		uint64_t cachedBlock1;
		uint64_t cachedBlockIndex1;
		uint64_t cachedBlock2;
		uint64_t cachedBlockIndex2;
		
		/* Returns the i'th block. A block contains 8 character. */
		uint64_t getBlock(const uint64_t i) {
			if (unlikely(i > textLengthInBlocks)) {
				return 0;
			}
			if (unlikely(i == 0)) {
				if(fingerprints[0] >= 0x8000000000000000ULL) {
					return fingerprints[0] - 0x8000000000000000ULL + prime;
				} else {
					return fingerprints[0];
				}
			} else {
				unsigned __int128 X = fingerprints[i-1] & 0x7FFFFFFFFFFFFFFFULL;
				X = X << 64;
				X = X % prime;
				
				uint64_t currentFingerprint = fingerprints[i];
				uint64_t sBit = currentFingerprint >> 63;
				currentFingerprint &= 0x7FFFFFFFFFFFFFFFULL;
				
				uint64_t Y = (uint64_t) X;
				
				Y = Y <= currentFingerprint ? currentFingerprint - Y : prime - (Y - currentFingerprint);
				
				return Y + sBit*(uint64_t)prime;
			}
		}
		
		/* Overwrites the n'th block with the fingerprint of the first n blocks. Because the Rabin-Karp fingerprint uses a rolling hash function, this is done in O(n) time. */
		void calculateFingerprints() {
			/* We run into problems with small endian systems, if we do not reverse the order of the characters. 
			 * I could not find a way to calculate fingerprints  without this "endian reversal".
			 * Luckily this step is not that slow. */
			char * input = (char*) fingerprints;
			for(uint64_t i = 0; i < textLengthInBlocks; i++) {
				uint64_t paquet = *(uint64_t*)"\x1\x0\x0\x0\x0\x0\x0\x0";
				if(paquet == 1){
					//reverse
					char *f=&input[0], *b=&input[7];
					while(f<b){
						char tmp = *f;
						*f++ = *b;
						*b-- = tmp;
					}
				}
				input += 8;
			}
			uint64_t previousFingerprint = 0;
			uint64_t currentBlock = fingerprints[0];
			
			for (uint64_t i = 0; i < textLengthInBlocks; i++) {
				currentBlock = fingerprints[i];
				unsigned __int128 X = previousFingerprint;
				X = X << 64;
				X = X + currentBlock;
				X = X % prime;
				previousFingerprint = (uint64_t) X;
				
				/* Additionally store if block > prime */
				if(currentBlock > prime) {
					X = X + 0x8000000000000000ULL;
				}
				fingerprints[i] = (uint64_t) X;
			}
		}
		
		/* Calculates the powers of 2. This reduces the time from polylogarithmic to logarithmic. */
		void calculatePowers() {
			unsigned int numberOfLevels = ((int) std::log2(textLengthInBlocks)) + 6; // +1 to round up and +4 because we need bit shifts by 1byte, 2byte, 4byte and 8byte
			//powerTable = new uint64_t[numberOfLevels];
			unsigned __int128 X = 256;
			powerTable[0] = (uint64_t) X;
			for (unsigned int i = 1; i < numberOfLevels; i++) {
				X = (X*X) % prime;
				powerTable[i] = (uint64_t) X;
			}
		}
		
		/* Calculates the fingerprint of T[from, from + 2^exp) */
		uint64_t fingerprintExp(const uint64_t from, const int exp) {
			if (unlikely(from == 0)) {
				return fingerprintTo((1 << exp)-1); // ie if exponent = 3, we want P[0..7];
			} else {
				unsigned __int128 fingerprintToI = fingerprintTo(from - 1);
				unsigned __int128 fingerprintToJ = fingerprintTo(from + (1 << exp) - 1);
				fingerprintToI *= powerTable[exp];
				fingerprintToI %= prime;

				return fingerprintToJ >= fingerprintToI ? (uint64_t) (fingerprintToJ - fingerprintToI) : uint64_t (fingerprintToI - fingerprintToJ);
			}
		}
		
		/* Calculates the fingerprint of T[0..i] */
		uint64_t fingerprintTo(const uint64_t i) {
			unsigned __int128 fingerprint = 0;
			//int pad = ((i+1) % 8) * 8;
			int pad = ((i+1) & 7) * 8;
			if(pad == 0) {
				// This fingerprints is already saved. We only have to remove the helping bit.
				return fingerprints[i/8] < 0x8000000000000000ULL ? fingerprints[i/8] : fingerprints[i/8] - 0x8000000000000000ULL; 
			}
			/* Add fingerprint from previous block */
			if (i > 7) {
				fingerprint = fingerprints[((i/8) - 1)] &= 0x7FFFFFFFFFFFFFFFULL;
				fingerprint <<= pad;
			}
			/* Add relevant part of block */
			fingerprint += (getBlock(i/8) >> (64-pad));
			fingerprint %= prime;
			return (uint64_t) fingerprint; 
		}
};
