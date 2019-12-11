#pragma once
#include <cstdint>
#include <vector>
#include <cmath>

#include <iostream> 

class Rmq {
private:
	const size_t size;
	std::vector<std::vector<uint64_t>> table;
	
	public:
	Rmq(std::vector<uint64_t> &v) : size(v.size()) {
		
		const unsigned int numberOfLevels = log2(size) + 1;
		table.resize(numberOfLevels);
		table[0].resize(size);
		// Fill first row of rmq table
		for(uint64_t j = 0; j < size; ++j) {
			table[0][j] = v[j];
			}

		// Calculate the rest of the rmq table
		for(unsigned int i = 1; i < numberOfLevels; ++i) {
			table[i].resize(size - pow(2, i)+1);
			for(uint64_t j = 0; j < size - pow(2, i)+1; ++j) {
				table[i][j] = std::min(table[i-1][j], table[i-1][j+pow(2, i-1)]);
			}
		}
	}
	
	// invariant: i <= j
	uint64_t rmq(uint64_t i, uint64_t j) const {
		uint64_t range = j-i + 1;
		uint64_t range_log2 = log2(range);
		
		//std::cout << "i: " << i << "  j: " << j << std::endl;
		//std::cout << range << "  " << range_log2 << std::endl;
		return std::min(table[range_log2][i], table[range_log2][j+1-pow(2, range_log2)]);
	}
	
	size_t getSize() const {
		return size;
	}
	
	void print_table() {
		for(uint64_t i = 0; i < table.size(); ++i) {
			for(uint64_t j = 0; j < table[i].size(); ++j) {
				std::cout << table[i][j] << " ";
			}
			std::cout << std::endl;
		}
	}
};
