#include <fstream>
#include <sys/time.h>
#include <vector>
#include <iomanip>
#include <ctime>
#include "structs/lce_naive.hpp"
#include "structs/lce_prezza.hpp"
#include "structs/lce_prezza_mersenne.hpp"
#include "structs/lce_synchronizing_sets.hpp"

#define benchmark_ordered_by_lce
//#define benchmark_random
//#define benchmark_complete

using namespace std;

const string file_name = "dna";
const string file = "../../text/" + file_name;
//const string file = "/scratch/text/" + fileName;



// We load the path to the precomputed lce queries
#ifdef benchmark_ordered_by_lce
const string lce_path = "../res/lce_" + file_name;
const array<string, 21> lce_set{lce_path + "/i0", lce_path + "/i1", lce_path + "/i2", lce_path + "/i3", lce_path + "/i4", lce_path + "/i5", lce_path + "/i6", lce_path + "/i7", lce_path + "/i8", lce_path + "/i9", lce_path + "/i10", lce_path + "/i11", lce_path + "/i12", lce_path + "/i13", lce_path + "/i14", lce_path + "/i15", lce_path + "/i16",lce_path + "/i17",lce_path + "/i18", lce_path + "/i19", lce_path + "/iH"};

const int kNumberOfSets = 21;
const uint64_t kNumberOfTests = 1'000'000ULL; 
#endif


#ifdef benchmark_random
const uint64_t kNumberOfTests = 100'000'000ULL;
#endif

#ifdef benchmark_complete
#endif


double timestamp();

int main(int argc, char *argv[]) {
	// Set up log to measure time
	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
    auto str = oss.str();
	ofstream log(string("../testResults/time-") + str + string(".txt"), ios::out|ios::trunc);
	log << "FILE: " << file << "   SIZE(Byte): " << util::calculateSizeOfInputFile(file) << std::endl;
	log << "---" << std::endl;
	/************************************
	 ****PREPARE LCE DATA STRUCTURES*****
	 ************************************/
	 
	/* Build data structures */
	double ts1, ts2;
	ts1 = timestamp();
	LceNaive dataN(file);
	ts2 = timestamp();
	log << "RESULT algo=naiveLCE time=" << ts2 - ts1 << std::endl;
	
	ts1 = timestamp();
	LcePrezza dataP(file);
	ts2 = timestamp();
	log << "RESULT algo=prezzaLCE time=" << ts2 - ts1 << std::endl;
	//rklce::LcePrezzaMersenne dataPM(file);
	
	ts1 = timestamp();
	LceSyncSets dataSSS(file);
	ts2 = timestamp();
	log << "RESULT algo=sssLCE time=" << ts2 - ts1 << std::endl;
	
	log << "structures build successfully" << std::endl;
	log << "---" << std::endl;
	
	
	std::array<LceDataStructure*, 3> lce_data_structures {&dataN, 
	&dataP, &dataSSS};
	string algo[lce_data_structures.size()] {"naiveLCE", "prezzaLCE", "sssLCE"};
	
	/************************************
	 ******** PREPARE INDEXES ***********
	 ************************************/

#if defined(benchmark_ordered_by_lce)
	for(int number_of_runs = 0; number_of_runs < kNumberOfSets; ++number_of_runs) {
		vector<uint64_t> v;
		uint64_t * lce_indices = new uint64_t[kNumberOfTests*2];
		
		ifstream lc(lce_set[number_of_runs], ios::in);
		util::inputErrorHandling(&lc);
		
		string line;
		string::size_type sz;
		while(getline(lc, line)) {
			v.push_back(stoi(line, &sz));
		}
		
		for(uint64_t i = 0; i < kNumberOfTests * 2; ++i) {
			lce_indices[i] = v[i % v.size()];
		}
#endif
		
#if defined(benchmark_random)
	for(int number_of_runs = 0; number_of_runs < 1; ++number_of_runs) {
		srand(time(NULL));
		uint64_t * lce_indices = new uint64_t[kNumberOfTests*2];
		for(uint64_t k = 0; k < kNumberOfTests*2; ++k) {
			lce_indices[k] = rand() % lce_data_structures[0]->getSizeInBytes();
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
		for(unsigned int alg = 0; alg < lce_data_structures.size(); ++alg) {
			// ..do NUMBEROFTESTS LCE queries
			ts1 = timestamp();
			
#if defined(benchmark_ordered_by_lce) || defined(benchmark_random)
			// Indexes for lce queries
			uint64_t i, j;
			for(uint64_t k = 0; k < kNumberOfTests*2; ++k) {
				i = lce_indices[k];
				j = lce_indices[++k];
				lce += lce_data_structures[alg]->lce(i, j);
			}
				/* //ERROR-HUNT
				  if(lce_data_structures[0]->lce(i,j) != lce_data_structures[1]->lce(i,j)) {
					cout << "Lce: " << lce_data_structures[0]->lce(i,j) << endl;
					cout << "wLce: " << lce_data_structures[1]->lce(i,j) << endl;
					cout << "i: " << i << "  j: " << j << endl;
				}*/
			
			ts2 = timestamp();
			log << "RESULT"
				<< " text=" << file_name
				<< " algo=" << algo[alg]
				<< " lceQueries=" << kNumberOfTests
				<< " time=" << ts2-ts1
				<< " lce=" << lce
				<< " aveLce=" << lce/kNumberOfTests
				#if defined(benchmark_ordered_by_lce)
				<< " lceLog=" << number_of_runs
				#endif
				<< endl;
			lce = 0;
		}
		delete[] lce_indices;
#endif

#if defined(benchmark_complete)
			const uint64_t max_index = 1000U < lce_data_structures[0]->getSizeInBytes() ? 1000U : lce_data_structures[0]->getSizeInBytes();
			for(uint64_t i = 0; i < max_index; ++i) {
				for(uint64_t j = 0; j < max_index; ++j) {
					lce += lce_data_structures[alg]->lce(i, j);
				}
			}
			ts2 = timestamp();
			log << "RESULT"
				<< " benchmark=" << "complete"
				<< " text=" << fileName
				<< " algo=" << algo[alg]
				<< " time=" << ts2-ts1
				<< " lce=" << lce
				<< " aveLce=" << lce/(max_index*max_index)
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
