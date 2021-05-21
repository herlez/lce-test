#pragma once

#include <omp.h>

#include <string>
#include <vector>

#include "../util/synchronizing_sets/ring_buffer.hpp"

template <size_t t_tau = 1024, typename t_index = uint32_t>
class string_synchronizing_set_par {
  // static constexpr __int128 c_prime = 562949953421231ULL;
  // static constexpr __int128 c_prime = 1125899906842597ULL;
  // static constexpr __int128 c_prime = 2251799813685119ULL;
  // static constexpr __int128 c_prime = 4503599627370449ULL;
  // static constexpr __int128 c_prime = 9007199254740881ULL;
  // static constexpr __int128 c_prime = 18014398509481951ULL;
  // static constexpr __int128 c_prime = 36028797018963913ULL;
  // static constexpr __int128 c_prime = 72057594037927931ULL;
  // static constexpr __int128 c_prime = 144115188075855859ULL;
  // static constexpr __int128 c_prime = 288230376151711717ULL;
  // static constexpr __int128 c_prime = 576460752303423433ULL;
  // static constexpr __int128 c_prime = 1152921504606846883ULL;
  // static constexpr __int128 c_prime = 2305843009213693951ULL;
  static constexpr __int128 c_prime = 18446744073709551253ULL;
  static constexpr uint64_t TwoPowTauModQ = calculatePowerModulo(std::log2(t_tau), c_prime);

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

  std::vector<t_index> fill_synchronizing_set(const std::vector<uint8_t>& text, const uint64_t from, const uint64_t to) const {
    std::vector<t_index> sss;

    //Calculate first fingerprint
    unsigned __int128 fp = {0ULL};
    for (uint64_t i = 0; i < t_tau; ++i) {
      fp *= 256;
      fp += (unsigned char)text[from + i];
      fp %= c_prime;
    }
    ring_buffer<uint64_t> fingerprints(4 * t_tau);
    fingerprints.resize(from);
    fingerprints.push_back(static_cast<uint64_t>(fp));

    uint64_t min = 0;
    for (uint64_t i = from; i < to;) {
      // Compare this id with every other index which is not in q
      min = 0;
      size_t required = i + t_tau - (fingerprints.size() - 1);
      calculate_fingerprints(text, required, fp, fingerprints);
      for (unsigned int j = 1; j <= t_tau; ++j) {
        if (fingerprints[i + j] < fingerprints[i + min]) {
          min = j;
        }
      }
      if (min == 0 || min == t_tau) {
        sss.push_back(i);
      }

      uint64_t local_min = i + min;
      ++i;
      calculate_fingerprints(text, 1, fp, fingerprints);
      while (i < to && i < local_min) {
        if (fingerprints[i + t_tau] <= fingerprints[local_min]) {
          sss.push_back(i);
          if (fingerprints[i + t_tau] != fingerprints[local_min]) {
            local_min = i + t_tau;
          }
        }
        i++;
        calculate_fingerprints(text, 1, fp, fingerprints);
      }
    }
    return sss;
  }

  // Calculates the next count fingerprints
  void calculate_fingerprints(const std::vector<uint8_t>& text, size_t const count, unsigned __int128& fp,
                              ring_buffer<uint64_t>& fingerprints) const {
    for (uint64_t i = 0; i < count; ++i) {
      fp *= 256;
      fp += (unsigned char)text[t_tau + fingerprints.size() - 1];
      fp %= c_prime;

      unsigned __int128 first_char_influence = text[fingerprints.size() - 1];
      first_char_influence *= TwoPowTauModQ;
      first_char_influence %= c_prime;

      if (first_char_influence < fp) {
        fp -= first_char_influence;
      } else {
        fp = c_prime - (first_char_influence - fp);
      }
      fingerprints.push_back(static_cast<uint64_t>(fp));
    }
  }
};
