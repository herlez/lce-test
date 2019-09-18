#pragma once

#include <vector>
#include "bit_vector.hpp"
//#include <bit>


class bit_vector_rank {
private:
    const bit_vector* m_bv;

    std::vector<size_t> m_rank0, m_rank1; /* BVCC - Ersetzen! */

public:
    inline bit_vector_rank(const bit_vector& bv) : m_bv(&bv) {
        /* BVCC - Konstruktion */
        const size_t n = m_bv->size();
        m_rank0 = std::vector<size_t>(1+n/64);
        m_rank1 = std::vector<size_t>(1+n/64);
		m_rank0[0] = 0;
        m_rank1[0] = 0;
        
        
        size_t r0 = 0, r1 = 0;
        for(size_t i = 1; i < (1+n/64); ++i) {
            
            uint64_t pop = __builtin_popcountll(m_bv->blockread(i-1));
            r1 += pop;
            r0 += (64-pop);

            m_rank0[i] = r0;
            //cout << "rank1 " << i << ": " << m_rank1[i];
            m_rank1[i] = r1;
            //cout << "rank0 " << i << ": " << m_rank0[i];
        }
    }

    inline size_t rank1(size_t i) const {
        return m_rank1[i/64] + __builtin_popcountll(m_bv->blockread(i/64) << (64-1-i%64)); /* BVCC - rank1(i) */
    }

    inline size_t rank0(size_t i) const {
        return 1 + i - rank1(i);
        //return m_rank0[i/64] + 64-__builtin_popcountll(m_bv->blockread(i/64) << (64-i%64)); /* BVCC - rank0(i) */
    }
};
