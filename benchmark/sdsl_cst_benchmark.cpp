/*******************************************************************************
 * benchmark/sdsl_cst_bechmark.cpp
 *
 * Copyright (C) 2020 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/


#include <sdsl/suffix_trees.hpp>
#include <iostream>
#include <string>

#include "timer.hpp"

using namespace sdsl;
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " file " << endl;
        return 1;
    }
    cst_sct3<> cst;

    timer t;
    construct(cst, argv[1], 1);
    auto const construction_time = t.get_and_reset();

    std::cout << "RESULT algo=cst_sct3 time=" << construction_time << " "
              << "input=" << argv[1] << std::endl;

    return 0;
}
/******************************************************************************/
