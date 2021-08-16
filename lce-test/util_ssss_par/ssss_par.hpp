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
      const size_t sss_end = text.size() - 2 * t_tau + 1;
      const size_t size_per_thread = (sss_end / omp_get_num_threads()) + 1;
      const int t = omp_get_thread_num();
      sss_part[t] = fill_synchronizing_set(text, size_per_thread * t, std::min(sss_end, size_per_thread * (t + 1)));
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
    herlez::rolling_hash::rk_prime<decltype(text.cbegin()), 107> rk(text.cbegin() + from, t_tau, 296813);

    uint128_t fp = rk.get_currect_fp();

    ring_buffer<uint128_t> fingerprints(4 * t_tau);
    fingerprints.resize(from);
    fingerprints.push_back(fp);

    for (size_t i = from; i < to;) {
      // Compare this id with every other index which is not in q
      size_t required = i + t_tau - (fingerprints.size() - 1);
      for (size_t num_rolled = 0; num_rolled < required; ++num_rolled) {
        fingerprints.push_back(rk.roll());
      }
      uint128_t first_min = i;
      bool add_index = true;

      for (unsigned int j = i + 1; j < i + t_tau; ++j) {
        if (fingerprints[j] < fingerprints[first_min]) {
          add_index = false;
          first_min = j;
        }
      }
      if (fingerprints[i + t_tau] <= fingerprints[first_min]) {
        add_index = true;
        if (fingerprints[i + t_tau] != fingerprints[first_min]) {
          first_min = i + t_tau;
        }
      }

      if (add_index) {
        sss.push_back(i);
      }

      ++i;
      fingerprints.push_back(rk.roll());
      while (i < to && i < first_min) {
        if (fingerprints[i + t_tau] <= fingerprints[first_min]) {
          sss.push_back(i);
          if (fingerprints[i + t_tau] != fingerprints[first_min]) {
            first_min = i + t_tau;
          }
        }
        ++i;
        fingerprints.push_back(rk.roll());
      }
    }
    return sss;
  }
};
