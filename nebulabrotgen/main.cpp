#include <thread>
#include <array>
#include <cmath>
#include <complex>
#include "libnebulabrotgen.hpp"

typedef float real_t;
typedef uint16_t counter_t;

const real_t xmid = 0;
const real_t ymid = -0.5;
const real_t size = 3;
const size_t width = 600;
const size_t height = 600;
const size_t iterations = 50000000 / 1000;
const real_t random_radius = 4;
const real_t norm_limit = 4;

inline real_t limit(real_t value) {
  return std::min((real_t) 1.0, std::max((real_t) 0.0, value));
}

inline real_t mapv(real_t value, real_t in_diff, real_t out_diff) {
  return value * out_diff / in_diff;
}

typedef std::complex<real_t> complex;

void func(std::complex<real_t>& z, std::complex<real_t> c) {
  z = z * z * complex(0, -1) + c;
  //z = std::pow(z, std::complex<real_t>(z.real(), z.imag()))+ c;
}

uint32_t img_func(real_t* values) {
  uint8_t result[4];
  result[0] = (uint8_t) (255.0 * limit(
      values[3] * 0.375 + sqrt(values[4] * 0.375) + sqrt(values[5]) * 0.5 + sqrt(values[6]) * 0.675));
  result[1] = (uint8_t) (255.0 * limit(
      values[1] * 0.375 + sqrt(values[2]) * 0.375 + sqrt(values[3]) * 0.5 + sqrt(values[4]) * 0.375 +
      values[5] * 0.375));
  result[2] = (uint8_t) (255.0 * limit(
      sqrt(values[0]) * 0.625 + sqrt(values[1]) * 0.5 + sqrt(values[2]) * 0.375 + values[3] * 0.375));
  result[3] = 0xff;
  return *(uint32_t*) result;
}

uint32_t img_func2(real_t* values) {
  uint8_t result[4];
  result[0] = (uint8_t) (255.0 * tanh(
      values[3] * 0.375 + sqrt(values[4] * 0.375) + sqrt(values[5]) * 0.5 + sqrt(values[6]) * 0.675));
  result[1] = (uint8_t) (255.0 * tanh(
      values[1] * 0.375 + sqrt(values[2]) * 0.375 + sqrt(values[3]) * 0.5 + sqrt(values[4]) * 0.375 +
      values[5] * 0.375));
  result[2] = (uint8_t) (255.0 * tanh(
      sqrt(values[0]) * 0.625 + sqrt(values[1]) * 0.5 + sqrt(values[2]) * 0.375 + values[3] * 0.375));
  result[3] = 0xff;
  return *(uint32_t*) result;
}

uint32_t img_func_rgb(real_t* values) {
  uint8_t result[4];
  result[0] = (uint8_t) std::min(255.0, 255.0 * 3 * values[0]);
  result[1] = (uint8_t) std::min(255.0, 255.0 * 3 * values[1]);
  result[2] = (uint8_t) std::min(255.0, 255.0 * 3 * values[2]);
  result[3] = 0xff;
  return *(uint32_t*) result;
}

uint32_t img_monochrome(real_t* values) {
  uint8_t result[4];
  real_t val = sqrt(values[0]);
  result[0] = (uint8_t) std::min(255.0 * val, 255.0);
  result[1] = (uint8_t) std::min(255.0 * val, 255.0);
  result[2] = (uint8_t) std::min(255.0 * val, 255.0);
  result[3] = 0xff;
  return *(uint32_t*) result;
}

void func_whole(size_t num_pixels, uint32_t** pixels, uint32_t* channels_max, uint32_t* result) {
  for (size_t i = 0; i < num_pixels; ++i) {
    uint8_t* c = (uint8_t*) (result + i);
    c[0] = (uint8_t) (255.0 * pixels[0][i] / channels_max[0]);
    c[1] = (uint8_t) (255.0 * pixels[0][i] / channels_max[0]);
    c[2] = (uint8_t) (255.0 * pixels[0][i] / channels_max[0]);
    c[3] = 0xff;
  }
}

int main() {
  size_t threads = std::thread::hardware_concurrency();

  namespace nbg = nebulabrotgen;

  nbg::NebulabrotRenderingManager<real_t, counter_t> manager(
      xmid, ymid, size, random_radius, norm_limit, width, height, threads);
  manager.add("i1", nbg::NebulabrotIterationData(
      800, iterations, nbg::InnerFunctionData<real_t>(func, 1)));
  manager.add("i2", nbg::NebulabrotIterationData(
      200, iterations, nbg::InnerFunctionData<real_t>(func, 1)));
  manager.add("i3", nbg::NebulabrotIterationData(
      50, iterations, nbg::InnerFunctionData<real_t>(func, 1)));
  auto collection = manager.execute();

  nbg::ImageRenderingManager img_manager(threads);
  img_manager.add("output/color",
                  nbg::ImageOutputData(
                      nbg::ImageFunctionData(img_func_rgb,
                                             {"i1", "i2", "i3"},
                                             {}), &collection));
  img_manager.add("output/c1_red",
                  nbg::ImageOutputData(
                      nbg::ImageFunctionData(img_monochrome,
                                             {"i1"},
                                             {}), &collection));
  img_manager.add("output/c2_green",
                  nbg::ImageOutputData(
                      nbg::ImageFunctionData(img_monochrome,
                                             {"i2"},
                                             {}), &collection));
  img_manager.add("output/c3_blue",
                  nbg::ImageOutputData(
                      nbg::ImageFunctionData(img_monochrome,
                                             {"i3"},
                                             {}), &collection));
  img_manager.execute();

  return 0;
}
