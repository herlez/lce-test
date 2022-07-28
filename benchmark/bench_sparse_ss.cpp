#include <malloc_count.h>
#include <src/libsais64.h>

#include <iostream>
#include <ips4o.hpp>
#include <numeric>
#include <string>
#include <string_view>
#include <tlx/sort/strings/parallel_sample_sort.hpp>
#include <tlx/sort/strings/string_ptr.hpp>
#include <tlx/sort/strings/string_set.hpp>
#include <vector>

#include "io.hpp"
#include "lce_naive.hpp"
#include "lce_prezza.hpp"
#include "lce_semi_synchronizing_sets_par.hpp"
#include "timer.hpp"

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cout << "Use: bench_sparse_ss text_path sample_distance algo\n";
    return -1;
  }
  std::filesystem::path text_path = argv[1];
  std::string text_name = text_path.filename();
  size_t sample_distance = std::stoi(argv[2]);
  size_t algo = std::atoi(argv[3]);
  std::cout << "Text: " << text_path << " Sample Distance: " << sample_distance << " Algo: " << algo << '\n';
  // Get Text
  std::vector<uint8_t> text;
  text = load_text(text_path);
  const size_t text_size = text.size();
  std::vector<uint8_t> text_check = text;
  // Sample Positions
  std::vector<size_t> positions;
  if (!std::has_single_bit(sample_distance) || sample_distance == 1) {
    for (size_t i{0}; i < text.size(); i += sample_distance) {
      positions.push_back(i);
    }
  } else {
    if (sample_distance == 256) {
      lce_test::par::LceSemiSyncSetsPar<256> lce_ds(text, false);
      auto positions64 = lce_ds.getSyncSet();
      positions = std::vector<size_t>(positions64.begin(), positions64.end());
    }
    if (sample_distance == 512) {
      lce_test::par::LceSemiSyncSetsPar<512> lce_ds(text, false);
      auto positions64 = lce_ds.getSyncSet();
      positions = std::vector<size_t>(positions64.begin(), positions64.end());
    }
    if (sample_distance == 1024) {
      lce_test::par::LceSemiSyncSetsPar<1024> lce_ds(text, false);
      auto positions64 = lce_ds.getSyncSet();
      positions = std::vector<size_t>(positions64.begin(), positions64.end());
    }
    if (sample_distance == 2048) {
      lce_test::par::LceSemiSyncSetsPar<2048> lce_ds(text, false);
      auto positions64 = lce_ds.getSyncSet();
      positions = std::vector<size_t>(positions64.begin(), positions64.end());
    }
  }
  std::vector<size_t> positions_check = positions;

  // Naive
  if (algo == 0) {
    malloc_count_reset_peak();
    auto mem_before = malloc_count_current();
    timer t;
    LceNaive lce_ds(text);
    std::sort(positions.begin(), positions.end(), [&lce_ds, &text_size, &text](size_t i, size_t j) {
      size_t lce = lce_ds.lce(i, j);
      if (i + lce >= text_size) {
        return true;
      }
      if (j + lce >= text_size) {
        return false;
      }
      return text[i + lce] < text[j + lce];
    });
    std::cout << "RESULT algo=naive_ips4o time=" << t.get_and_reset()
              << " sample_distance=" << sample_distance
              << " text_name=" << text_name
              << " mem_ds=" << malloc_count_current() - mem_before
              << " mem_peak=" << malloc_count_peak() - mem_before
              << "\n";
  }
  // Memcmp
  if (algo == 1) {
    malloc_count_reset_peak();
    auto mem_before = malloc_count_current();
    timer t;
    std::sort(positions.begin(), positions.end(), [&text_size, &text](size_t i, size_t j) {
      return (std::memcmp(text.data() + i, text.data() + j, text_size - std::max(i, j)) < 0);
    });
    std::cout << "RESULT algo=memcmp time=" << t.get_and_reset()
              << " sample_distance=" << sample_distance
              << " text_name=" << text_name
              << " mem_ds=" << malloc_count_current() - mem_before
              << " mem_peak=" << malloc_count_peak() - mem_before
              << "\n";
  }

  // In-Place Fingerprinting
  if (algo == 2) {
    text.resize(text.size() + (8 - (text.size() % 8)));
    malloc_count_reset_peak();

    auto mem_before = malloc_count_current();
    timer t;
    timer t_construct;
    LcePrezza lce_ds(reinterpret_cast<uint64_t*>(text.data()), text.size());
    auto constr_time = t.get();

    timer t_sort;
    std::sort(positions.begin(), positions.end(), [&lce_ds, &text_size, &text](size_t i, size_t j) {
      size_t lce = lce_ds.lce(i, j);
      if (i + lce >= text_size) [[unlikely]] {
        return true;
      }
      if (j + lce >= text_size) [[unlikely]] {
        return false;
      }
      return lce_ds[i + lce] < lce_ds[j + lce];
    });
    auto sort_time = t_sort.get();

    timer t_reconstruct;
    lce_ds.retransform_text();
    auto reconstruct_time = t_reconstruct.get();

    std::cout << "RESULT algo=prezza_ips4o time=" << t.get_and_reset()
              << " sample_distance=" << sample_distance
              << " text_name=" << text_name
              << " constr_time=" << constr_time
              << " sort_time=" << sort_time
              << " reconstruct_time=" << reconstruct_time
              << " mem_ds=" << malloc_count_current() - mem_before
              << " mem_peak=" << malloc_count_peak() - mem_before
              << "\n";
  }

  // SSS
  if (algo == 3) {
    malloc_count_reset_peak();
    auto mem_before = malloc_count_current();
    timer t;
    lce_test::par::LceSemiSyncSetsPar<256> lce_ds(text, false);
    auto constr_time = t.get();
    std::sort(positions.begin(), positions.end(), [&lce_ds, &text_size, &text](size_t i, size_t j) {
      size_t lce = lce_ds.lce(i, j);
      if (i + lce >= text_size) [[unlikely]] {
        return true;
      }
      if (j + lce >= text_size) [[unlikely]] {
        return false;
      }
      return text[i + lce] < text[j + lce];
    });
    std::cout << "RESULT algo=sss256_ips4o time=" << t.get_and_reset()
              << " sample_distance=" << sample_distance
              << " text_name=" << text_name
              << " constr_time=" << constr_time
              << " mem_ds=" << malloc_count_current() - mem_before
              << " mem_peak=" << malloc_count_peak() - mem_before
              << "\n";
  }
  if (algo == 4) {
    malloc_count_reset_peak();
    auto mem_before = malloc_count_current();
    timer t;
    lce_test::par::LceSemiSyncSetsPar<512> lce_ds(text, false);
    auto constr_time = t.get();
    std::sort(positions.begin(), positions.end(), [&lce_ds, &text_size, &text](size_t i, size_t j) {
      size_t lce = lce_ds.lce(i, j);
      if (i + lce >= text_size) [[unlikely]] {
        return true;
      }
      if (j + lce >= text_size) [[unlikely]] {
        return false;
      }
      return text[i + lce] < text[j + lce];
    });
    std::cout << "RESULT algo=sss512_ips4o time=" << t.get_and_reset()
              << " sample_distance=" << sample_distance
              << " text_name=" << text_name
              << " constr_time=" << constr_time
              << " mem_ds=" << malloc_count_current() - mem_before
              << " mem_peak=" << malloc_count_peak() - mem_before
              << "\n";
  }
  if (algo == 5) {
    malloc_count_reset_peak();
    auto mem_before = malloc_count_current();
    timer t;
    lce_test::par::LceSemiSyncSetsPar<1024> lce_ds(text, false);
    auto constr_time = t.get();
    std::sort(positions.begin(), positions.end(), [&lce_ds, &text_size, &text](size_t i, size_t j) {
      size_t lce = lce_ds.lce(i, j);
      if (i + lce >= text_size) [[unlikely]] {
        return true;
      }
      if (j + lce >= text_size) [[unlikely]] {
        return false;
      }
      return text[i + lce] < text[j + lce];
    });
    std::cout << "RESULT algo=sss1024_ips4o time=" << t.get_and_reset()
              << " sample_distance=" << sample_distance
              << " text_name=" << text_name
              << " constr_time=" << constr_time
              << " mem_ds=" << malloc_count_current() - mem_before
              << " mem_peak=" << malloc_count_peak() - mem_before
              << "\n";
  }
  if (algo == 6) {
    malloc_count_reset_peak();
    auto mem_before = malloc_count_current();
    timer t;
    lce_test::par::LceSemiSyncSetsPar<2048> lce_ds(text, false);
    auto constr_time = t.get();
    std::sort(positions.begin(), positions.end(), [&lce_ds, &text_size, &text](size_t i, size_t j) {
      size_t lce = lce_ds.lce(i, j);
      if (i + lce >= text_size) [[unlikely]] {
        return true;
      }
      if (j + lce >= text_size) [[unlikely]] {
        return false;
      }
      return text[i + lce] < text[j + lce];
    });
    std::cout << "RESULT algo=sss2048_ips4o time=" << t.get_and_reset()
              << " sample_distance=" << sample_distance
              << " text_name=" << text_name
              << " constr_time=" << constr_time
              << " mem_ds=" << malloc_count_current() - mem_before
              << " mem_peak=" << malloc_count_peak() - mem_before
              << "\n";
  }

  // String Sorting
  if (algo == 7) {
    std::string str;
    for (size_t i{0}; i < text.size(); ++i) {
      str.push_back(text[i]);
    }
    malloc_count_reset_peak();
    auto mem_before = malloc_count_current();
    timer t;
    tlx::sort_strings_detail::StringSuffixSet string_suf_set(str, positions.begin(), positions.end());
    tlx::sort_strings_detail::StringPtr strptr(string_suf_set);
    tlx::sort_strings_detail::parallel_sample_sort(strptr, 0, 0);

    std::cout << "RESULT algo=string_sort time=" << t.get_and_reset()
              << " sample_distance=" << sample_distance
              << " text_name=" << text_name
              << " mem_ds=" << malloc_count_current() - mem_before
              << " mem_peak=" << malloc_count_peak() - mem_before
              << "\n";
  }

  // Libsais
  if (algo == 8 && sample_distance == 1) {
    malloc_count_reset_peak();
    auto mem_before = malloc_count_current();
    timer t;
    libsais(text.data(), reinterpret_cast<int32_t*>(positions.data()), text_size, 0, nullptr);
    std::cout << "RESULT algo=libsais time=" << t.get()
              << " sample_distance=" << sample_distance
              << " text_name=" << text_name
              << " mem_ds=" << malloc_count_current() - mem_before
              << " mem_peak=" << malloc_count_peak() - mem_before
              << "\n";
  }

  // Check if text is the same as before
  for (size_t i{0}; i < text_check.size(); ++i) {
    if (text_check[i] != text[i]) {
      std::cout << "MISMATCH AT TEXT POSITION " << i << '\n';
      break;
    }
  }

  // Sort suffixes safely in order to check correctness
  std::sort(positions_check.begin(), positions_check.end(), [&text_check](size_t i, size_t j) {
    return (memcmp(text_check.data() + i, text_check.data() + j, text_check.size() - std::max(i, j)) < 0);
  });

  // Sanity check safe suffix sorting
  for (size_t i{0}; i < positions_check.size() - 1; ++i) {
    size_t pos1 = positions_check[i];
    size_t pos2 = positions_check[i + 1];
    if (std::memcmp(text_check.data() + pos1, text_check.data() + pos2, text_check.size() - std::max(pos1, pos2)) >= 0) {
      std::cout << "SANITY CHECK WRONG. WRONG ORDER AT POSITION " << i << '\n';
      break;
    }
  }

  // Check suffix sorting
  for (size_t i{0}; i < positions_check.size(); ++i) {
    if (positions_check[i] != positions[i]) {
      std::cout << "WRONG ORDER AT POSITION " << i << '\n';
      break;
    }
  }

  std::cout << '\n';
}
