#pragma once

#include <assert.h>

#include <vector>

#include "par_rmq_nlgn.hpp"
#include <omp.h>

namespace lce_test::par {
//static constexpr uint64_t c_block_size = 32;
template <typename key_type, u_int64_t c_block_size = 256>
class par_RMQ_n {
  std::vector<key_type> const& m_data;
  std::vector<uint32_t> m_sampled_indexes;
  std::vector<uint32_t> m_sampled_minimas;
  par_RMQ_nlgn<uint32_t> m_sampled_rmq;

 public:
  par_RMQ_n(std::vector<key_type> const& data) : m_data(data) {
    const uint64_t num_sampled_elements = (data.size() - 1) / c_block_size + 1;
    m_sampled_indexes.resize(num_sampled_elements);
    m_sampled_minimas.resize(num_sampled_elements);
    
    //Get the minimal elements from the blocks.
    #pragma omp parallel for
    for (size_t block = 0; block < num_sampled_elements; ++block) {
      uint32_t min_index = block * c_block_size;
      for (size_t i = block * c_block_size; i < (1 + block) * c_block_size; ++i) {
        min_index = data[min_index] <= data[i] ? min_index : i;
      }
      m_sampled_indexes[block] = min_index;
      m_sampled_minimas[block] = m_data[min_index];
    }
    //Also get the minimum from the last block.
    if (data.size() % c_block_size != 0) {
      uint32_t min_index = data.size() - 1;
      for (size_t i = data.size() - 1; i % c_block_size != 0; --i) {
        min_index = data[min_index] <= data[i] ? min_index : i;
      }
      m_sampled_indexes.push_back(min_index);
      m_sampled_minimas.push_back(m_data[min_index]);
    }
    //Build an RMQ data structure for these block minimas.
    m_sampled_rmq = par_RMQ_nlgn<uint32_t>(m_sampled_minimas);
  }

  size_t rmq(uint32_t const left, uint32_t const right) const {
    if (right - left <= c_block_size) {
      uint32_t min = left;
      for (uint32_t i = left; i <= right; ++i) {
        min = m_data[min] < m_data[i] ? min : i;
      }
      return min;
    }
    //Min in left block
    uint32_t min_beg = left;
    uint32_t const check_left_until = std::min(m_data.size(), c_block_size * (1 + left / c_block_size));
    for (uint32_t i = left; i < check_left_until; ++i) {
      min_beg = m_data[min_beg] < m_data[i] ? min_beg : i;
    }

    //Min in right block
    uint32_t min_end = (right / c_block_size) * c_block_size;
    for (uint32_t i = (right / c_block_size) * c_block_size; i <= right; ++i) {
      min_end = m_data[min_end] < m_data[i] ? min_end : i;
    }
    uint32_t const min_beg_end = m_data[min_beg] < m_data[min_end] ? min_beg : min_end;

    //Now look for min in middle part.
    uint32_t const l_block = (left / c_block_size) + 1;
    uint32_t const r_block = (right / c_block_size) - 1;
    if (r_block < l_block) {
      return min_beg_end;
    }
    uint32_t const min_mid = m_sampled_indexes[m_sampled_rmq.rmq(l_block, r_block)];
    return m_data[min_mid] < m_data[min_beg_end] ? min_mid : min_beg_end;
  }
};  // class RMQ_n
}  // namespace par_rmq