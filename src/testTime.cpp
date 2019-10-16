#include <fstream>
#include <sys/time.h>
#include <vector>
#include <iomanip>
#include <ctime>
#include "structs/lceNaive.hpp"
#include "structs/lcePrezza.hpp"
#include "structs/lcePrezzaMersenne.hpp"
#include "structs/lceSyncSets.hpp"

#define benchmark_ordered_by_lce
//#define benchmark_random
//#define benchmark_complete

using namespace std;

//const string fileName = "benutzungsrichtlinie.txt";
const string fileName = "ecoli.fa";
//const string file = "../../text/" + fileName;
const string file = "/scratch/text/" + fileName;



// Here we ...
#ifdef benchmark_ordered_by_lce
const string lSet = "../res/lce_" + fileName;
const string lceSet[] = {lSet + "/i0", lSet + "/i1", lSet + "/i2", lSet + "/i3", lSet + "/i4", lSet + "/i5", lSet + "/i6", lSet + "/i7", lSet + "/i8", lSet + "/i9", lSet + "/i10", lSet + "/i11", lSet + "/i12", lSet + "/i13", lSet + "/i14", lSet + "/i15", lSet + "/i16",lSet + "/i17",lSet + "/i18", lSet + "/i19", lSet + "/iH"};
const int NUMBEROFSETS = 12;
const uint64_t NUMBEROFTESTS = 1'000'000ULL; 
#endif


#ifdef benchmark_random
const uint64_t NUMBEROFTESTS = 100'000'000ULL;
#endif

#ifdef benchmark_complete
#endif


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
	LceNaive dataN(file);
	LcePrezza dataP(file);
	//rklce::LcePrezzaMersenne dataPM(file);
	LceSyncSets dataSSS(file);

	
	const int NUMBEROFSTRUCTS = 2;
	cout << "structures build successfully" << std::endl;
	LceDataStructure * lceData[NUMBEROFSTRUCTS] {&dataN, &dataSSS};
	string algo[NUMBEROFSTRUCTS] {"naiveLCE", "sssLCE"};
	
	
	/************************************
	 ******** PREPARE INDEXES ***********
	 ************************************/

#if defined(benchmark_ordered_by_lce)
	for(int numberOfRuns = 0; numberOfRuns < NUMBEROFSETS; ++numberOfRuns) {
		vector<uint64_t> v;
		uint64_t * lceI = new uint64_t[NUMBEROFTESTS*2];
		
		ifstream lc(lceSet[numberOfRuns], ios::in);
		util::inputErrorHandling(&lc);
		
		string line;
		string::size_type sz;
		while(getline(lc, line)) {
			v.push_back(stoi(line, &sz));
		}
		
		for(uint64_t k = 0; k < NUMBEROFTESTS * 2; ++k) {
			lceI[k] = v[k % v.size()];
		}
#endif
		
#if defined(benchmark_random)
	for(int numberOfRuns = 0; numberOfRuns < 1; ++numberOfRuns) {
		srand(time(NULL));
		uint64_t * lceI = new uint64_t[NUMBEROFTESTS*2];
		for(uint64_t k = 0; k < NUMBEROFTESTS*2; ++k) {
			lceI[k] = rand() % lceData[0]->getSizeInBytes();
		}
#endif


		/************************************
		*************LCE QUERIES*************
		************************************/
		
		// Result of lce query
		uint64_t lce = 0;
		// Timestamps
		double ts1, ts2;
		// For every lce data structure..
		for(int alg = 0; alg < NUMBEROFSTRUCTS; ++alg) {
			// ..do NUMBEROFTESTS LCE queries
			ts1 = timestamp();
			
#if defined(benchmark_ordered_by_lce) || defined(benchmark_random)
			// Indexes for lce queries
			uint64_t i, j;
			for(uint64_t k = 0; k < NUMBEROFTESTS*2; ++k) {
				i = lceI[k];
				j = lceI[++k];
				lce += lceData[alg]->lce(i, j);
			}
				/* //ERROR-HUNT
				  if(lceData[0]->lce(i,j) != lceData[1]->lce(i,j)) {
					cout << "Lce: " << lceData[0]->lce(i,j) << endl;
					cout << "wLce: " << lceData[1]->lce(i,j) << endl;
					cout << "i: " << i << "  j: " << j << endl;
				}*/
			
			ts2 = timestamp();
			log << "RESULT"
				<< " text=" << fileName
				<< " algo=" << algo[alg]
				<< " lceQueries=" << NUMBEROFTESTS
				<< " time=" << ts2-ts1
				<< " lce=" << lce
				<< " aveLce=" << lce/NUMBEROFTESTS
				#if defined(benchmark_ordered_by_lce)
				<< " lceLog=" << numberOfRuns
				#endif
				<< endl;
			lce = 0;
		}
		delete[] lceI;
#endif

#if defined(benchmark_complete)
			const uint64_t maxIndex = 100000U < lceData[0]->getSizeInBytes() ? 100000U : lceData[0]->getSizeInBytes();
			for(uint64_t i = 0; i < maxIndex; ++i) {
				for(uint64_t j = 0; j < maxIndex; ++j) {
					lce += lceData[alg]->lce(i, j);
				}
			}
			ts2 = timestamp();
			log << "RESULT"
				<< " benchmark=" << "complete"
				<< " text=" << fileName
				<< " algo=" << algo[alg]
				<< " time=" << ts2-ts1
				<< " lce=" << lce
				<< " aveLce=" << lce/(maxIndex*maxIndex)
				<< endl;
			lce = 0;
#endif

		log << "---" << endl;
	}
	return EXIT_SUCCESS;
}


double timestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}
