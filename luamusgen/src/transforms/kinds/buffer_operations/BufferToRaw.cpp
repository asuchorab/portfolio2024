//
// Created by rutio on 13.08.2019.
//

#include "BufferToRaw.h"
#include <fstream>
#include <utility>

BufferToRaw::BufferToRaw(std::string filename): filename(std::move(filename)) {}

void BufferToRaw::applyMonoInPlace(double sample_rate, int64_t length, Transform::buf_t buf,
                                   const std::vector<Transform::arg_t>& arguments) {
  std::ofstream ofs;
  ofs.open(filename.c_str(), std::ios::binary);
  if (!ofs.is_open()) {
    logErrorC("Cannot open file output stream (%s)", filename.c_str());
    return;
  }

  ofs.write(reinterpret_cast<char*>(buf->data_pointer), length * sizeof(double));
  ofs.close();
}

void BufferToRaw::applyStereoInPlace(double sample_rate, int64_t length, Transform::buf_t bufL, Transform::buf_t bufR,
                                     const std::vector<Transform::arg_t>& arguments) {
  std::ofstream ofs;
  ofs.open(filename.c_str(), std::ios::binary);
  if (!ofs.is_open()) {
    logErrorC("Cannot open file output stream (%s)", filename.c_str());
    return;
  }

  std::vector<double> file_data(static_cast<size_t>(length * 2));

  for (int64_t i = 0; i < length; ++i) {
    file_data[i*2] = bufL->data_pointer[i];
    file_data[i*2+1] = bufR->data_pointer[i];
  }

  ofs.write(reinterpret_cast<char*>(file_data.data()), file_data.size() * sizeof(double));
  ofs.close();
}
