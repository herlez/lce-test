#include <fstream>
#include <sys/time.h>
#include <vector>
#include <iomanip>
#include <ctime>
#include "structs/lceNaive.hpp"
#include "structs/lcePrezza.hpp"
#include "structs/lcePrezzaMersenne.hpp"

using namespace std;

const string fileNames[] {"english"};
const string files[] {"/scratch/text/english"};
//const string files[] {"../../text/dna"};

const string lceSet[] = {"../res/lceDna/i0", "../res/lceDna/i1", "../res/lceDna/i2", "../res/lceDna/i3", "../res/lceDna/i4", "../res/lceDna/i5", "../res/lceDna/i6", "../res/lceDna/i7", "../res/lceDna/i8", "../res/lceDna/i9", "../res/lceDna/i10", "../res/lceDna/i11", "../res/lceDna/i12", "../res/lceDna/i13", "../res/lceDna/i14", "../res/lceDna/i15", "../res/lceDna/i16","../res/lceDna/i17","../res/lceDna/i18", "../res/lceDna/i19", "../res/lceDna/iH"};
const int NUMBEROFSETS = 21;


const int SETPAD = 0;
/* Up to 100000000 tests possible */
const uint64_t NUMBEROFTESTS = 100000ULL;

double timestamp();



int main(int argc, char *argv[]) {
	// Results output
	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
    auto str = oss.str();
	ofstream log(string("../testResults/time-") + str + string(".txt"), ios::out|ios::trunc);
	
	/************************************
	 ****PREPARE LCE DATA STRUCTURES*****
	 ************************************/
	 
	/* Build data structures */
	LceNaive dataN(files[0]);
	LcePrezza dataP(files[0]);
	//rklce::LcePrezzaMersenne dataPM(files[0]);

	const int NUMBEROFSTRUCTS = 2;
	LceDataStructure * lceData[NUMBEROFSTRUCTS] {&dataN, &dataP}; //, &dataPM};
	string algo[NUMBEROFSTRUCTS] {"naiveLCE", "prezzaLCE"};//, "prezzaMersenneLCE"}; 
	
	
	/************************************
	 *******PREPARE RANDOM INDEXES*******
	 ************************************/
	
	/* LCE query test */
	for(int k = SETPAD; k < NUMBEROFSETS; ++k) {
		ifstream lc(lceSet[k], ios::in);
		util::inputErrorHandling(&lc);
		/* We use indexes which lead to lce results in defined ranges.
		 * Here we extract the indexes and save them in a vector */
		vector<uint64_t> v;
		string line;
		
		string::size_type sz;
		while(getline(lc, line)) {
			v.push_back(stoi(line, &sz));
		}
		
		/* We use a fixed size array in order to answer LCE queries,
		 * because we do not want an overhead by checking
		 * if we are out of bound of the vector */
		
		uint64_t * lceI = new uint64_t[NUMBEROFTESTS*2];
		for(uint64_t k = 0; k < NUMBEROFTESTS * 2; ++k) {
			lceI[k] = v[k % v.size()];
		}
		
		
		/************************************
		*************LCE QUERIES*************
		************************************/
		// Indexes for lce queries
		uint64_t i, j;
		// Result of lce query
		uint64_t lce = 0;
		// Timestamps
		double ts1, ts2;
		// For every lce Datastructure..
		for(int alg = 0; alg < NUMBEROFSTRUCTS; ++alg) { 
			// ..do NUMBEROFTESTS LCE queries
			ts1 = timestamp();
			
			for(uint64_t k = 0; k < NUMBEROFTESTS*2; ++k) {
				i = lceI[k];
				j = lceI[++k];
				lce += lceData[alg]->lce(i, j);
				//if(lceDa
			}
			ts2 = timestamp();
			log << "RESULT"
				<< " text=" << fileNames[0]
				<< " algo=" << algo[alg]
				<< " lceQueries=" << NUMBEROFTESTS
				<< " time=" << ts2-ts1
				<< " lce=" << lce
				<< " aveLce=" << lce/NUMBEROFTESTS
				<< " lceLog=" << k
				<< endl;
			lce = 0;
		}
		delete[] lceI;
	}
	return EXIT_SUCCESS;
}


double timestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}
