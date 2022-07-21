#pragma once

#include "binsearch_cache.hpp"
#include <pgm/pgm_index.hpp>

namespace stash {
namespace pred {

// a wrapper around the PGM index for successor queries
template<typename array_t, typename item_t, size_t m_epsilon, size_t m_cache_num = 512ULL / sizeof(item_t)>
class pgm_index : public binsearch_cache<array_t, item_t, m_cache_num> {
private:
    using base_t = binsearch_cache<array_t, item_t, m_cache_num>;

    using base_t::m_min;
    using base_t::m_max;
    using base_t::m_num;

    pgm::PGMIndex<item_t, m_epsilon> m_pgm;

public:
    inline pgm_index() : base_t() {
    }

    pgm_index(pgm_index const&) = default;
    pgm_index& operator=(pgm_index const&) = default;
    pgm_index(pgm_index&&) = default;
    pgm_index& operator=(pgm_index&&) = default;

    inline pgm_index(const array_t& array) : base_t(array), m_pgm(array.data(), array.data() + array.size() - 1) {
    }

    // finds the greatest element less than OR equal to x
    inline result predecessor(const item_t x) const {
      if(unlikely(x < m_min))  return result { false, 0 };
      if(unlikely(x >= m_max)) return result { true, m_num-1 };

      auto range = m_pgm.search(x);

      // nb: the PGM index returns the interval that would contain x if it were contained
      // the predecessor and successor may thus be the items just outside the interval!
      if(range.lo) --range.lo;
      if(range.hi+1) ++range.hi;

      return base_t::predecessor_seeded(x, range.lo, range.hi);
    }

    // finds the smallest element greater than OR equal to x
    inline result successor(const item_t x) const {
      if(unlikely(x <= m_min)) return result { true, 0 };
      if(unlikely(x > m_max))  return result { false, 0 };

      auto range = m_pgm.search(x);

      // nb: the PGM index returns the interval that would contain x if it were contained
      // the predecessor and successor may thus be the items just outside the interval!
      if(range.lo) --range.lo;
      if(range.hi+1) ++range.hi;

      return base_t::successor_seeded(x, range.lo, range.hi);
    }
};

}}
