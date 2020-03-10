#pragma once

#include <utility>

#include "util.hpp"
#include "bit_vector.hpp"
#include "int_vector.hpp"

namespace stash {

class bit_rank {
private:
    static constexpr size_t SUP_SZ = 4096;
    static constexpr size_t SUP_MSK = 4095;
    static constexpr size_t SUP_W = 12;

    static constexpr size_t BLOCKS_PER_SB = SUP_SZ >> 6ULL;
    static constexpr size_t SB_INNER_RS = SUP_W - 6ULL;
        
    const bit_vector* m_bv;

    int_vector m_blocks;    // size 64 each
    int_vector m_supblocks; // size SUP_SZ each

public:
    inline bit_rank(const bit_vector& bv) : m_bv(&bv) {
        const size_t n = m_bv->size();

        // determine number of superblocks and superblock entry width
        const size_t sw = log2_ceil(n-1);
        const size_t sq = n >> SUP_W;
        const size_t sm = n & SUP_MSK;
        m_supblocks.resize(sm ? sq + 1 : sq, sw);

        // determine number of blocks and block width
        const size_t bq = n >> 6ULL; // div 64
        const size_t bm = n & 63ULL; // mod 64
        m_blocks.resize(bm ? bq + 1 : bq, SUP_W);

        // construct
        {
            size_t rank_bv = 0; // 1-bits in whole BV
            size_t rank_sb = 0; // 1-bits in current superblock
            size_t cur_sb = 0;  // current superblock

            for(size_t j = 0; j < m_blocks.size(); j++) {
                size_t i = j >> SB_INNER_RS;
                if(i > cur_sb) {
                    // we reached a new superblock
                    m_supblocks[cur_sb] = rank_bv;
                    rank_sb = 0;
                    cur_sb = i;
                }

                auto rank_b = rank1_u64(m_bv->block64(j));
                rank_sb += rank_b;
                rank_bv += rank_b;

                m_blocks[j] = rank_sb;
            }
        }
    }

    inline bit_rank() : m_bv(nullptr) {
    }

    inline bit_rank(const bit_rank& other) {
        *this = other;
    }

    inline bit_rank(bit_rank&& other) {
        *this = std::move(other);
    }

    inline bit_rank& operator=(const bit_rank& other) {
        m_bv = other.m_bv;
        m_blocks = other.m_blocks;
        m_supblocks = other.m_supblocks;
        return *this;
    }

    inline bit_rank& operator=(bit_rank&& other) {
        m_bv = other.m_bv;
        m_blocks = std::move(other.m_blocks);
        m_supblocks = std::move(other.m_supblocks);
        return *this;
    }

    inline void reassign(bit_rank&& other, const bit_vector& bv) {
        // FIXME: this shouldn't be necessary
        *this = std::move(other),
        m_bv = &bv;
    }

    inline size_t rank1(const size_t x) const {
        size_t r = 0;
        const size_t i = x >> SUP_W;
        if(i > 0) r += m_supblocks[i - 1];
        const size_t j = x >> 6ULL;

        const size_t k = j - i * BLOCKS_PER_SB;
        if(k > 0) r += m_blocks[j - 1];

        r += rank1_u64(m_bv->block64(j), x & 63ULL);
        return r;
    }

    inline size_t operator()(size_t i) const {
        return rank1(i);
    }

    inline size_t rank0(size_t x) const {
        return x + 1 - rank1(x);
    }
};

}
