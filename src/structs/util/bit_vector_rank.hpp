#pragma once

#include "bit_vector.hpp"

class bit_vector_rank {
private:
    const bit_vector* m_bv;

    std::vector<size_t> m_rank1; /* BVCC - Ersetzen! */

public:
    inline bit_vector_rank(const bit_vector& bv) : m_bv(&bv) {
        /* BVCC - Konstruktion */
        const size_t n = m_bv->size();
         
        m_rank1 = std::vector<size_t>(1+n/(64*8));
        m_rank1[0] = 0;
        
        size_t r2 = 0;
        for(size_t i = 1; i < m_rank1.size(); ++i) {
			for(size_t j = 0; j < 8; ++j) {
				r2 += __builtin_popcountll(m_bv->blockread(8*(i-1)+j));
			}
			m_rank1[i] = r2;
		}
    }

     inline size_t rank1(size_t i) const {
		size_t popped = 0;
		size_t j = ((i+1) % 512) / 64;
		for(size_t k = 0; k < j; ++k)  {
			popped += __builtin_popcountll(m_bv->blockread(i/512*8+k));
		}
		//std::cout << "rank  : " << i << std::endl;
		//std::cout << "popped: " << popped << std::endl;
		//std::cout << "m_rank1:" << m_rank1[(i+1)/128*2] << std::endl;
		//std::cout << "correct:" << m_rank1[(i+1)/64] << std::endl;
        return m_rank1[(i+1)/512] + popped + __builtin_popcountll(m_bv->blockread(i/64) << 1 << (62 - i%64)%64);
    }

    inline size_t rank0(size_t i) const {
        return 1 + i - rank1(i);
    }
};
