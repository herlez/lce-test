# Space Efficient Data Structures for Longest Common Extensions

The _longest common extension_ (LCE) of two text positions in a text ![T](https://render.githubusercontent.com/render/math?math=%5Ctextstyle+T) is the maximum size of a common substring starting at those positions, i.e., more formally ![lce_T:=\max\{\ell\geq 0 \colon T[i,i+\ell)=T[j,j+\ell)\}](https://render.githubusercontent.com/render/math?math=%5Ctextstyle+lce_T%3A%3D%5Cmax%5C%7B%5Cell%5Cgeq+0+%5Ccolon+T%5Bi%2Ci%2B%5Cell%29%3DT%5Bj%2Cj%2B%5Cell%29%5C%7D).

This repository contains different implementations to compute LCEs using different techniques:

1. [simply scanning the text naively](lce-test/lce_naive_ultra.hpp)
2. [simply scanning the text naively (but using 128 bit integers to pack characters)](lce-test/lce_naive.hpp)
3. [computing in-place fingerprints](lce-test/lce_prezza.hpp)
4. [using string synchronizing sets](lce-test/lce_semi_synchroniing_sets.hpp)

For easier comparison, we also provide the different implementations ([based on in-place fingerprints](lce-test/lce_prezza_mersenne.hpp) and [using data structures contained in the SDSL](lce-test/lce_sdsl_cst.hpp)).

## How to Build the Code

First, we clone the code and submodules:
```
git clone https://github.com/herlez/lce-test.git
cd lce-test
git submodule update --init --recursive
```
Then we build the code:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```
If we want detailed information (timings and memory requirements) of the string synchronizing set LCE data structure, we have to use ``-DDETAILED_TIME=True``.
Note that this options invalidates all other times and memory measurements for this data structure.
