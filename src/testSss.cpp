#include <fstream>
#include <iostream> 
#include <cstdlib>
#include <inttypes.h>
#include <sys/time.h>
#include <ctime>
#include <iomanip>
#include <array>
#include <algorithm>
#include <vector>
#include "structs/lceNaive.hpp"
#include "structs/lcePrezza.hpp"
#include "structs/lcePrezzaMersenne.hpp"


using namespace std;

const string fileNames[] {"dna"};
const string files[] {"/scratch/text/dna"};
//const string files[] {"../../text/dna"};


const string lceSet[] = {"../res/lceDna/iH", "../res/lceDna/i10"};
const int NUMBEROFSETS = 2;
const int SETPAD = 0;



double timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

int main(int argc, char *argv[]) {
	
	// Results output
	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
    auto str = oss.str();
	ofstream log(string("../testResults/sss-") + str + string(".txt"), ios::out|ios::trunc);
	
	// Timestamps
	double ts1 = 0, ts2 = 0;
	
	
	for(unsigned int fileNumber = 0; fileNumber < 1; ++fileNumber) {
		/* Load LCE data structure */
		LceNaive dataN(files[0]);
		
		/* Load raw text */
		LcePrezza dataP(files[0]);
		
		/* Load Prezza's Mersenne prime data structure */
		rklce::LcePrezzaMersenne dataPM(files[0]);
		
		unsigned int NUMBEROFSTRUCTS = 3;
		LceDataStructure * lceData[NUMBEROFSTRUCTS] = {&dataN, &dataP, &dataPM};
		string structName[NUMBEROFSTRUCTS] {"naiveLCE", "prezzaLCE", "prezzaMersenneLCE"};
		
		
		
		for(unsigned int sufSet = 0; sufSet < NUMBEROFSETS; ++sufSet) {
			ts1 = ts2 = 0;
			for(uint64_t nSuffixes = (2 << 5); nSuffixes < (2 << 19); nSuffixes *= 2) {
				// Load stored suffixes
				vector<uint64_t> suf1;
				vector<uint64_t> suf2;
				suf1.resize(nSuffixes);
				suf2.resize(nSuffixes);
				
				
				ifstream lc(lceSet[0], ios::in);
				util::inputErrorHandling(&lc);
				string line;
				string::size_type sz;
				uint64_t l = 0;
				
				while(getline(lc, line) && l < nSuffixes) {
					suf1.push_back(stoi(line, &sz));
					suf2.push_back(stoi(line, &sz));
					l++;
				}
				
				//Sort, using std::sort
				for(unsigned int i = 0; i < NUMBEROFSTRUCTS; ++i) {
					if(ts2-ts1 < 30) {
						ts1 = timestamp();
						std::sort(suf1.begin(), suf1.end(), [&lceData]( const uint64_t lhs, const uint64_t rhs )
							{
								return lceData[0]->isSmallerSuffix(lhs, rhs);
							});
						ts2 = timestamp();
						log << "RESULT"
								<< " text=" << fileNames[fileNumber]
								<< " size=" << dataN.getSizeInBytes()
								<< " suffixes=" << nSuffixes
								<< " time=" << ts2-ts1
								<< " algo=" << structName[i]
								<< " set=" << lceSet[sufSet]
								<< endl;
					}
				}
			//validate sss
			}
		}
	}
	return EXIT_SUCCESS;
}



