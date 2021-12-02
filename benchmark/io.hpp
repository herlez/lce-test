/*******************************************************************************
 * util/io.hpp
 *
 * Copyright (C) 2019 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <filesystem>
#include <iostream>
#include <vector>
#include <fstream>

std::vector<uint8_t> load_text(std::string const& file_path,
                               size_t const prefix_size=0) {
  std::ifstream stream(file_path.c_str(), std::ios::in | std::ios::binary);
  if (!stream) {
    std::cerr << "File " << file_path << " not found" << std::endl;
    std::exit(-1);
  }

  std::filesystem::path p(file_path);

  uint64_t file_size = std::filesystem::file_size(p);
  if (prefix_size > 0) {
    file_size = std::min(prefix_size, file_size);
  }
  stream.seekg(0);
  std::vector<uint8_t> result(file_size);
  stream.read(reinterpret_cast<char*>(result.data()), file_size);
  stream.close();
  return result;
}


/******************************************************************************/
