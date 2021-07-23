#pragma once
    
#include <vector>
#include <utility>

#include "util.hpp"

namespace stash {

class int_vector {
private:
    size_t m_size;
    size_t m_width;
    size_t m_mask;
    std::vector<uint64_t> m_data;

    inline void set(size_t i, uint64_t v) {
        v &= m_mask; // make sure it fits...
        
        const size_t j = i * m_width;
        const size_t a = j >> 6ULL;       // left border
        const size_t b = (j + m_width - 1ULL) >> 6ULL; // right border
        if(a < b) {
            // the bits are the suffix of m_data[a] and prefix of m_data[b]
            const size_t da = j & 63ULL;
            const size_t wa = 64ULL - da;
            const size_t wb = m_width - wa;
            //const size_t db = 64ULL - wb;

            // combine the da lowest bits from a and the wa lowest bits of v
            const uint64_t a_lo = m_data[a] & bit_mask(da);
            const uint64_t v_lo = v & bit_mask(wa);
            m_data[a] = (v_lo << da) | a_lo;

            // combine the db highest bits of b and the wb highest bits of v
            const uint64_t b_hi = m_data[b] >> wb;
            const uint64_t v_hi = v >> wa;
            m_data[b] = (b_hi << wb) | v_hi;
        } else {
            const size_t dl = j & 63ULL;
            const size_t dvl = dl + m_width;
            
            const uint64_t xa = m_data[a];
            const uint64_t lo = xa & bit_mask(dl);
            const uint64_t hi = xa >> dvl;
            m_data[a] = lo | (v << dl) | (hi << dvl);
        }
    }

    inline uint64_t get(size_t i) const {
        const size_t j = i * m_width;
        const size_t a = j >> 6ULL;                    // left border
        const size_t b = (j + m_width - 1ULL) >> 6ULL; // right border

        // da is the distance of a's relevant bits from the left border
        const size_t da = j & 63ULL;

        // wa is the number of a's relevant bits
        const size_t wa = 64ULL - da;

        // get the wa highest bits from a
        const uint64_t a_hi = m_data[a] >> da;

        // get b (its high bits will be masked away below)
        // NOTE: we could save this step if we knew a == b,
        //       but the branch caused by checking that is too expensive
        const uint64_t b_lo = m_data[b];

        // combine
        return ((b_lo << wa) | a_hi) & m_mask;
    }

public:
    struct Ref {
        int_vector* iv;
        size_t i;

        inline operator uint64_t() const {
            return iv->get(i);
        }

        inline void operator=(uint64_t v) {
            iv->set(i, v);
        }
    };

    inline int_vector() : m_size(0), m_width(0), m_mask(0) {
    }

    inline int_vector(const int_vector& other) {
        *this = other;
    }

    inline int_vector(int_vector&& other) {
        *this = std::move(other);
    }

    inline int_vector(size_t size, size_t width) {
        resize(size, width);
    }

    inline int_vector& operator=(const int_vector& other) {
        m_size = other.m_size;
        m_width = other.m_width;
        m_mask = other.m_mask;
        m_data = other.m_data;
        return *this;
    }

    inline int_vector& operator=(int_vector&& other) {
        m_size = other.m_size;
        m_width = other.m_width;
        m_mask = other.m_mask;
        m_data = std::move(other.m_data);
        return *this;
    }

    inline void resize(size_t size, size_t width) {
        m_size = size;
        m_width = width;
        m_mask = bit_mask(width);
        
        const size_t bits = m_size * m_width;
        const size_t q = bits >> 6ULL; // divide by 64
        const size_t k = bits & 63ULL; // mod 64
        m_data.resize(k ? q + 1 : q);
    }

    inline void rebuild(size_t size, size_t width) {
        int_vector new_iv(size, width);
        for(size_t i = 0; i < size; i++) {
            new_iv.set(i, get(i));
        }
        
        m_size = new_iv.m_size;
        m_width = new_iv.m_width;
        m_mask = new_iv.m_mask;
        m_data = std::move(new_iv.m_data);
    }

    inline void rebuild(size_t size) {
        rebuild(size, m_width);
    }

    inline uint64_t operator[](size_t i) const {
        return get(i);
    }

    inline Ref operator[](size_t i) {
        return Ref { this, i };
    }

    inline size_t size() const {
        return m_size;
    }
};

}
