//
// Created by rutio on 26.02.19.
//

#ifndef LUAMUSGEN_TEST_BLP_H
#define LUAMUSGEN_TEST_BLP_H

void test_blp() {
  /*
  timeutil::point t1 = timeutil::now();

  double len = 20;
  double sample_rate = 44100;

  Buffer buf1(sample_rate);
  buf1.requestSpace(0, len);
  buf1.allocate();

  MonoWhiteNoise gen1;

  auto tr1 = buf1.registerTransform(0, len);
  gen1.apply(tr1->data_pointer, tr1->length, tr1->sample_rate);

  ParLinear par1(1000, 200, len);
  ParDataConst par1C = 200;
  ParLinear par1a(2000, 400, len);
  ParDataConst par1aC = 2000;
  ParLinear par2(0.99, 0, len);
  ParDataConst par2C = 0.5;
  ParLinear par3(0.95, 1.0, len);
  ParDataConst par3C = 1.0;
  par1.setLength(len, sample_rate);
  par1a.setLength(len, sample_rate);
  par2.setLength(len, sample_rate);
  par3.setLength(len, sample_rate);
  par1.compute();
  par1a.compute();
  par2.compute();
  par3.compute();

  ButterworthBandStop fi1(8, par1a, par1C, par2C, par3C);
  fi1.apply(tr1->data_pointer, tr1->length, tr1->sample_rate);

  WaveOld wave1(sample_rate);
  wave1.requestSpace(0, len);
  wave1.allocate();

  auto tr2L = wave1.registerTrasformL(0, len);
  auto tr2R = wave1.registerTrasformR(0, len);

  CopyBufferStereo op1(tr1->data_pointer, 1.0);
  op1.apply(tr2L->data_pointer, tr2R->data_pointer, tr2L->length, tr2R->sample_rate);
  tr2L->release();
  tr2R->release();

  tr1->release();

  std::cout<<"Time: "<<timeutil::duration(t1)<<std::endl;

  wave1.outputToFile("test_blp.wav");*/
}

#endif //LUAMUSGEN_TEST_BLP_H
