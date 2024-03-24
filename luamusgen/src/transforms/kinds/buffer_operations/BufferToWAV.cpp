//
// Created by rutio on 18.07.19.
//

#include "BufferToWAV.h"
#include <util/WAV_File.h>
#include <fstream>
#include <utility>

BufferToWAV::BufferToWAV(std::string filename): filename(std::move(filename)) {}

void BufferToWAV::applyMonoInPlace(double sample_rate, int64_t length, Transform::buf_t buf,
                                   const std::vector<Transform::arg_t>& arguments) {
  std::ofstream ofs;
  ofs.open(filename.c_str(), std::ios::binary);
  if (!ofs.is_open()) {
    logErrorC("Cannot open file output stream (%s)", filename.c_str());
    return;
  }

  WAV_Header_fmt16 header;
  header.setChannelsAndSPS(1, static_cast<unsigned int>(sample_rate));
  header.setSize(static_cast<unsigned int>(length));

  ofs.write(reinterpret_cast<char*>(&header), sizeof(header));

  std::vector<int16_t> file_data(static_cast<size_t>(length));

  for (int64_t i = 0; i < length; ++i) {
    double val = buf->data_pointer[i];
    if (val > 1.0) {
      file_data[i] = 32767;
    } else if (val < -1.0) {
      file_data[i] = -32768;
    } else {
      file_data[i] = (int16_t) (val * 32767);
    }
  }

  ofs.write(reinterpret_cast<char*>(file_data.data()), file_data.size() * sizeof(int16_t));
  ofs.close();
}

void BufferToWAV::applyStereoInPlace(double sample_rate, int64_t length, Transform::buf_t bufL, Transform::buf_t bufR,
                                     const std::vector<Transform::arg_t>& arguments) {
  std::ofstream ofs;
  ofs.open(filename.c_str(), std::ios::binary);
  if (!ofs.is_open()) {
    logErrorC("Cannot open file output stream (%s)", filename.c_str());
    return;
  }

  WAV_Header_fmt16 header;
  header.setChannelsAndSPS(2, static_cast<unsigned int>(sample_rate));
  header.setSize(static_cast<unsigned int>(length));

  ofs.write(reinterpret_cast<char*>(&header), sizeof(header));

  std::vector<int16_t> file_data(static_cast<size_t>(length * 2));

  for (int64_t i = 0; i < length; ++i) {
    double valL = bufL->data_pointer[i];
    if (valL > 1.0) {
      file_data[i*2] = 32767;
    } else if (valL < -1.0) {
      file_data[i*2] = -32768;
    } else {
      file_data[i*2] = (int16_t) (valL * 32767);
    }
    double valR = bufR->data_pointer[i];
    if (valR > 1.0) {
      file_data[i*2+1] = 32767;
    } else if (valR < -1.0) {
      file_data[i*2+1] = -32768;
    } else {
      file_data[i*2+1] = (int16_t) (valR * 32767);
    }
  }

  ofs.write(reinterpret_cast<char*>(file_data.data()), file_data.size() * sizeof(int16_t));
  ofs.close();
}
