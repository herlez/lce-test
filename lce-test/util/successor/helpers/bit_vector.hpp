#pragma once

#include <cstdint>
#include <vector>
#include <utility>

#include "util.hpp"

namespace stash {

class bit_vector {
private:
    size_t m_size;
    std::vector<uint64_t> m_bits;
    
    inline static size_t block(size_t i) {
        return i >> 6ULL; // divide by 64
    }

    inline static size_t offset(size_t i) {
        return i & 63ULL; // mod 64
    }

    inline bool bitread(size_t i) const {
        const size_t q = block(i);
        const size_t k = offset(i);
        const size_t mask = (1ULL << k);
        return bool(m_bits[q] & mask);
    }

    inline void bitset(size_t i, bool b) {
        const size_t q = block(i);
        const size_t k = offset(i);
        const size_t mask = (1ULL << k);

        // TODO: avoid branch!
        if(b) {
            m_bits[q] |= mask;
        } else {
            m_bits[q] &= ~mask;
        }
    }

public:
    struct Ref {
        bit_vector* bv;
        size_t i;
        
        inline operator bool() const {
            return bv->bitread(i);
        }

        inline void operator=(bool b) {
            bv->bitset(i, b);
        }
    };

    inline bit_vector() : m_size(0) {
    }

    inline bit_vector(const bit_vector& other) {
        *this = other;
    }

    inline bit_vector(bit_vector&& other) {
        *this = std::move(other);
    }

    inline bit_vector(size_t size) : m_size(size) {
        m_bits.resize(idiv_ceil(size, 64ULL));
    }

    inline bit_vector(const std::vector<bool>& bv) : bit_vector(bv.size()) {
        // FIXME: is there no faster way?
        for(size_t i = 0; i < m_size; i++) {
            bitset(i, bv[i]);
        }
    }

    inline bit_vector& operator=(const bit_vector& other) {
        m_size = other.m_size;
        m_bits = other.m_bits;
        return *this;
    }

    inline bit_vector& operator=(bit_vector&& other) {
        m_size = std::move(other.m_size);
        m_bits = std::move(other.m_bits);
        return *this;
    }

    inline uint64_t block64(size_t i) const {
        return m_bits[i];
    }

    inline size_t num_blocks() const {
        return m_bits.size();
    }

    inline bool operator[](size_t i) const {
        return bitread(i);
    }

    inline Ref operator[](size_t i) {
        return Ref { this, i };
    }

    inline size_t size() const {
        return m_size;
    }
};

}
