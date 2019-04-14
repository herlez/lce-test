#include <fstream>
#include <sys/time.h>
#include "structs/util.hpp"

#include "structs/lceNaive.hpp"
#include "structs/lcePrezza.hpp"
using namespace std;

const string fileNames{"english"};
//const string files[] {"../../../text/english"};
const string files[] {"/scratch/text/english"};

double timestamp();


int main(int argc, char *argv[]) {
	ofstream log("../test/correctness.test", ios::out|ios::trunc);
	lceNaive T1(files[0], 4096);
	lcePrezza T2(files[0], 4096);
	
	log << "Prime: "; util::printInt128(T2.getPrime());
	log << "Block extraction test" << endl;
	
	/* Test if character extraction works */
	log << "Character extraction test" << endl;
	for(uint64_t i = 0; i < 1024; i++) {
		if (T1.getChar(i) != T2.getChar(i)) {
			log << "Mismatch at position " << i << endl;
			log << "NaiveLCE: " << T1.getChar(i) << endl;
			log << "FastLCE: " << T2.getChar(i) << endl; 
		}
	}

	/* Test if LCE-queries work */
	uint64_t i, j;
	uint64_t textSizeInBytes = T2.getSizeInBytes();
	log << "LCE queries test" << endl;
	for(uint64_t k = 0; k < 100000000; k++) {
		i = util::randomIndex(textSizeInBytes);
		j = util::randomIndex(textSizeInBytes);
		if (T1.lce(i, j) != T2.lce(i, j)) {
			log << "Wrong LCE at position " << i << " and " << j << endl;
			log << "NaiveLCE says: " << T1.lce(i, j) << endl;
			log << "FastLCE says: " << T2.lce(i, j) << endl;
		}
	}
	return EXIT_SUCCESS;
}

double timestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}
