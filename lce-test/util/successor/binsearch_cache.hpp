#pragma once

#include "helpers/util.hpp"
#include "result.hpp"

namespace stash {
namespace pred {

// the "bs*" data structure for successor queries
template<typename array_t, typename item_t, size_t m_cache_num = 512ULL / sizeof(item_t)>
class binsearch_cache {
protected:
    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

public:
    inline binsearch_cache() : m_array(nullptr), m_num(0), m_min(), m_max() {
    }

    inline binsearch_cache(binsearch_cache&& other) {
        *this = other;
    }

    inline binsearch_cache(const binsearch_cache& other) {
        *this = other;
    }

    inline binsearch_cache(const array_t& array)
        : m_array(&array),
          m_num(array.size()),
          m_min(array[0]),
          m_max(array[m_num-1]) {

        assert_sorted_ascending(array);
    }

    inline binsearch_cache& operator=(binsearch_cache&& other) {
        m_array = other.m_array;
        m_num = other.m_num;
        m_min = other.m_min;
        m_max = other.m_max;
        return *this;
    }

    inline binsearch_cache& operator=(const binsearch_cache& other) {
        m_array = other.m_array;
        m_num = other.m_num;
        m_min = other.m_min;
        m_max = other.m_max;
        return *this;
    }

    // finds the smallest element greater than OR equal to x
    // seeded using a start interval
    inline result predecessor_seeded(const item_t x, size_t p, size_t q) const {
        assert(x >= m_min && x < m_max);
        while(q - p > m_cache_num) {
            assert(x >= (*m_array)[p]);

            const size_t m = (p + q) >> 1ULL;

            const bool le = ((*m_array)[m] <= x);

            /*
                the following is a fast form of:
                if(le) p = m; else q = m;
            */
            const size_t le_mask = -size_t(le);
            const size_t gt_mask = ~le_mask;

            p = (le_mask & m) | (gt_mask & p);
            q = (gt_mask & m) | (le_mask & q);
        }

        // linear search
        while((*m_array)[p] <= x) ++p;
        assert((*m_array)[p-1] <= x);

        return result { true, p-1 };
    }

    // finds the greatest element less than OR equal to x
    inline result predecessor(const item_t x) const {
        if(unlikely(x < m_min))  return result { false, 0 };
        if(unlikely(x >= m_max)) return result { true, m_num-1 };
        return predecessor_seeded(x, 0, m_num - 1);
    }

    // finds the smallest element greater than OR equal to x
    // seeded using a start interval
    inline result successor_seeded(const item_t x, size_t p, size_t q) const {
        assert(x >= m_min && x < m_max);
        while(q - p > m_cache_num) {
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

        // linear search
        while((*m_array)[p] < x) ++p;
        assert((*m_array)[p] >= x);

        return result { true, p };
    }

    // finds the smallest element greater than OR equal to x
    inline result successor(const item_t x) const {
        if(unlikely(x <= m_min)) return result { true, 0 };
        if(unlikely(x > m_max))  return result { false, 0 };
        return successor_seeded(x, 0, m_num - 1);
    }
};

}}
