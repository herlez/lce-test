#pragma once

#include <omp.h>

#include <string>
#include <vector>

#include "../util/synchronizing_sets/ring_buffer.hpp"
#include "rk_prime.hpp"

template <size_t t_tau = 1024, typename t_index = uint32_t>
class string_synchronizing_set_par {
  __extension__ typedef unsigned __int128 uint128_t;

 private:
  std::vector<t_index> m_sss;

 public:
  std::vector<t_index> get_sss() {
    return m_sss;
  }

  string_synchronizing_set_par(const std::vector<uint8_t>& text) {
    std::vector<std::vector<t_index>> sss_part{};
    sss_part.resize(omp_get_max_threads());

    #pragma omp parallel
    {
      const size_t size_per_thread = text.size() / omp_get_num_threads();
      const int t = omp_get_thread_num();
      sss_part[t] = fill_synchronizing_set(text, size_per_thread * t, std::min(text.size(), size_per_thread * (t + 1)));
    }

    //Merge parts
    std::vector<size_t> write_pos{0};
    for (auto& part : sss_part) {
      write_pos.push_back(write_pos.back() + part.size());
    }
    size_t sss_size = write_pos.back();
    m_sss.resize(sss_size);

    #pragma omp parallel
    {
      const int t = omp_get_thread_num();
      std::copy(sss_part[t].begin(), sss_part[t].end(), m_sss.begin() + write_pos[t]);
    }
  }

  std::vector<t_index> fill_synchronizing_set(const std::vector<uint8_t>& text, const size_t from, const size_t to) const {
    std::vector<t_index> sss;

    //Calculate first fingerprint
    herlez::rolling_hash::rk_prime<decltype(text.cbegin()), 107> rk(text.cbegin(), t_tau);
    
    uint128_t fp = rk.get_currect_fp();

    ring_buffer<uint128_t> fingerprints(4 * t_tau);
    fingerprints.resize(from);
    fingerprints.push_back(fp);

    uint128_t min = 0;
    for (size_t i = from; i < to;) {
      // Compare this id with every other index which is not in q
      min = 0;
      size_t required = i + t_tau - (fingerprints.size() - 1);
      for(size_t num_rolled = 0; num_rolled < required; ++num_rolled) {
        fingerprints.push_back(rk.roll());
      }

      for (unsigned int j = 1; j <= t_tau; ++j) {
        if (fingerprints[i + j] < fingerprints[i + min]) {
          min = j;
        }
      }
      if (min == 0 || min == t_tau) {
        sss.push_back(i);
      }

      size_t local_min = i + min;
      ++i;
      fingerprints.push_back(rk.roll());
      while (i < to && i < local_min) {
        if (fingerprints[i + t_tau] <= fingerprints[local_min]) {
          sss.push_back(i);
          if (fingerprints[i + t_tau] != fingerprints[local_min]) {
            local_min = i + t_tau;
          }
        }
        i++;
        fingerprints.push_back(rk.roll());
      }
    }
    return sss;
  }
};
