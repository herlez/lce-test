#include <fstream>
#include <sys/time.h>
#include <vector>
#include <iomanip>

#include "structs/lce_naive.hpp"
#include "structs/lce_naive_ultra.hpp"

#include "structs/lce_prezza.hpp"
#include "structs/lce_prezza_mersenne.hpp"

#include "structs/lce_semi_synchronizing_sets.hpp"


#define benchmark_ordered_by_lce
//#define benchmark_random
//#define benchmark_complete

using namespace std;

const string file_name = "dna";
const string file = "../../text/" + file_name;
//const string file = "/scratch/text/" + file_name;



// We load the path to the precomputed lce queries
#ifdef benchmark_ordered_by_lce
const string lce_path = "../res/lce_" + file_name;
const array<string, 21> lce_set{lce_path + "/i0", lce_path + "/i1", lce_path + "/i2", lce_path + "/i3", lce_path + "/i4", lce_path + "/i5", lce_path + "/i6", lce_path + "/i7", lce_path + "/i8", lce_path + "/i9", lce_path + "/i10", lce_path + "/i11", lce_path + "/i12", lce_path + "/i13", lce_path + "/i14", lce_path + "/i15", lce_path + "/i16",lce_path + "/i17",lce_path + "/i18", lce_path + "/i19", lce_path + "/iH"};

const uint64_t kNumberOfTests = 1'000'000ULL; 
#endif


#ifdef benchmark_random
const uint64_t kNumberOfTests = 100'000'000ULL;
#endif

#ifdef benchmark_complete
#endif



int main(int argc, char *argv[]) {
	// Set up log to measure time
	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
    auto str = oss.str();
	ofstream log(string("../test_results/time-") + str + string(".txt"), ios::out|ios::trunc);
	log << "FILE: " << file << "   SIZE(Byte): " << util::calculateSizeOfInputFile(file) << std::endl;
	log << "---" << std::endl;
	
	// FOR FUTURE COMMAND LINE COMPABILITY:
	// a flag for every data structure, decides if this data structure will be tested
	const bool f1 = false, f2 = true, f3 = true, f4 = true, f5 = true;
	// a flag for the mode of the benchmark
	//const bool benchmark_lce_set1 = true;
	//const bool benchmark_random1 = benchmark_lce_set1;
	//const bool benchmark_complete1 = false;
	
	
	
	
	
	/************************************
	 ****PREPARE LCE DATA STRUCTURES*****
	 ************************************/
	 
	/* Build data structures */
	// Timer to keep track of construction time
	util::Timer timer{}; 
	// time for construction is saved in ts
	double ts; 
	
	// pointer to every tested data structure
	std::vector<LceDataStructure*> lce_data_structures{};
	// names of tested data structure
	std::vector<string> lce_data_structure_names{};
	
	
	if(f1) {
		timer.reset();
		LceUltraNaive * dataUN = new LceUltraNaive{file};
		ts = timer.elapsed();
		log << "RESULT algo=construction structure=ultra_naive_lce time=" << ts << std::endl;
		lce_data_structures.push_back(dataUN);
		lce_data_structure_names.push_back("ds_ultra_naive_lce");
	}

	if(f2) {
		timer.reset();
		LceNaive * dataN = new LceNaive{file};
		ts = timer.elapsed();
		log << "RESULT algo=construction structure=naive_lce time=" << ts << std::endl;
		lce_data_structures.push_back(dataN);
		lce_data_structure_names.push_back("ds_naive_lce");
	}
	
	if(f3) {
		timer.reset();
		rklce::LcePrezzaMersenne * dataPM = new rklce::LcePrezzaMersenne{file};
		ts = timer.elapsed();
		log << "RESULT algo=construction structure=prezza_mersenne_lce=" << ts << std::endl;
		lce_data_structures.push_back(dataPM);
		lce_data_structure_names.push_back("ds_prezza_mersenne_lce");
	}
	
	if(f4) {
		timer.reset();
		LcePrezza * dataP = new LcePrezza{file};
		ts = timer.elapsed();
		log << "RESULT algo=construction structure=prezza_lce time=" << ts << std::endl;
		lce_data_structures.push_back(dataP);
		lce_data_structure_names.push_back("ds_prezza_lce");
	}
	
	if(f5) {
		timer.reset();
		LceSemiSyncSets * dataSSS = new LceSemiSyncSets{file};
		ts = timer.elapsed();
		log << "RESULT algo=construction structure=sss_lce time=" << ts << std::endl;
		lce_data_structures.push_back(dataSSS);
		lce_data_structure_names.push_back("ds_sss_lce");
	}
	
	log << lce_data_structures.size() << " datastructure(s) build successfully:" << std::endl;
	for(string& s : lce_data_structure_names) { log << s << "; "; }
	log << std::endl << "---" << std::endl;
	
	
	/************************************
	 ******** PREPARE INDEXES ***********
	 ************************************/

#if defined(benchmark_ordered_by_lce)
	for(unsigned int number_of_runs = 0; number_of_runs < lce_set.size(); ++number_of_runs) {
		vector<uint64_t> v;
		vector<uint64_t> lce_indices;
		lce_indices.resize(kNumberOfTests*2);
		//uint64_t * lce_indices = new uint64_t[kNumberOfTests*2];
		
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
		double ts;
		// For every lce data structure..
		for(unsigned int alg = 0; alg < lce_data_structures.size(); ++alg) {
			// ..do NUMBEROFTESTS LCE queries
			//dataSSS.resetTimer();
			timer.reset();
			
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
				
			ts = timer.elapsed();
			log << "RESULT"
				<< " text=" << file_name
				<< " algo=" << lce_data_structure_names[alg]
				<< " lceQueries=" << kNumberOfTests
				<< " time=" << ts
				<< " lce=" << lce
				<< " aveLce=" << lce/kNumberOfTests
				#if defined(benchmark_ordered_by_lce)
				<< " lceLog=" << number_of_runs
				#endif
				<< endl;
			lce = 0;
		}
		//log << "RESULT"
		//	<< " algo= ssssLce"
		//	<< " naive_part=" << dataSSS.getTimeNaive()
		//	<< " rank_part=" << dataSSS.getTimeRank()
		//	<< " sss_part=" << dataSSS.getTimeRmq()
		//	<< endl;
		//delete[] lce_indices;
#endif

#if defined(benchmark_complete)
			const uint64_t max_index = 1000U < lce_data_structures[0]->getSizeInBytes() ? 1000U : lce_data_structures[0]->getSizeInBytes();
			for(uint64_t i = 0; i < max_index; ++i) {
				for(uint64_t j = 0; j < max_index; ++j) {
					lce += lce_data_structures[alg]->lce(i, j);
				}
			}
			ts = time.elapsed();
			log << "RESULT"
				<< " benchmark=" << "complete"
				<< " text=" << fileName
				<< " algo=" << lce_data_structure_names[alg]
				<< " time=" << ts
				<< " lce=" << lce
				<< " aveLce=" << lce/(max_index*max_index)
				<< endl;
			lce = 0;
#endif

		log << "---" << endl;
	}
	return EXIT_SUCCESS;
}



