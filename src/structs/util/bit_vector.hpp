#pragma once

#include <vector>
#include <cstdint> //uint64_t

class bit_vector {
private:
    size_t m_size;
    std::vector<uint64_t> m_bv;

public:
    inline bit_vector(size_t size) : m_size(size) {
        /* BVCC - Allokation */
        m_bv = std::vector<uint64_t>((size/64)+1);
    }

    inline void bitset(size_t i, bool b) {
        /* BVCC - Bit an Position i auf b setzen */
        //m_bv[i/64] ^= (-b ^ m_bv[i/64]) & (1UL << (i%64));
        if(b) {
			m_bv[i/64] |= (1ULL << i%64);
		} else {
			m_bv[i/64] &= (~(1ULL << i%64));
		}
    }

    inline bool bitread(size_t i) const {
        /* BVCC - Bit aus Position i lesen */
        return (m_bv[i/64] >> (i%64)) & 1U;
    }


	inline uint64_t blockread(size_t i) const {
		return m_bv[i];
	}
    inline size_t size() const {
        return m_size;
    }
};
