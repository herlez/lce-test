#include <fstream>
#include <sys/time.h>
#include <vector>
#include <iomanip>
#include <ctime>

#include "structs/lce_naive.hpp"

#include "structs/lce_prezza.hpp"
#include "structs/lce_prezza_mersenne.hpp"

#include "structs/lce_synchronizing_sets.hpp"
#include "structs/lce_semi_synchronizing_sets.hpp"

#include "malloc_count/malloc_count.h"
#include "malloc_count/stack_count.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


using namespace std;
const string file_name = "english.1024MB";
const string file = "../../text/" + file_name;


int main(int argc, char *argv[]) {
	// Set up log to measure time
	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
    auto str = oss.str();
	ofstream log(string("../test_results/mem-") + str + string(".txt"), ios::out|ios::trunc);
	log << "FILE: " << file << "   SIZE(Byte): " << util::calculateSizeOfInputFile(file) << std::endl;
	log << "---" << std::endl;
	
	uint64_t mem1, mem2;
	
	
	mem1 = malloc_count_current();
	LceNaive dataN(file);
	mem2 = malloc_count_current();
	log << "RESULT ds=naive " << "mem=" << mem2-mem1 << endl;
	
	mem1 = malloc_count_current();
	LcePrezza dataP(file);
	mem2 = malloc_count_current();
	log << "RESULT ds=prezza_naive " << "mem=" << mem2-mem1 << endl;
	
	mem1 = malloc_count_current();
	LceSemiSyncSets dataSSS(file);
	mem2 = malloc_count_current();
	log << "RESULT ds=ssss " << "mem=" << mem2-mem1 << endl;
	
	mem1 = malloc_count_current();
	rklce::LcePrezzaMersenne dataPM(file);
	mem2 = malloc_count_current();
	log << "RESULT ds=prezza_mersenne " << "mem=" << mem2-mem1 << endl;
	return 0;
}