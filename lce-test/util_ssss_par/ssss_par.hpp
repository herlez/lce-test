#pragma once

#include <omp.h>

#include <string>
#include <vector>
#include <parallel_hashmap/phmap.h>
#include <mutex>

#include "../util/synchronizing_sets/ring_buffer.hpp"
#include "rk_prime.hpp"

template <size_t t_tau = 1024, typename t_index = uint32_t>
class string_synchronizing_set_par {
  __extension__ typedef unsigned __int128 uint128_t;

 private:
  std::vector<t_index> m_sss;
  bool m_runs_detected;
  phmap::parallel_flat_hash_map<t_index, int64_t, phmap::priv::hash_default_hash<t_index>,
      phmap::priv::hash_default_eq<t_index>, 
      phmap::priv::Allocator<std::pair<const t_index, int64_t>>,
      4, std::mutex> m_run_info;

 public:
  std::vector<t_index> const& get_sss() const {
    return m_sss;
  }

  int64_t get_run_info(t_index i) const {
    auto run_info_entry = m_run_info.find(i);
    return run_info_entry == m_run_info.end() ? 0 : run_info_entry->second;
  }
  
  bool has_runs() const {
    return m_runs_detected;
  }
  size_t size() const {
    return m_sss.size();
  }

  inline t_index operator[](size_t i) const {
    return m_sss[i];
  }

  string_synchronizing_set_par() = default;
  string_synchronizing_set_par(const std::vector<uint8_t>& text) {
    std::vector<std::vector<t_index>> sss_part(omp_get_max_threads());

#pragma omp parallel
    {
      const size_t sss_end = text.size() - 2 * t_tau + 1;
      const size_t size_per_thread = (sss_end / omp_get_num_threads()) + 1;
      const int t = omp_get_thread_num();
      const size_t start = size_per_thread * t;
      const size_t end = (t == omp_get_num_threads() - 1) ? sss_end : (t + 1) * size_per_thread;
      sss_part[t] = fill_synchronizing_set(text, start, end);
    }

    //Merge SSS parts
    std::vector<size_t> write_pos{0};
    for (auto& part : sss_part) {
      write_pos.push_back(write_pos.back() + part.size());
    }
    size_t sss_size = write_pos.back();  //+1 for sentinel
    m_runs_detected = sss_size > text.size()*6 / t_tau;

    //If the text contains long runs, the sss inflates. We the then use a algorithm which detects runs.
    if (m_runs_detected) {
#pragma omp parallel
      {
        const size_t sss_end = text.size() - 2 * t_tau + 1;
        const size_t size_per_thread = (sss_end / omp_get_num_threads()) + 1;
        const int t = omp_get_thread_num();
        const size_t start = size_per_thread * t;
        const size_t end = (t == omp_get_num_threads() - 1) ? sss_end : (t + 1) * size_per_thread;
        sss_part[t] = fill_synchronizing_set_runs(text, start, end);
      }
      write_pos = {0};
      for (auto& part : sss_part) {
        write_pos.push_back(write_pos.back() + part.size());
      }
      sss_size = write_pos.back() + 1;  //+1 for sentinel
    }

    m_sss.resize(sss_size);
#pragma omp parallel
    {
      const int t = omp_get_thread_num();
      std::copy(sss_part[t].begin(), sss_part[t].end(), m_sss.begin() + write_pos[t]);
    }
    if (m_runs_detected) {
      m_sss.back() = text.size() - 2 * t_tau + 1;  //sentinel needed for text with runs
    }
  }

  std::vector<t_index> fill_synchronizing_set(const std::vector<uint8_t>& text, const size_t from, const size_t to) const {
    //calculate SSS
    std::vector<t_index> sss;

    herlez::rolling_hash::rk_prime<decltype(text.cbegin()), 107> rk(text.cbegin() + from, t_tau, 296813);
    ring_buffer<uint128_t> fingerprints(4 * t_tau);
    fingerprints.resize(from);
    fingerprints.push_back(rk.get_current_fp());

    t_index first_min = 0;

    //Loop:
    for (size_t i = from; i < to; ++i) {
      for (size_t j = fingerprints.size(); j <= i + t_tau; ++j) {
        fingerprints.push_back(rk.roll());
      }

      if (first_min == 0 || first_min < i) {
        first_min = i;
        for (size_t j = i; j <= i + t_tau; ++j) {
          if (fingerprints[j] < fingerprints[first_min]) {
            first_min = j;
          }
        }
      } else if (fingerprints[i + t_tau] < fingerprints[first_min]) {
        first_min = i + t_tau;
      }

      if (fingerprints[first_min] == fingerprints[i] || fingerprints[first_min] == fingerprints[i + t_tau]) {
        sss.push_back(i);
      }
    }
    return sss;
  }

  std::vector<t_index> fill_synchronizing_set_runs(const std::vector<uint8_t>& text, const size_t from, const size_t to) {
    //calculate Q
    std::vector<std::pair<t_index, t_index>> qset = calculate_q(text, from, to);
    
    /* PRINT Q
    #pragma omp critical
    {
      std::cout << "\nfrom " << from << " to " << to << " (" << to + t_tau <<")\n";
      std::cout << "Q size: " << qset.size() << " = [";
      for(size_t i = 0; i < std::min(qset.size(), size_t{10}); ++i) {
        std::cout << "[" << qset[i].first << ", " << qset[i].second << "], ";
      }
      std::cout << "]; \n";
    }
    */
    
    qset.push_back(std::make_pair(std::numeric_limits<t_index>::max(), std::numeric_limits<t_index>::max()));
    auto it_q = qset.begin();
    //calculate SSS
    //BEGIN
    std::vector<t_index> sss;

    herlez::rolling_hash::rk_prime<decltype(text.cbegin()), 107> rk(text.cbegin() + from, t_tau, 296813);
    ring_buffer<uint128_t> fingerprints(4 * t_tau);
    fingerprints.resize(from);
    fingerprints.push_back(rk.get_current_fp());

    t_index MIN_UNKNOWN = std::numeric_limits<t_index>::max();
    t_index first_min = MIN_UNKNOWN;
    //Loop:
    for (size_t i = from; i < to; ++i) {
      for (size_t j = fingerprints.size(); j <= i + t_tau; ++j) {
        fingerprints.push_back(rk.roll());
      }
      while (it_q->second < i) {
        std::advance(it_q, 1);
      }

      //If then minimum in the current range is not known, we need to find one
      if (first_min == MIN_UNKNOWN || first_min < i) {
        auto it_qt = it_q;
        for (size_t j = i; j <= i + t_tau; ++j) {
          //advance q pointer
          if (it_qt->second < j) {
            std::advance(it_qt, 1);
          }
          //don't compare values from q
          if (it_qt->first <= j) {
            //first_min = it_qt->second + 1;
            //j = first_min;
            j = it_qt->second;
            continue;
          }
          //take first fingerprint not in q
          if (first_min == MIN_UNKNOWN || first_min < i) {
            first_min = j;
          }
          //compare values that are not in q
          if (fingerprints[j] < fingerprints[first_min]) {
            first_min = j;
          }
        }
        //If no minimum exists, we jump to the next position, which may be part of sss 
        if(first_min == MIN_UNKNOWN || first_min < i) {
          i = it_qt->second - t_tau;
          continue;
        }
      }
      //If the minimum of the range is already known, we only need to compare with the new fingerprint
      else if (first_min <= i + t_tau) {
        auto it_qt = it_q;
        while (it_qt->second < i + t_tau) {
          std::advance(it_qt, 1);
        }
        if (it_qt->first > i + t_tau && fingerprints[i + t_tau] < fingerprints[first_min]) {
          first_min = i + t_tau;
        }
      }
      //maybe_add(i);
      if (fingerprints[first_min] == fingerprints[i] || fingerprints[first_min] == fingerprints[i + t_tau]) {
        sss.push_back(i);
      }
    }
    return sss;
  }

  std::vector<std::pair<t_index, t_index>> calculate_q(const std::vector<uint8_t>& text, const size_t from, const size_t to) {
    std::vector<std::pair<t_index, t_index>> qset{};
    constexpr size_t small_tau = t_tau / 3;
    herlez::rolling_hash::rk_prime<decltype(text.cbegin()), 107> rk(text.cbegin() + from, small_tau, 296813);

    ring_buffer<uint128_t> fingerprints(4 * t_tau);
    fingerprints.resize(from);
    fingerprints.push_back(rk.get_current_fp());

    for (size_t i = from; i < to + t_tau; ++i) {  //++i correct?
      for (size_t j = fingerprints.size(); j < i + t_tau; ++j) {
        fingerprints.push_back(rk.roll());
      }
      //find first minimum
      size_t first_min = i;
      for (size_t j = first_min; j < i + small_tau; ++j) {
        if (fingerprints[j] < fingerprints[first_min]) {
          first_min = j;
        }
      }
      //find next minimum
      size_t next_min = first_min + 1;
      for (size_t j = next_min; j < first_min + small_tau; ++j) {
        if (fingerprints[j] < fingerprints[first_min]) {
          next_min = j;
        }
      }

      //if minimum fps match, look for run
      if (fingerprints[next_min] != fingerprints[first_min]) {
        i = next_min - 1;
      } else {
        //if matching fingerprint exists, extend the run and add it to q
        size_t const period = next_min - first_min;
        //now extend run naivly to the left
        size_t run_start = first_min;
        while (run_start > from && text[run_start-1] == text[run_start + period-1]) {
          --run_start;
        }
        
        //extend run naivly to the right
        size_t run_end = next_min;
        while (run_end < to + 2 * t_tau - 2 && text[run_end+1] == text[run_end - period+1]) {
          ++run_end;
        }

        //add run to set q
        if (run_end - run_start + 1 >= t_tau) {
          qset.push_back(std::make_pair(run_start, run_end - t_tau + 1));
          i = run_end - small_tau;

          if(run_end - run_start + 1 >= 3 * t_tau - 1) {
            size_t const sss_pos1 = run_start - 1;
            size_t const sss_pos2 = run_end - (2*t_tau) + 2; 
            int64_t const run_info = int64_t{1} * text.size() - sss_pos2 + sss_pos1;
            m_run_info[sss_pos1] = text[run_end + 1] > text[run_end - period + 1] ? run_info : run_info * (-1); 
          }
        } else {
          i = next_min - 1;
        }
      }
    }
    return qset;
  }

  bool check_sss() {

    //Check sss for 
    return true;
  }
};
