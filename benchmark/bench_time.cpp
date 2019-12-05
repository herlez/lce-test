/*******************************************************************************
 * benchmark/bench_time.cpp
 *
 * Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <fstream>
#include <sys/time.h>
#include <vector>
#include <iomanip>

#include <tlx/cmdline_parser.hpp>

#include "build_lce_ranges.hpp"
#include "lce_naive.hpp"
#include "lce_naive_ultra.hpp"
#include "lce_prezza.hpp"
#include "lce_prezza_mersenne.hpp"
#include "lce_semi_synchronizing_sets.hpp"


using namespace std;

bool benchmark_ordered = false;
bool benchmark_random = false;
bool benchmark_complete = false;

uint64_t prefix_length = 0;
unsigned int lce_from = 0;
unsigned int lce_to = 21;

std::string file{};
std::string file_name{};
std::string output_path{};

uint64_t number_of_lce_queries;
uint64_t number_of_runs;

bool test_ultra_naive = false, test_naive = false, test_prezza_mersenne = false, test_prezza = false, test_sss = false;


class lce_benchmark {

public:
  void run() {

    file_name = util::getFileName(file);
    if (mode == "complete" || mode == "c" || mode == "comp" || mode == "compl") {
      benchmark_complete = true;
      number_of_runs = 1;
      lce_from = 0;
      lce_to = 1;
    } else if (mode == "rand" || mode == "r" || mode == "random") {
      benchmark_random = true;
      number_of_lce_queries = 10'000'000ULL;
      lce_from = 0;
      lce_to = 1;
    }  else if (true || mode == "ord" || mode == "o" || mode == "order" || mode == "ordered") {
      benchmark_ordered = true;
      number_of_lce_queries = 1'000'000ULL;
      lce_from = lce_from;
      lce_to = lce_to;
    }
    
    if(prefix_length == 0) {
      prefix_length = util::calculateSizeOfInputFile(file);
    }
    if(prefix_length > util::calculateSizeOfInputFile(file)) {
      std::cerr << "File is only " << util::calculateSizeOfInputFile(file) << " bytes long."
                << " Reading " << prefix_length << " bytes is not possible." << std::endl;
      std::exit(-1);
    }
	
    if(output_path.empty()) {
      output_path = "../test_results";
    }
    if(!test_ultra_naive && !test_naive && !test_prezza_mersenne && !test_prezza && !test_sss) {
      test_ultra_naive = test_naive = test_prezza_mersenne = test_prezza = test_sss = true;
    }
    
    // Set up log to measure time
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
    auto str = oss.str();
    ofstream log(output_path + string("/time-") + str + string(".txt"), ios::out|ios::trunc);
    log << "FILE: " << file << "   SIZE(Byte): " << util::calculateSizeOfInputFile(file) << "   PREFIX(Byte): " << prefix_length << std::endl;
    log << "---" << std::endl;
    
    
    
    
    const string lce_path = "../res/" + file_name + "_" + std::to_string(prefix_length);
    const array<string, 21> lce_set{lce_path + "/lce_0", lce_path + "/lce_1", lce_path + "/lce_2", lce_path + "/lce_3", lce_path + "/lce_4", lce_path + "/lce_5", lce_path + "/lce_6", lce_path + "/lce_7", lce_path + "/lce_8", lce_path + "/lce_9", lce_path + "/lce_10", lce_path + "/lce_11", lce_path + "/lce_12", lce_path + "/lce_13", lce_path + "/lce_14", lce_path + "/lce_15", lce_path + "/lce_16",lce_path + "/lce_17",lce_path + "/lce_18", lce_path + "/lce_19", lce_path + "/lce_X"};
    if(benchmark_ordered) {
      log << "Indices for sorted lce queries have not been computated yet. Computing them now..";
      build_lce_range(file, std::string("../res/") + file_name, prefix_length);
      log << "..done" << std::endl;
    }
    
    

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

    if(test_ultra_naive) {
      timer.reset();
      LceUltraNaive * dataUN = new LceUltraNaive{file, prefix_length};
      ts = timer.elapsed();
      log << "RESULT algo=construction structure=ultra_naive_lce time=" << ts << std::endl;
      lce_data_structures.push_back(dataUN);
      lce_data_structure_names.push_back("ds_ultra_naive_lce");
    }

    if(test_naive) {
      timer.reset();
      LceNaive * dataN = new LceNaive{file, prefix_length};
      ts = timer.elapsed();
      log << "RESULT algo=construction structure=naive_lce time=" << ts << std::endl;
      lce_data_structures.push_back(dataN);
      lce_data_structure_names.push_back("ds_naive_lce");
    }
    
    if(test_prezza_mersenne) {
      timer.reset();
      rklce::LcePrezzaMersenne * dataPM = new rklce::LcePrezzaMersenne{file, prefix_length};
      ts = timer.elapsed();
      log << "RESULT algo=construction structure=prezza_mersenne_lce=" << ts << std::endl;
      lce_data_structures.push_back(dataPM);
      lce_data_structure_names.push_back("ds_prezza_mersenne_lce");
    }
    
    if(test_prezza) {
      timer.reset();
      LcePrezza * dataP = new LcePrezza{file};
      ts = timer.elapsed();
      log << "RESULT algo=construction structure=prezza_lce time=" << ts << std::endl;
      lce_data_structures.push_back(dataP);
      lce_data_structure_names.push_back("ds_prezza_lce");
    }
    
    if(test_sss) {
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
    
    

    for(; lce_from < lce_to; ++lce_from) {
        
      /************************************
       ******** PREPARE INDEXES ***********
       ************************************/
      vector<uint64_t> lce_indices;
      lce_indices.resize(number_of_lce_queries*2);
        
        
      if(benchmark_ordered) {
        vector<uint64_t> v;
        log << "loading indices for lce queries: " << lce_set[lce_from] << std::endl;
        ifstream lc(lce_set[lce_from], ios::in);
        util::inputErrorHandling(&lc);
            
        string line;
        string::size_type sz;
        while(getline(lc, line)) {
          v.push_back(stoi(line, &sz));
        }
            
        if(v.size() == 0) {
          continue;
        } else  {
          log << v.size() << " indices loaded. Extending them to " << (2*number_of_lce_queries) << " indices." << std::endl;
        }
        for(uint64_t i = 0; i < number_of_lce_queries * 2; ++i) {
          lce_indices[i] = v[i % v.size()];
        }
            
      }

      if(benchmark_random) {
        uint64_t text_size = util::calculateSizeOfInputFile(file);
        srand(time(NULL));
        for(uint64_t i = 0; i < number_of_lce_queries * 2; ++i) {
          lce_indices[i] = rand() % text_size;
        }
      }



      /************************************
       *************LCE QUERIES*************
       ************************************/
        
      // Result of lce query
      uint64_t lce = 0;
      // For every lce data structure..
      for(unsigned int alg = 0; alg < lce_data_structures.size(); ++alg) {
        // ..do NUMBEROFTESTS LCE queries
        //dataSSS.resetTimer();
        timer.reset();
            
        if(benchmark_ordered || benchmark_random) {
          // Indexes for lce queries
          uint64_t i, j;
          for(uint64_t k = 0; k < number_of_lce_queries*2; ++k) {
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
              << " lceQueries=" << number_of_lce_queries
              << " time=" << ts
              << " lce=" << lce
              << " aveLce=" << lce/number_of_lce_queries
              << " lceLog=" << lce_from
              << endl;
          lce = 0;
        }
        
        //log << "RESULT"
        //  << " algo= ssssLce"
        //  << " naive_part=" << dataSSS.getTimeNaive()
        //  << " rank_part=" << dataSSS.getTimeRank()
        //  << " sss_part=" << dataSSS.getTimeRmq()
        //  << endl;
        //delete[] lce_indices;

        if(benchmark_complete) {
          const uint64_t max_index = 1000U < lce_data_structures[0]->getSizeInBytes() ? 1000U : lce_data_structures[0]->getSizeInBytes();
          for(uint64_t i = 0; i < max_index; ++i) {
            for(uint64_t j = 0; j < max_index; ++j) {
              lce += lce_data_structures[alg]->lce(i, j);
            }
          }
          ts = timer.elapsed();
          log << "RESULT"
              << " benchmark=" << "complete"
              << " text=" << file
              << " algo=" << lce_data_structure_names[alg]
              << " time=" << ts
              << " lce=" << lce
              << " aveLce=" << lce/(max_index*max_index)
              << endl;
          lce = 0;
        }
      }
      log << "---" << endl;
    }
    std::exit(0);
  }


public:
  std::string file;
  std::string file_name;
  std::string output_path;
  uint64_t prefix_length = 0;

  std::string algorithm;

  std::string mode;

  uint64_t number_lce_queries;
  uint64_t runs = 5;

  uint32_t lce_from = 0;
  uint32_t lce_to = 21;


private:
  bool ordered_ = false;
  bool random_ = false;
  bool complete_ = false;
}; // class lce_benchmark

int32_t main(int argc, char *argv[]) {
  lce_benchmark lce_bench;

  tlx::CmdlineParser cp;
  cp.set_description("This programs measures construction time and LCE query "
                     "time for several LCE data structures.");
  cp.set_author("Alexander Herlez <alexander.herlez@tu-dortmund.de>\n"
                "        Florian Kurpicz  <florian.kurpicz@tu-dortmund.de>");

  cp.add_param_string("file", lce_bench.file, "The for from which the LCE "
                      "data structures are build.");
  cp.add_string('o', "output_path", lce_bench.output_path, "Path where result "
                "file are saved here (optional).");
  cp.add_bytes('p', "pre", lce_bench.prefix_length, "Size of the prefix in "
               "bytes that will be read (optional).");
  cp.add_string('a', "algorithm", lce_bench.algorithm, "LCP data structure "
                "that is computed: [u]ltra naive, [n]aive, prezza [m]ersenne, "
                "[p]rezza, or [s]tring synchronizing sets.");
  cp.add_string('M', "mode", lce_bench.mode, "Test mode: [r]andom, [c]omplete, "
                "or [s]orted.");
  cp.add_uint("from", lce_bench.lce_from, "If mode: sorted, use only lce "
              "queries which return at least 2^{from} (optinal).");
  cp.add_uint("to", lce_bench.lce_to, "If mode: sorted, use only lce queries "
              "which return less than 2^{from} with from < 22 (optional)");

  if (!cp.process(argc, argv)) {
    std::exit(EXIT_FAILURE);
  }

  lce_bench.run();
  return 0;
}

/******************************************************************************/
