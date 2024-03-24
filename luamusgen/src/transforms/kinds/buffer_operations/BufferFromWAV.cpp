//
// Created by rutio on 18.07.19.
//

#include "BufferFromWAV.h"
#include <util/WAV_File.h>
#include <fstream>
#include <utility>
#include <util/MathUtil.h>

BufferFromWAV::BufferFromWAV(std::string filename): filename(std::move(filename)) {}

void BufferFromWAV::applyMonoInPlace(double sample_rate, int64_t length, Transform::buf_t buf,
                                     const std::vector<Transform::arg_t>& arguments) {
  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ios::binary);
  if (!ifs.is_open()) {
    logErrorC("Cannot open file input stream (%s)", filename.c_str());
    return;
  }

  WAV_Header_fmt16 header;
  ifs.read(reinterpret_cast<char*>(&header), sizeof(header));

  if (!header.isValid()) {
    logErrorC(("[" + filename + "] Invalid header").c_str());
    return;
  }

  int64_t sample_count = header.getSamplesCount();

  std::vector<int16_t> file_data(static_cast<size_t>(sample_count));

  ifs.read(reinterpret_cast<char*>(file_data.data()), sample_count * sizeof(int16_t));

  if (ifs.eof()) {
    int64_t expected_length = length;
    length = ifs.tellg() / sizeof(double);
    logWarningC("[%s] Could not read expected amount of samples; requested %d, found %d", filename.c_str(),
                expected_length, length);
  }

  if (header.nChannels != 1) {
    logErrorC(("[" + filename + "] Requested file isn't in mono, nChannels = " + std::to_string(header.nChannels)).c_str());
    return;
  }

  if (sample_count > length) {
    logWarningC(("[" + filename + "] File is too long (" + std::to_string(sample_count)
                + " to fit in the requested size (" + std::to_string(length) + ")").c_str());
  } else if (sample_count < length) {
    length = sample_count;
  }

  if (!mathut::equals(sample_rate, header.nSamplesPerSec)) {
    logWarningC(("[" + filename + "] File's sample rate (" + std::to_string(header.nSamplesPerSec)
                + " differs from expected (" + std::to_string(sample_rate) + ")").c_str());
  }

  for (int64_t i = 0; i < length; ++i) {
    buf->data_pointer[i] += file_data[i] / 32767.0;
  }
}

void BufferFromWAV::applyStereoInPlace(double sample_rate, int64_t length, Transform::buf_t bufL, Transform::buf_t bufR,
                                       const std::vector<Transform::arg_t>& arguments) {
  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ios::binary);
  if (!ifs.is_open()) {
    logErrorC("Cannot open file input stream (%s)", filename.c_str());
    return;
  }

  WAV_Header_fmt16 header;
  ifs.read(reinterpret_cast<char*>(&header), sizeof(header));

  if (!header.isValid()) {
    logErrorC(("[" + filename + "] Invalid header").c_str());
    return;
  }

  int64_t sample_count = header.getSamplesCount();

  std::vector<int16_t> file_data(static_cast<size_t>(sample_count * 2));

  ifs.read(reinterpret_cast<char*>(file_data.data()), sample_count * 2 * sizeof(int16_t));

  if (ifs.eof()) {
    int64_t expected_length = length;
    length = ifs.tellg() / (sizeof(double) * 2);
    logWarningC("[%s] Could not read expected amount of samples; requested %d, found %d", filename.c_str(),
                expected_length, length);
  }

  if (header.nChannels != 2) {
    logErrorC(("[" + filename + "] Requested file isn't in stereo, nChannels = " + std::to_string(header.nChannels)).c_str());
    return;
  }

  if (sample_count > length) {
    logWarningC(("[" + filename + "] File is too long (" + std::to_string(sample_count)
                + " to fit in the requested size (" + std::to_string(length) + ")").c_str());
  } else if (sample_count < length) {
    length = sample_count;
  }

  if (!mathut::equals(sample_rate, header.nSamplesPerSec)) {
    logWarningC(("[" + filename + "] File's sample rate (" + std::to_string(header.nSamplesPerSec)
                + " differs from expected (" + std::to_string(sample_rate) + ")").c_str());
  }

  for (int64_t i = 0; i < length; ++i) {
    bufL->data_pointer[i] += file_data[i*2] / 32767.0;
    bufR->data_pointer[i] += file_data[i*2+1] / 32767.0;
  }
}
