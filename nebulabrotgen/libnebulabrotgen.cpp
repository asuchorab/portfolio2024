#include "libnebulabrotgen.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <filesystem>

namespace nebulabrotgen {

ImageColorBuffer::ImageColorBuffer(size_t width, size_t height)
    : width(width), height(height), data(width*height) {}


bool ImageColorBuffer::saveFile(const std::string& filename) {
  std::string actual_filename = filename;
  while(std::filesystem::exists(actual_filename + ".png")) {
    actual_filename += '_';
  }
  bool success =  (bool) stbi_write_png((actual_filename + ".png").c_str(), (int) width, (int) height, 4, data.data(), (int) width*4);
  if (success) {
    std::cout<<"Saved image " + actual_filename + "\n";
  } else {
    std::cout<<"Failed to save image " + actual_filename + "\n";
  }
  return success;
}

}

