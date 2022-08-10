#pragma once

#include <pgm/pgm_index.hpp>

namespace stash {
namespace pred {

// a wrapper around the PGM index for successor queries
template<typename array_t, typename item_t, size_t m_epsilon>
class pgm_index {
private:
    const array_t* m_array;
    size_t m_num;
    item_t m_min;
    item_t m_max;

    pgm::PGMIndex<item_t, m_epsilon> m_pgm;

public:
    inline pgm_index() {
    }

    pgm_index(pgm_index const&) = default;
    pgm_index& operator=(pgm_index const&) = default;
    pgm_index(pgm_index&&) = default;
    pgm_index& operator=(pgm_index&&) = default;

    inline pgm_index(const array_t& array) : m_array(&array), m_num(array.size()),
                                             m_min(array[0]),
                                             m_max(array[m_num-1]),
                                             m_pgm(array) {
    }

    // finds the greatest element less than OR equal to x
    inline result predecessor(const item_t x) const {
      if(unlikely(x < m_min))  return result { false, 0 };
      //if(unlikely(x >= m_max)) return result { true, m_num-1 };

      auto range = m_pgm.search(x);
      auto lo = m_array->data() + range.lo;
      auto hi = m_array->data() + range.hi;
      return {true, static_cast<size_t>(std::distance(m_array->data(), std::upper_bound(lo, hi, x)) - 1)};
      // nb: the PGM index returns the interval that would contain x if it were contained
      // the predecessor and successor may thus be the items just outside the interval!
      /*if(range.lo) --range.lo;
      if(range.hi+1) ++range.hi;

      return base_t::predecessor_seeded(x, range.lo, range.hi);*/
    }

    // finds the smallest element greater than OR equal to x
    inline result successor(const item_t x) const {
      //if(unlikely(x <= m_min)) return result { true, 0 };
      if(unlikely(x > m_max))  return result { false, 0 };

      auto range = m_pgm.search(x);
      auto lo = m_array->data() + range.lo;
      auto hi = m_array->data() + range.hi;
      return {true, static_cast<size_t>(std::distance(m_array->data(), std::lower_bound(lo, hi, x)))};

      // nb: the PGM index returns the interval that would contain x if it were contained
      // the predecessor and successor may thus be the items just outside the interval!
      /*if(range.lo) --range.lo;
      if(range.hi+1) ++range.hi;

      return base_t::successor_seeded(x, range.lo, range.hi);*/
    }
};

}}
