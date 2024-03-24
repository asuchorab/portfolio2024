//
// Created by rutio on 26.02.19.
//

#include <buffer/Buffer.h>
#include <transforms/generators/MonoWhiteNoise.h>
#include <wave/WaveOld.h>
#include <transforms/stereo_operations/CopyBufferStereo.hpp>
#include <util/TimeUtil.h>
#include <transforms/filters/ChangeVolume.h>
#include <parameters/pararraytypes/ParLinear.h>
#include <parameters/pararraytypes/operators/ParAdd.h>
#include <iostream>

void test_basic_par() {
  timeutil::point t1 = timeutil::now();

  int64_t len = 20;
  int64_t sample_rate = 44100;

  Buffer buf1(sample_rate);
  buf1.requestSpace(0, len);
  buf1.allocate();

  MonoWhiteNoise gen1;

  auto tr1 = buf1.registerTransform(0, len);
  gen1.apply(tr1->data_pointer, tr1->length, tr1->sample_rate);

  std::cout<<"Noise: "<<timeutil::duration(t1)<<std::endl;

  t1 = timeutil::now();

  ParDataConst par1 = 0.7;

  ParLinear par2(0, 0.3);
  par2.setLength(len*sample_rate);
  par2.compute();

  ParAdd par3(par1, par2);
  par3.setLength(len*sample_rate);
  par3.compute();

  ChangeVolume fi1(par3);
  fi1.apply(tr1->data_pointer, tr1->length, tr1->sample_rate);

  std::cout<<"Change volume: "<<timeutil::duration(t1)<<std::endl;

  t1 = timeutil::now();

  WaveOld wave1(sample_rate);
  wave1.requestSpace(0, len);
  wave1.allocate();

  auto tr3L = wave1.registerTrasformL(0, len);
  auto tr3R = wave1.registerTrasformR(0, len);

  CopyBufferStereo op1(tr1->data_pointer, 1.0);
  op1.apply(tr3L->data_pointer, tr3R->data_pointer, tr3L->length, tr3R->sample_rate);
  tr3L->release();
  tr3R->release();

  tr1->release();
  std::cout<<"Copy: "<<timeutil::duration(t1)<<std::endl;

  wave1.outputToFile("test1.wav");
}

