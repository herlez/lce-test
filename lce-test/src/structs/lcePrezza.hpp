#include <math.h>
#include "lceInterface.hpp"
#include "util.hpp"

/* This class builds Prezza's in-place LCE data structure and
 * answers LCE-queries in O(log(n)). */

class lcePrezza : public lceDataStructure {
	public:
		/* Loads the full file located at PATH and builds Prezza's LCE data structure */
		lcePrezza(std::string path) {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			textLengthInBytes = util::calculateSizeOfInputFile(&input);
			/* Calculate how many 8 byte blocks are needed */
			textLengthInBlocks = textLengthInBytes/8;
			if(textLengthInBytes%8 != 0) {
				textLengthInBlocks++;
			}
			fingerprints = new uint64_t[textLengthInBlocks];
			input.seekg(0);
			input.read((char*)fingerprints, textLengthInBytes);
			prime = util::getLow64BitPrime();
			calculateFingerprints();
			calculatePowers();
		}
		
		/* Loads a prefix of the file located at PATH and build Prezza's LCE data structure */ 
		lcePrezza(std::string path, uint64_t numberOfChars) {
			std::ifstream input(path, std::ios::in|std::ios::binary);
			util::inputErrorHandling(&input);
			uint64_t dataSize = util::calculateSizeOfInputFile(&input);
			if(numberOfChars > dataSize) {
				std::cerr << "Attemted to load the first " << numberOfChars << " Bytes, but the file " << path << " is only " << dataSize << " Bytes big." << std::endl;
				exit(EXIT_FAILURE);
			}
			textLengthInBytes = numberOfChars;
			/* Calculate how many 8 byte blocks are needed */
			textLengthInBlocks = textLengthInBytes/8;
			if(textLengthInBytes%8 != 0) {
				textLengthInBlocks++;
			}
			
			fingerprints = new uint64_t[textLengthInBlocks];
			input.seekg(0);
			input.read((char*)fingerprints, textLengthInBytes);
			prime = util::getLow64BitPrime();
			calculateFingerprints();
			calculatePowers();
		}
		
		/* Fast LCE-query in O(log(n)) time */
		uint64_t lce(uint64_t i, uint64_t j) {
			// Up to n characters are checked by doing a naive scan
			int n = 0; 
			// Factor which is used for the exponential search.
			if (i == j) {
				return textLengthInBytes - i;
			}
			
			/* naive scan */ 
			// TODO: more effective if a block is translated and saved
			for (int lce = 0; lce < n ; lce++) {
				if(i + lce >= textLengthInBytes || 
				 j + lce >= textLengthInBytes ||
				 getChar(i + lce) != getChar(j + lce)) {
					return lce;
				}
			}
			
			/* exponential search */
			int k = 0;
			uint64_t dist = 1;
			while(i+dist <= textLengthInBytes &&
			 j+dist <= textLengthInBytes &&
			 fingerprintExp(i, k) == fingerprintExp(j, k)) {
				k++;
				dist *= 2;
			}
			if (k == 0) {return 0;}
			if (k == 1) {return 1;}
			
			/*binary search */
			k--;
			dist /= 2;
			uint64_t i2 = i + dist;
			uint64_t j2 = j + dist;
			while(k != 0) {
				k--;
				dist /= 2;
				if(i2+dist <= textLengthInBytes &&
				 j2+dist <= textLengthInBytes &&
				 fingerprintExp(i2, k) == fingerprintExp(j2, k)) {
					i2 += dist;
					j2 += dist;
				}
			}
			return i2-i;
		}
		
		/* Returns the prime */
		unsigned __int128 getPrime() {
			return prime;
		}
		
		/* Returns the character at index i */ 
		char getChar(uint64_t i) {
			uint64_t blockNumber = i / 8;
			uint64_t offset = 7 - (i % 8);
			uint64_t block = getBlock(blockNumber);
			return ((char*)&block)[offset];
		}
	
	private:
		uint64_t * fingerprints;
		uint64_t textLengthInBytes;
		uint64_t textLengthInBlocks;
		unsigned __int128 prime;
		uint64_t * powerTable;
		
		/* Returns the i'th block. A block contains 8 character. */
		uint64_t getBlock(uint64_t i) {
			if (i == 0) {
				if(fingerprints[0] >= 0x8000000000000000ULL) {
					return fingerprints[0] - 0x8000000000000000ULL + prime ;
				} else {
					return fingerprints[0];
				}
			} else {
				unsigned __int128 X = fingerprints[i-1] > 0x8000000000000000ULL ? fingerprints[i-1] - 0x8000000000000000ULL : fingerprints [i-1];
				X = X << 64;
				X = X % prime;
				
				int sBit = 0;
				uint64_t currentFingerprint = fingerprints[i];
				if(currentFingerprint >= 0x8000000000000000ULL) {
					sBit = 1;
					currentFingerprint -=  0x8000000000000000ULL;
				}
				
				uint64_t Y = (uint64_t) X;
				if (Y <= currentFingerprint) {
					Y = currentFingerprint - Y;
				} else {
					Y = prime - (Y - currentFingerprint);
				}
				
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
			powerTable = new uint64_t[numberOfLevels];
			unsigned __int128 X = 256;
			powerTable[0] = (uint64_t) X;
			for (unsigned int i = 1; i < numberOfLevels; i++) {
				X = (X*X) % prime;
				powerTable[i] = (uint64_t) X;
			}
		}
		
		/* Calculates the fingerprint of T[from, 2^exp) */
		uint64_t fingerprintExp(uint64_t from, int exp) {
			if (from == 0) {
				return fingerprintTo((1 << exp)-1); // ie if exponent = 3, we want P[0..7];
			} else {
				unsigned __int128 fingerprintToI = fingerprintTo(from - 1);
				unsigned __int128 fingerprintToJ = fingerprintTo(from + (1 << exp) - 1);
				fingerprintToI *= powerTable[exp];
				fingerprintToI %= prime;

				if (fingerprintToJ >= fingerprintToI) {
					return (uint64_t) (fingerprintToJ - fingerprintToI);
				} else {
					return (uint64_t) (prime - (fingerprintToI - fingerprintToJ));
				}
			}
		}
		
		/* Calculates the fingerprint of T[0..i] */
		uint64_t fingerprintTo(uint64_t i) {
			unsigned __int128 fingerprint = 0;
			int pad = ((i+1) % 8) * 8; 
			
			if(pad == 0) {
				// This fingerprints is already saved. We only have to remove the helping bit.
				return fingerprints[i/8] < 0x8000000000000000ULL ? fingerprints[i/8] : fingerprints[i/8] - 0x8000000000000000ULL; 
			}
			/* Add fingerprint from previous block */
			if (i > 7) {
				fingerprint = fingerprints[((i/8) - 1)];
				if (fingerprint > 0x8000000000000000) {
					fingerprint -= 0x8000000000000000;
				}
				fingerprint <<= pad;
			}
			/* Add relevant part of block */
			fingerprint += (getBlock(i/8) >> (64-pad));
			fingerprint %= prime;
			return (uint64_t) fingerprint; 
		}
};
