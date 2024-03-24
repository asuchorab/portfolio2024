//
// Created by rutio on 13.08.2019.
//

#include "BufferFromRaw.h"
#include <fstream>
#include <utility>
#include <util/MathUtil.h>

BufferFromRaw::BufferFromRaw(std::string filename) : filename(std::move(filename)) {}

void BufferFromRaw::applyMonoInPlace(double sample_rate, int64_t length, Transform::buf_t buf,
                                     const std::vector<Transform::arg_t>& arguments) {
  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ios::binary);
  if (!ifs.is_open()) {
    logErrorC("Cannot open file input stream (%s)", filename.c_str());
    return;
  }

  std::vector<double> file_data(length);

  ifs.read(reinterpret_cast<char*>(file_data.data()), length * sizeof(double));

  if (ifs.eof()) {
    int64_t expected_length = length;
    length = ifs.tellg() / sizeof(double);
    logWarningC("[%s] Could not read expected amount of samples; requested %d, found %d", filename.c_str(),
                expected_length, length);
  }
  for (int64_t i = 0; i < length; ++i) {
    buf->data_pointer[i] += file_data[i];
  }
}

void BufferFromRaw::applyStereoInPlace(double sample_rate, int64_t length, Transform::buf_t bufL, Transform::buf_t bufR,
                                       const std::vector<Transform::arg_t>& arguments) {
  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ios::binary);
  if (!ifs.is_open()) {
    logErrorC("Cannot open file input stream (%s)", filename.c_str());
    return;
  }
  std::vector<double> file_data(length * 2);

  ifs.read(reinterpret_cast<char*>(file_data.data()), length * 2 * sizeof(double));

  int found_length = (int) length;
  if (ifs.eof()) {
    ifs.clear();
    found_length = (int) ifs.tellg() / (sizeof(double) * 2);
    logWarningC("[%s] Could not read expected amount of samples; requested %d, found %d", filename.c_str(),
                length, found_length);
  }

  if (found_length <= (int) length) {
    for (int64_t i = 0; i < length; ++i) {
      bufL->data_pointer[i] += file_data[i * 2];
      bufR->data_pointer[i] += file_data[i * 2 + 1];
    }
  }
}

