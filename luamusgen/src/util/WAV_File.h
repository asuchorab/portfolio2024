//
// Created by rutio on 17.01.19.
//

#ifndef LUAMUSGEN_WAVHEADER_H
#define LUAMUSGEN_WAVHEADER_H

struct WAV_Header_fmt16 {
  char riff_ckID[4] = {'R','I','F','F'};
  unsigned int riff_cksize = 36;
  char riff_WAVEID[4] = {'W','A','V','E'};
  char fmt_ckID[4] = {'f','m','t',' '};
  unsigned int fmt_cksize = 16;
  unsigned short wFormatTag = 1;
  unsigned short nChannels = 2;
  unsigned int nSamplesPerSec = 44100;
  unsigned int nAvgBytesPerSec = 176400;
  unsigned short nBlockAlign = 4;
  unsigned short wBitsPerSample = 16;
  char data_ckID[4] = {'d','a','t','a'};
  unsigned int data_cksize = 0;

  bool isValid();
  void setSize(unsigned int samples);
  void setChannelsAndSPS(unsigned short channels, unsigned int samples_per_second);
  unsigned int getSamplesCount();
};

#endif //LUAMUSGEN_WAVHEADER_H
