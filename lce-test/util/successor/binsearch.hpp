#pragma once

#include "helpers/util.hpp"
#include "result.hpp"

namespace stash {
namespace pred {

// the "bs" data structure for successor queries
template<typename array_t, typename item_t>
class binsearch {
private:
    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

public:
    inline binsearch() : m_array(nullptr), m_num(0), m_min(), m_max() {
    }

    inline binsearch(binsearch&& other) {
        *this = other;
    }

    inline binsearch(const binsearch& other) {
        *this = other;
    }

    inline binsearch(const array_t& array)
        : m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]),
          m_array(&array) {

        assert_sorted_ascending(array);
    }

    inline binsearch& operator=(binsearch&& other) {
        m_array = other.m_array;
        m_num = other.m_num;
        m_min = other.m_min;
        m_max = other.m_max;
        return *this;
    }

    inline binsearch& operator=(const binsearch& other) {
        m_array = other.m_array;
        m_num = other.m_num;
        m_min = other.m_min;
        m_max = other.m_max;
        return *this;
    }

    // finds the greatest element less than OR equal to x
    inline result predecessor(const item_t x) const {
        if(unlikely(x < m_min))  return result { false, 0 };
        if(unlikely(x >= m_max)) return result { true, m_num-1 };

        size_t p = 0;
        size_t q = m_num - 1;

        while(p < q - 1) {
            assert(x >= (*m_array)[p]);
            assert(x < (*m_array)[q]);

            const size_t m = (p + q) >> 1ULL;

            const bool le = ((*m_array)[m] <= x);

            /*
                the following is a fast form of:
                if(le) p = m; else q = m;
            */
            const size_t le_mask = -size_t(le);
            const size_t gt_mask = ~le_mask;

            if(le) assert(le_mask == SIZE_MAX && gt_mask == 0ULL);
            else   assert(gt_mask == SIZE_MAX && le_mask == 0ULL);

            p = (le_mask & m) | (gt_mask & p);
            q = (gt_mask & m) | (le_mask & q);
        }

        return result { true, p };
    }

    // finds the smallest element greater than OR equal to x
    inline result successor(const item_t x) const {
        if(unlikely(x <= m_min)) return result { true, 0 };
        if(unlikely(x > m_max))  return result { false, 0 };

        size_t p = 0;
        size_t q = m_num - 1;

        while(p < q - 1) {
            assert(x > (*m_array)[p]);
            assert(x <= (*m_array)[q]);

            const size_t m = (p + q) >> 1ULL;

            const bool lt = ((*m_array)[m] < x);

            /*
                the following is a fast form of:
                if(lt) p = m; else q = m;
            */
            const size_t lt_mask = -size_t(lt);
            const size_t ge_mask = ~lt_mask;

            p = (lt_mask & m) | (ge_mask & p);
            q = (ge_mask & m) | (lt_mask & q);
        }

        return result { true, q };
    }
};

}}
