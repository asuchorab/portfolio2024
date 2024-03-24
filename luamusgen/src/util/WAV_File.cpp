//
// Created by rutio on 17.01.19.
//

#include <util/WAV_File.h>
#include "WAV_File.h"
#include <util/Logger.h>
#include <string>

void WAV_Header_fmt16::setSize(unsigned int samples) {
  data_cksize = samples * nBlockAlign;
  riff_cksize = 36 + data_cksize;
}

void WAV_Header_fmt16::setChannelsAndSPS(unsigned short channels, unsigned int samples_per_second) {
  unsigned int samples = (riff_cksize - 36) / nBlockAlign;
  unsigned short bytesPerSample = nBlockAlign / nChannels;
  nChannels = channels;
  nSamplesPerSec = samples_per_second;
  nBlockAlign = bytesPerSample * nChannels;
  nAvgBytesPerSec = nBlockAlign * nSamplesPerSec;
  setSize(samples);
}

unsigned int WAV_Header_fmt16::getSamplesCount() {
  return data_cksize / nBlockAlign;
}

bool WAV_Header_fmt16::isValid() {
  if (riff_ckID[0] != 'R') return false;
  if (riff_ckID[1] != 'I') return false;
  if (riff_ckID[2] != 'F') return false;
  if (riff_ckID[3] != 'F') return false;
  if (fmt_ckID[0] != 'f') return false;
  if (fmt_ckID[1] != 'm') return false;
  if (fmt_ckID[2] != 't') return false;
  if (fmt_ckID[3] != ' ') return false;

  if (wFormatTag != 1) {
    logErrorC(
        ("Currently not supporting any format except 16 bit PCM (format code is " + std::to_string(wFormatTag) + ")").c_str());
    return false;
  }

  if (nChannels < 1 || nChannels > 2) {
    logErrorC("Number of channels must be 1 or 2");
    return false;
  }

  if (wBitsPerSample != 16) {
    logErrorC(("Currently not supporting any format except 16 bit PCM (is " + std::to_string(wBitsPerSample) + "bit)").c_str());
    return false;
  }

  if (8 * nBlockAlign / nChannels != wBitsPerSample) {
    logErrorC("Inconsistent fmt chunk (8 * nBlockAlign / nChannels != wBitsPerSample)");
    return false;
  }

  if (nSamplesPerSec * nBlockAlign != nAvgBytesPerSec) {
    logErrorC("Inconsistent fmt chunk (nSamplesPerSec * nBlockAlign != nAvgBytesPerSec)");
    return false;
  }

  if (data_ckID[0] != 'd') return false;
  if (data_ckID[1] != 'a') return false;
  if (data_ckID[2] != 't') return false;
  if (data_ckID[3] != 'a') return false;

  if (data_cksize + 36 != riff_cksize) {
    logWarningC(("inconsistent RIFF and data chunk sizes(RIFF: " + std::to_string(riff_cksize) + ", data: " +
                std::to_string(data_cksize) + ")").c_str());
  }

  return true;
}
