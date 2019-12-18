/*******************************************************************************
 * benchmark/bench_time.cpp
 *
 * Copyright (C) 2019 Alexander Herlez <alexander.herlez@tu-dortmund.de>
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <malloc_count.h>

#include <fstream>
#include <sys/time.h>
#include <vector>
#include <iomanip>

#include <filesystem>

#include <memory>

#include <tlx/cmdline_parser.hpp>
#include <tlx/math/aggregate.hpp>

#include "io.hpp"
#include "timer.hpp"
#include "build_lce_ranges.hpp"
#include "lce_naive.hpp"
#include "lce_naive_ultra.hpp"
#include "lce_prezza.hpp"
#include "lce_prezza_mersenne.hpp"
#include "lce_semi_synchronizing_sets.hpp"

namespace fs = std::filesystem;

class lce_benchmark {

public:
  void run() {
    if (mode == "r") {
      random_ = true;
    } else if (mode == "s") {
      sorted_ = true;
    } else if (mode == "c") {
      complete_ = true;
    }
    
    fs::path input_path(file_path);
    std::string const filename = input_path.filename();
    
    string lce_path = output_path + filename;

    if (prefix_length > 0) {
      lce_path += "_" + std::to_string(prefix_length);
    }

    const array<string, 21> lce_set{lce_path + "/lce_0", lce_path + "/lce_1",
                                    lce_path + "/lce_2", lce_path + "/lce_3",
                                    lce_path + "/lce_4", lce_path + "/lce_5",
                                    lce_path + "/lce_6", lce_path + "/lce_7",
                                    lce_path + "/lce_8", lce_path + "/lce_9",
                                    lce_path + "/lce_10", lce_path + "/lce_11",
                                    lce_path + "/lce_12", lce_path + "/lce_13",
                                    lce_path + "/lce_14", lce_path + "/lce_15",
                                    lce_path + "/lce_16",lce_path + "/lce_17",
                                    lce_path + "/lce_18", lce_path + "/lce_19",
                                    lce_path + "/lce_X"};
    if(sorted_) {
      build_lce_range(file_path, output_path + filename, prefix_length);
    }

    /************************************
     ****PREPARE LCE DATA STRUCTURES*****
     ************************************/

    std::unique_ptr<LceDataStructure> lce_structure;
    std::vector<uint8_t> text;

    timer t;
    tlx::Aggregate<size_t> construction_times;
    tlx::Aggregate<size_t> construction_mem;

    std::cout << "RESULT "
              << "algo=" << print_algo_name() << " "
              << "runs=" << runs << " ";

    for (size_t i = 0; i < runs; ++i) {
      text = load_text(file_path, prefix_length);

      auto* old_structure = lce_structure.release();
      if (old_structure != nullptr) {
        delete old_structure;
      }
      if (algorithm == "u") {
        size_t const mem_before = malloc_count_current();
        t.reset();
        lce_structure = std::make_unique<LceUltraNaive>(text);
        construction_times.add(t.get_and_reset());
        construction_mem.add(malloc_count_current() - mem_before);
      } else if (algorithm == "n") {
        size_t const mem_before = malloc_count_current();
        t.reset();
        lce_structure = std::make_unique<LceNaive>(text);
        construction_times.add(t.get_and_reset());
        construction_mem.add(malloc_count_current() - mem_before);
      } else if (algorithm == "m") {
        t.reset();
        lce_structure = std::make_unique<rklce::LcePrezzaMersenne>(text);
        construction_times.add(t.get_and_reset());
      } else if (algorithm == "p") {
        // Make sure the text can be divided into 64 bit blocks
        text.resize(text.size() + (8 - (text.size() % 8)));
        size_t const mem_before = malloc_count_current();
        t.reset();
        lce_structure =
          std::make_unique<LcePrezza>(reinterpret_cast<uint64_t*>(text.data()),
                                      text.size());
        construction_times.add(t.get_and_reset());
        construction_mem.add(malloc_count_current() - mem_before);
      } else if (algorithm == "s") {
        size_t const mem_before = malloc_count_current();
        t.reset();
        lce_structure = std::make_unique<LceSemiSyncSets>(text);
        construction_times.add(t.get_and_reset());
        construction_mem.add(malloc_count_current() - mem_before);
      } else {
        return;
      }
    }

    std::cout << "construction_min_time=" << construction_times.min() << " "
              << "construction_max_time=" << construction_times.max() << " "
              << "construction_avg_time=" << construction_times.avg() << " "

              << "input=" << file_path << " "
              << "size=" << text.size() << " "

              << "lce_mem=" << construction_mem.max() << " ";

    std::vector<uint64_t> lce_indices(number_lce_queries * 2);
    tlx::Aggregate<size_t> queries_times;
    tlx::Aggregate<size_t> lce_values;

    std::cout << "lce_query_type=";
    if(random_) {
      std::cout << "random ";
      std::srand(std::time(nullptr));
      for(uint64_t i = 0; i < number_lce_queries * 2; ++i) {
        lce_indices[i] = rand() % lce_structure->getSizeInBytes();
      }
      for (size_t i = 0; i < runs; ++i) {
        t.reset();
        for (size_t j = 0; j < number_lce_queries * 2; j += 2) {
          size_t const lce = lce_structure->lce(lce_indices[j],
                                                lce_indices[j + 1]);
          lce_values.add(lce);
        }
        queries_times.add(t.get_and_reset());
      }
    } else if (sorted_) {
      std::cout << "sorted "
                << "from=" << lce_from << " "
                << "to=" << lce_to << " ";
      for (size_t i = lce_from; i < lce_to; ++i) {
        vector<uint64_t> v;
        std::ifstream lc(lce_set[i], ios::in);
        util::inputErrorHandling(&lc);
            
        string line;
        string::size_type sz;
        while(getline(lc, line)) {
          v.push_back(stoi(line, &sz));
        }
        lc.close();

        if (v.size() > 0) {
          for(uint64_t i = 0; i < number_lce_queries * 2; ++i) {
            lce_indices[i] = v[i % v.size()];
          }
          for (size_t i = 0; i < runs; ++i) {
            t.reset();
            for (size_t j = 0; j < number_lce_queries * 2; j += 2) {
              size_t const lce = lce_structure->lce(lce_indices[j],
                                                    lce_indices[j + 1]);
              lce_values.add(lce);
            }
            queries_times.add(t.get_and_reset());
          }
        }
      }
    } else {
      std::cout << "none ";
    }

    if (random_ || sorted_) {
      std::cout << "lce_values_min=" << lce_values.min() << " "
                << "lce_values_max=" << lce_values.max() << " "
                << "lce_values_avg=" << lce_values.avg() << " "
                << "lce_values_count=" << lce_values.count() << " "
                << "queries_times_min=" << queries_times.min() << " "
                << "queries_times_max=" << queries_times.max() << " "
                << "queries_times_avg=" << queries_times.avg() << " ";
    }
    std::cout << "check=";
    if (check) {
      // Create random queries for the test
      std::srand(std::time(nullptr));
      for(uint64_t i = 0; i < number_lce_queries * 2; ++i) {
        lce_indices[i] = rand() % lce_structure->getSizeInBytes();
      }
      std::vector<uint8_t> cmp_text = load_text(file_path, prefix_length);
      auto lce_naive = LceUltraNaive(cmp_text);
      bool correct = true;
      for (size_t i = 0; correct && i < number_lce_queries * 2; i += 2) {
        size_t const lce_res = lce_structure->lce(lce_indices[i],
                                                  lce_indices[i + 1]);
        size_t const lce_res_naive = lce_naive.lce(lce_indices[i],
                                                   lce_indices[i + 1]);
        correct = (lce_res == lce_res_naive);
      }
      if (!correct) {
        std::cout << "failed";
      } else {
        std::cout << "passed";
      }
    } else {
      std::cout << "none";
    }
    std::cout << std::endl;
  }


public:
  std::string file_path;
  std::string output_path = "/tmp/res_lce/";
  uint64_t prefix_length = 0;

  std::string algorithm = "u";

  bool check = false;

  std::string mode;

  size_t number_lce_queries = 1000000;
  uint32_t runs = 5;

  uint32_t lce_from = 0;
  uint32_t lce_to = 21;


private:
  std::string print_algo_name() {
    std::string name("unknown");
    if (algorithm == "u") {
      name = "ultra_naive";
    } else if (algorithm == "n") {
      name = "naive";
    } else if (algorithm == "m") {
      name = "prezza_mersenne";
    } else if (algorithm == "p") {
      name = "prezza";
    } else if (algorithm == "s") {
      name = "sss";
    }
    return name;
  }

private:
  bool sorted_ = false;
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

  cp.add_param_string("file", lce_bench.file_path, "The for from which the LCE "
                      "data structures are build.");
  cp.add_string('o', "output_path", lce_bench.output_path, "Path where LCE "
                "queries for [-m]ode [s]orted are stored "
                "(default: /tmp/res_lce).");
  cp.add_bytes('p', "pre", lce_bench.prefix_length, "Size of the prefix in "
               "bytes that will be read (optional).");
  cp.add_string('a', "algorithm", lce_bench.algorithm, "LCP data structure "
                "that is computed: [u]ltra naive (default), [n]aive, "
                "prezza [m]ersenne, [p]rezza, or [s]tring synchronizing sets.");
  cp.add_flag('c', "check", lce_bench.check, "Check correctness of LCE queries "
              "by comparing with results of naive computation.");
  cp.add_string('m', "mode", lce_bench.mode, "Test mode: [r]andom, [c]omplete, "
                "or [s]orted.");
  cp.add_size_t('q', "queries", lce_bench.number_lce_queries, "Number of LCE "
              "queries that are executed (default=1,000,000).");
  cp.add_uint('r', "runs", lce_bench.runs, "Number of runs that are used to "
              "report an average running time (default=5).");
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
