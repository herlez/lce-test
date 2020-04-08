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

using namespace sdsl;
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Usage: " << argv[0] << " file " << endl;
        return 1;
    }
    cst_sct3<> cst;
    construct(cst, argv[1], 1);

    return 0;
}

/******************************************************************************/
