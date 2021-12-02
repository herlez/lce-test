#include <iostream>
#include <ips4o.hpp>
#include <string>
#include <tlx/sort/strings/parallel_sample_sort.hpp>
#include <tlx/sort/strings/string_ptr.hpp>
#include <tlx/sort/strings/string_set.hpp>
#include <vector>

#include "io.hpp"
#include "lce_prezza_fast.hpp"
#include "lce_semi_synchronizing_sets_par.hpp"
#include "timer.hpp"

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cout << "Use: bench_sparse_ss text_path sample_distance algo\n";
    return -1;
  }
  std::string text_path = argv[1];
  size_t sample_distance = std::stoi(argv[2]);
  size_t algo = std::atoi(argv[3]);
  std::cout << "Text: " << text_path << " Sample Distance: " << sample_distance << " Algo: " << algo << '\n';
  // Get Text
  std::vector<uint8_t> text;
  text = load_text(text_path);
  size_t text_size = text.size();
  // Sample Positions
  std::vector<size_t> positions;
  for (size_t i{0}; i < text.size(); i += sample_distance) {
    positions.push_back(i);
  }

  // Sort Positions
  if (algo == 0) {
    text.resize(text.size() + (8 - (text.size() % 8)));
    timer t;
    LcePrezzaFast lce_ds(reinterpret_cast<uint64_t*>(text.data()), text.size());

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
    std::cout << "algo=prezza_std time=" << t.get_and_reset() << "\n";
  }

  if (algo == 1) {
    text.resize(text.size() + (8 - (text.size() % 8)));
    timer t;
    LcePrezzaFast lce_ds(reinterpret_cast<uint64_t*>(text.data()), text.size());

    ips4o::sort(positions.begin(), positions.end(), [&lce_ds, &text_size, &text](size_t i, size_t j) {
      size_t lce = lce_ds.lce(i, j);
      if (i + lce >= text_size) {
        return true;
      }
      if (j + lce >= text_size) {
        return false;
      }
      return text[i + lce] < text[j + lce];
    });
    std::cout << "algo=prezza_ips4o time=" << t.get_and_reset() << "\n";
  }

  if (algo == 2) {
    timer t;
    lce_test::par::LceSemiSyncSetsPar<> lce_ds(text, false);

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
    std::cout << "algo=sss_std time=" << t.get_and_reset() << "\n";
  }

  if (algo == 3) {
    timer t;
    lce_test::par::LceSemiSyncSetsPar<> lce_ds(text, false);

    ips4o::sort(positions.begin(), positions.end(), [&lce_ds, &text_size, &text](size_t i, size_t j) {
      size_t lce = lce_ds.lce(i, j);
      if (i + lce >= text_size) {
        return true;
      }
      if (j + lce >= text_size) {
        return false;
      }
      return text[i + lce] < text[j + lce];
    });
    std::cout << "algo=sss_ips4o time=" << t.get_and_reset() << "\n";
  }

  if (algo == 4) {
    std::string str;
    for (size_t i{0}; i < text.size(); ++i) {
      str.push_back(text[i]);
    }
    timer t;
    tlx::sort_strings_detail::StringSuffixSet string_suf_set(str, positions.begin(), positions.end());
    tlx::sort_strings_detail::StringPtr strptr(string_suf_set);
    tlx::sort_strings_detail::parallel_sample_sort(strptr, 0, 0);

    std::cout << "algo=string_sort time=" << t.get_and_reset() << "\n";
  }
}
