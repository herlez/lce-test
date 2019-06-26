#include <fstream>
#include <sys/time.h>
#include <vector>
#include <iomanip>
#include <ctime>
#include "structs/lceNaive.hpp"
#include "structs/lcePrezza.hpp"
#include "structs/lcePrezzaMersenne.hpp"

using namespace std;

const string fileName = "dna";
const string file = "../../text/" + fileName;
const string lSet = "../res/lce_" + fileName; 
const string lceSet[] = {lSet + "/i0", lSet + "/i1", lSet + "/i2", lSet + "/i3", lSet + "/i4", lSet + "/i5", lSet + "/i6", lSet + "/i7", lSet + "/i8", lSet + "/i9", lSet + "/i10", lSet + "/i11", lSet + "/i12", lSet + "/i13", lSet + "/i14", lSet + "/i15", lSet + "/i16",lSet + "/i17",lSet + "/i18", lSet + "/i19", lSet + "/iH"};
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
	//size_t size = 13000000000ULL/9;
	LceNaive dataN(file);
	LcePrezza dataP(file);
	rklce::LcePrezzaMersenne dataPM(file);

	const int NUMBEROFSTRUCTS = 3;
	LceDataStructure * lceData[NUMBEROFSTRUCTS] {&dataN, &dataP, &dataPM};
	string algo[NUMBEROFSTRUCTS] {"naiveLCE", "prezzaLCE",  "prezzaMersenneLCE"}; 
	
	/*
	const int NUMBEROFSTRUCTS = 2;
	LceDataStructure * lceData[NUMBEROFSTRUCTS] {&dataN, &dataP};
	string algo[NUMBEROFSTRUCTS] {"naiveLCE", "prezzaLCE"};
	*/
	
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
				/* //ERROR-HUNT
				 * if(lceData[0]->lce(i,j) != lceData[1]->lce(i,j)) {
					cout << "Lce: " << lceData[0]->lce(i,j) << endl;
					cout << "wLce: " << lceData[1]->lce(i,j) << endl;
					cout << "i: " << i << "  j: " << j << endl;
				}*/
			}
			ts2 = timestamp();
			log << "RESULT"
				<< " text=" << fileName
				<< " algo=" << algo[alg]
				<< " lceQueries=" << NUMBEROFTESTS
				<< " time=" << ts2-ts1
				<< " lce=" << lce
				<< " aveLce=" << lce/NUMBEROFTESTS
				<< " lceLog=" << k
				<< endl;
			lce = 0;
		}
		log << "---" << endl;
		delete[] lceI;
	}
	return EXIT_SUCCESS;
}


double timestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}
