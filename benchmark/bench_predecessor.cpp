/*******************************************************************************
 * benchmark/bench_predecessor.cpp
 *
 * Copyright (C) 2022 Patrick Dinklage <patrick.dinklage@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <cstdlib>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include <tlx/cmdline_parser.hpp>

#include <malloc_count.h>

#include "util/successor/binsearch.hpp"
#include "util/successor/binsearch_cache.hpp"
#include "util/successor/index.hpp"
#include "util/successor/pgm_index.hpp"
#include "util/successor/rank.hpp"

#ifdef ALLOW_PARALLEL
#include "util/successor/index_par.hpp"
#endif

uint64_t time() {
    using namespace std::chrono;
    return uint64_t(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

template<typename O>
std::vector<O> load_file_lines_as_vector(const std::string& filename) {
    std::vector<O> v;
    std::ifstream f(filename);

    // 2^64 has 20 decimal digits, so a buffer of size 24 should be safe
    for(std::array<char, 24> linebuf; f.getline(&linebuf[0], 24);) {
        if(linebuf[0]) {
            v.push_back(O(uint64_t(std::atoll(&linebuf[0]))));
        }
    }

    return v;
}

using namespace stash;

using value_t = uint64_t;
using binsearch       = pred::binsearch<std::vector<value_t>, value_t>;
using binsearch_cache = pred::binsearch_cache<std::vector<value_t>, value_t>;
using rank            = pred::rank<std::vector<value_t>, value_t>;

template<size_t k>
using index = pred::index<std::vector<value_t>, value_t, k>;

#ifdef ALLOW_PARALLEL
template<size_t k>
using index_par = pred::index_par<std::vector<value_t>, value_t, k>;
#endif

template<size_t epsilon>
using pgm_index = pred::pgm_index<std::vector<value_t>, value_t, epsilon>;

std::vector<value_t> generate_queries(size_t num, size_t universe, size_t seed = 147ULL) {
    std::vector<value_t> queries;
    queries.reserve(num);

    // seed
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<uint64_t> dist(0, universe);

    // generate
    for(size_t i = 0; i < num; i++) {
        queries.push_back(value_t(dist(gen)));
    }

    return queries;
}

std::string test_types[] = { "predecessor", "successor" };

struct test_result {
    uint8_t  type;
    uint64_t t_construct;
    size_t   m_ds;
    uint64_t t_queries;
    uint64_t sum;
};

template<typename pred_t>
test_result test_predecessor(
    const std::vector<value_t>& array,
    const std::vector<value_t>& queries) {

    // construct
    auto t0 = time();
    auto m0 = malloc_count_current();
    pred_t q(array);
    uint64_t t_construct = time() - t0;
    size_t   m_ds = malloc_count_current() - m0;

    // do queries
    const auto min = array[0];
    uint64_t t_queries;
    uint64_t sum = 0;
    {
        auto t0 = time();
        for(value_t x : queries) {
            auto r = q.predecessor(x);

            if(x >= min) {
                assert(r.exists);
                assert(x >= array[r.pos]);
                sum += array[r.pos];
            } else {
                assert(!r.exists);
            }
        }
        t_queries = time() - t0;
    }

    return test_result { 0, t_construct, m_ds, t_queries, sum };
}

template<typename pred_t>
test_result test_successor(
    const std::vector<value_t>& array,
    const std::vector<value_t>& queries) {

    // construct
    auto t0 = time();
    auto m0 = malloc_count_current();
    pred_t q(array);
    uint64_t t_construct = time() - t0;
    size_t   m_ds = malloc_count_current() - m0;

    // do queries
    const auto max = array[array.size()-1ULL];
    uint64_t t_queries;
    uint64_t sum = 0;
    {
        auto t0 = time();
        for(value_t x : queries) {
            auto r = q.successor(x);
            if(x <= max) {
                assert(r.exists);
                assert(x <= array[r.pos]);
                sum += array[r.pos];
            } else {
                assert(!r.exists);
            }
        }
        t_queries = time() - t0;
    }

    return test_result { 1, t_construct, m_ds, t_queries, sum };
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;

    std::string input_filename;
    cp.add_param_string("file", input_filename, "The input file, containing a string representation of a number per line.");

    size_t num_queries = 10'000'000ULL;
    cp.add_bytes('q', "queries", num_queries, "The number of queries to perform.");

    size_t universe = 0;
    cp.add_bytes('u', "universe", universe, "The universe to draw query numbers from. Default is maximum value + 1.");

    bool add_max = false;
    cp.add_flag('m', "max", add_max, "Append the maximum possible value to the input sequence.");

    bool no_pred = false;
    cp.add_bool("no-pred", no_pred, "Don't do predecessor benchmark.");

    bool no_succ = false;
    cp.add_bool("no-succ", no_succ, "Don't do successor benchmark.");

    if (!cp.process(argc, argv)) {
        return -1;
    }
    
    #ifdef ALLOW_PARALLEL
    std::cout << "# benchmarking PARALLEL construction" << std::endl;
    #else
    std::cout << "# benchmarking SEQUENTIAL construction" << std::endl;
    #endif

    // load input
    std::cout << "# loading input: " << input_filename << std::endl;

    auto array = load_file_lines_as_vector<value_t>(input_filename);
    if(!universe) {
        universe = (size_t)array[array.size() - 1] + 1;
    }

    if(add_max) {
        array.push_back(value_t(UINT64_MAX));
    }

    // generate queries
    std::cout << "# generating queries ..." << std::endl;
    auto queries = generate_queries(num_queries, universe);

    // lambda to print a test result
    auto print_result = [&](const std::string& name, test_result&& r){
        std::cout << "RESULT algo="<< name
            << " queries=" << queries.size()
            << " type=" << test_types[r.type]
            << " universe=" << universe
            << " keys=" << array.size()
            << " t_construct=" << r.t_construct
            << " t_queries=" << r.t_queries
            << " m_ds=" << r.m_ds
            << " sum=" << r.sum << std::endl;
    };

    // run tests
    if(!no_pred) {
    std::cout << "# predecessor ..." << std::endl;
    
    #ifdef ALLOW_PARALLEL
    print_result("idx<4>", test_predecessor<index_par<4>>(array, queries));
    print_result("idx<5>", test_predecessor<index_par<5>>(array, queries));
    print_result("idx<6>", test_predecessor<index_par<6>>(array, queries));
    print_result("idx<7>", test_predecessor<index_par<7>>(array, queries));
    print_result("idx<8>", test_predecessor<index_par<8>>(array, queries));
    print_result("idx<9>", test_predecessor<index_par<9>>(array, queries));
    print_result("idx<10>", test_predecessor<index_par<10>>(array, queries));
    print_result("idx<11>", test_predecessor<index_par<11>>(array, queries));
    print_result("idx<12>", test_predecessor<index_par<12>>(array, queries));
    print_result("idx<13>", test_predecessor<index_par<13>>(array, queries));
    print_result("idx<14>", test_predecessor<index_par<14>>(array, queries));
    print_result("idx<15>", test_predecessor<index_par<15>>(array, queries));
    print_result("idx<16>", test_predecessor<index_par<16>>(array, queries));
    #else
    print_result("bs", test_predecessor<binsearch>(array, queries));
    print_result("bs*", test_predecessor<binsearch_cache>(array, queries));
    print_result("rank", test_predecessor<rank>(array, queries));
    print_result("idx<4>", test_predecessor<index<4>>(array, queries));
    print_result("idx<5>", test_predecessor<index<5>>(array, queries));
    print_result("idx<6>", test_predecessor<index<6>>(array, queries));
    print_result("idx<7>", test_predecessor<index<7>>(array, queries));
    print_result("idx<8>", test_predecessor<index<8>>(array, queries));
    print_result("idx<9>", test_predecessor<index<9>>(array, queries));
    print_result("idx<10>", test_predecessor<index<10>>(array, queries));
    print_result("idx<11>", test_predecessor<index<11>>(array, queries));
    print_result("idx<12>", test_predecessor<index<12>>(array, queries));
    print_result("idx<13>", test_predecessor<index<13>>(array, queries));
    print_result("idx<14>", test_predecessor<index<14>>(array, queries));
    print_result("idx<15>", test_predecessor<index<15>>(array, queries));
    print_result("idx<16>", test_predecessor<index<16>>(array, queries));
    #endif
    
    print_result("pgm<4>", test_predecessor<pgm_index<4>>(array, queries));
    print_result("pgm<8>", test_predecessor<pgm_index<8>>(array, queries));
    print_result("pgm<12>", test_predecessor<pgm_index<12>>(array, queries));
    print_result("pgm<16>", test_predecessor<pgm_index<16>>(array, queries));
    print_result("pgm<20>", test_predecessor<pgm_index<20>>(array, queries));
    print_result("pgm<24>", test_predecessor<pgm_index<24>>(array, queries));
    print_result("pgm<32>", test_predecessor<pgm_index<32>>(array, queries));
    }

    if(!no_succ) {
    std::cout << "# successor ..." << std::endl;
    
    #ifdef ALLOW_PARALLEL
    print_result("idx<4>", test_successor<index_par<4>>(array, queries));
    print_result("idx<5>", test_successor<index_par<5>>(array, queries));
    print_result("idx<6>", test_successor<index_par<6>>(array, queries));
    print_result("idx<7>", test_successor<index_par<7>>(array, queries));
    print_result("idx<8>", test_successor<index_par<8>>(array, queries));
    print_result("idx<9>", test_successor<index_par<9>>(array, queries));
    print_result("idx<10>", test_successor<index_par<10>>(array, queries));
    print_result("idx<11>", test_successor<index_par<11>>(array, queries));
    print_result("idx<12>", test_successor<index_par<12>>(array, queries));
    print_result("idx<13>", test_successor<index_par<13>>(array, queries));
    print_result("idx<14>", test_successor<index_par<14>>(array, queries));
    print_result("idx<15>", test_successor<index_par<15>>(array, queries));
    print_result("idx<16>", test_successor<index_par<16>>(array, queries));
    #else
    print_result("bs", test_successor<binsearch>(array, queries));
    print_result("bs*", test_successor<binsearch_cache>(array, queries));
    print_result("rank", test_successor<rank>(array, queries));
    print_result("idx<4>", test_successor<index<4>>(array, queries));
    print_result("idx<5>", test_successor<index<5>>(array, queries));
    print_result("idx<6>", test_successor<index<6>>(array, queries));
    print_result("idx<7>", test_successor<index<7>>(array, queries));
    print_result("idx<8>", test_successor<index<8>>(array, queries));
    print_result("idx<9>", test_successor<index<9>>(array, queries));
    print_result("idx<10>", test_successor<index<10>>(array, queries));
    print_result("idx<11>", test_successor<index<11>>(array, queries));
    print_result("idx<12>", test_successor<index<12>>(array, queries));
    print_result("idx<13>", test_successor<index<13>>(array, queries));
    print_result("idx<14>", test_successor<index<14>>(array, queries));
    print_result("idx<15>", test_successor<index<15>>(array, queries));
    print_result("idx<16>", test_successor<index<16>>(array, queries));
    #endif
    
    print_result("pgm<4>", test_successor<pgm_index<4>>(array, queries));
    print_result("pgm<8>", test_successor<pgm_index<8>>(array, queries));
    print_result("pgm<12>", test_successor<pgm_index<12>>(array, queries));
    print_result("pgm<16>", test_successor<pgm_index<16>>(array, queries));
    print_result("pgm<20>", test_successor<pgm_index<20>>(array, queries));
    print_result("pgm<24>", test_successor<pgm_index<24>>(array, queries));
    print_result("pgm<32>", test_successor<pgm_index<32>>(array, queries));
    }
}
