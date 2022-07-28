#pragma once

#include "helpers/bit_vector.hpp"
#include "helpers/bit_rank.hpp"
#include "helpers/bit_select.hpp"

#include "helpers/util.hpp"
#include "result.hpp"

namespace stash {
namespace pred {

// the "rank" data structure for successor queries
template<typename array_t, typename item_t>
class rank {
private:
    const array_t* m_array;
    size_t      m_num;
    item_t      m_min;
    item_t      m_max;
    bit_vector  m_bv;
    bit_rank    m_rank;

public:
    inline rank(const array_t& array)
        : m_array(&array),
          m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]) {

        assert_sorted_ascending(array);

        m_bv = bit_vector(m_max - m_min + 1);
        for(size_t i = 0; i < m_num; i++) {
            m_bv[array[i] - m_min] = 1;
        }

        assert(m_bv[0] == 1);
        assert(m_bv[m_max - m_min] == 1);

        m_rank = bit_rank(m_bv);
    }

    // finds the greatest element less than OR equal to x
    inline result predecessor(item_t x) const {
        if(unlikely(x < m_min))  return result { false, 0 };
        if(unlikely(x >= m_max)) return result { true, m_num-1 };

        const size_t p = m_rank(x - m_min);
        assert(p > 0);
        return result { true, p - 1 };
    }

    // finds the smallest element greater than OR equal to x
    inline result successor(item_t x) const {
        if(unlikely(x <= m_min))  return result { true, 0 };
        if(unlikely(x > m_max)) return result { false, 0 };

        const size_t p = m_rank(x - m_min);
        assert(p > 0);
        return result { true, p - 1 + (x > (*m_array)[p - 1]) };
    }
};

}}
