#include "./rmq.hpp"
#include <vector>
#include <algorithm> //std::sort
#include <string>

class Lce_rmq {
private:
	const uint64_t tau = 512;
	std::string * text;
	uint64_t text_size;
	std::vector<uint64_t> * sync_set;
	
	std::vector<uint64_t> isa;
	std::vector<uint64_t> lcp;
	Rmq * rmq_ds;
	
	uint64_t lce_in_text(uint64_t i, uint64_t j) {
		const uint64_t maxLce = text_size - (i > j ? i : j); 
		uint64_t lce_naive = 0;
		while (lce_naive < maxLce) {
			if (text->at(i) != text->at(j)) {
				return lce_naive;
			}
			++lce_naive;
		}
		return lce_naive;
	}
	
public:
	Lce_rmq(std::string * v_text, std::vector<uint64_t> * v_syncSet) 
							: text(v_text), sync_set(v_syncSet), text_size(v_text->size()) {
		//Construct SA
		std::vector<uint64_t> sa(sync_set->size());
		for(uint64_t i = 0; i < sa.size(); ++i) {
			sa[i] = i;
		}
		
		
		std::sort(sa.begin(), sa.end(), [=](uint64_t i, uint64_t j) {
						
						const uint64_t start_i = sync_set->at(i);
						const uint64_t start_j = sync_set->at(j);
						uint64_t max_lce = text_size - (start_i > start_j ? start_i : start_j);
						
						for(uint64_t k = 0; k < max_lce; ++k) {
							if (text->at(start_i + k) != text->at(start_j + k)) {
								return (text->at(start_i + k) < text->at(start_j + k));
							}
						}
						return i > j;
					});
		std::cout << "SA" << std::endl;
		for(uint64_t i = 0; i < sa.size(); ++i) {
			std::cout << sa[i] << ' ';
		} std::cout << std::endl;
		
		
		
		//Calculate ISA
		isa.resize(sa.size());
		for(uint64_t i = 0; i < sa.size(); ++i) {
			isa[sa[i]] = i;
		}
		std::cout << "ISA" << std::endl;
		for(uint64_t i = 0; i < isa.size(); ++i) {
			std::cout << isa[i] << ' ';
		} std::cout << std::endl;
		
		
		
		//Calculate LCP array
		lcp.resize(sa.size());
		for(uint64_t i = 1; i < sa.size(); ++i) {
			lcp[i] = lce_in_text(sync_set->at(sa[i-1]), sync_set->at(sa[i]));
		}
		std::cout << "LCP" << std::endl;
		for(uint64_t i = 0; i < lcp.size(); ++i) {
			std::cout << lcp[i] << ' ';
		} std::cout << std::endl;
		
		//Build RMQ data structure
		rmq_ds = new Rmq(lcp);
		std::cout << "RMQ" << std::endl;
	}
	
	
	uint64_t lce(uint64_t i, uint64_t j) {
		if(i == j) {
			return text_size - i;
		}
		
		if(isa[i] < isa[j]) {
			return rmq_ds->rmq(isa[i]+1, isa[j]);
		} else {
			return rmq_ds->rmq(isa[i], isa[j]+1);
		}
	}
	
	uint64_t get_size() {
		return text_size;
	}
};
