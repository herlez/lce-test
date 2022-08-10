#pragma once

#include <algorithm>
#include "result.hpp"

namespace stash {
namespace pred {

// predecessor/successor with std::lower_bound/std::upper/bound
template<
    typename array_t,
    typename item_t>
class binsearch_std {
private:
    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

public:
    inline binsearch_std(const array_t& array)
        : m_array(&array),
          m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]) {
    }

    // finds the greatest element less than OR equal to x
    inline result predecessor(const item_t x) const {
        if(unlikely(x < m_min))  return result { false, 0 };
        //if(unlikely(x >= m_max)) return result { true, m_num-1 };

        return {true, static_cast<size_t>(std::distance(m_array->data(), std::upper_bound(m_array->data(),  m_array->data() + m_num, x)) - 1)};

    }

    // finds the smallest element greater than OR equal to x
    inline result successor(const item_t x) const {
        //if(unlikely(x <= m_min)) return result { true, 0 };
        if(unlikely(x > m_max))  return result { false, 0 };

        return {true,  static_cast<size_t>(std::distance(m_array->data(), std::lower_bound(m_array->data(),  m_array->data() + m_num, x)))};
    }
};

}}
