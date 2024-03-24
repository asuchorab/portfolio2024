//
// Created by rutio on 13.08.2019.
//

#ifndef LUAMUSGEN_BUFFERTORAW_H
#define LUAMUSGEN_BUFFERTORAW_H

#include <transforms/Transform.h>

class BufferToRaw : public Transform {
public:
  explicit BufferToRaw(std::string filename);

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override;
  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override;

private:
  std::string filename;
};

#endif //LUAMUSGEN_BUFFERTORAW_H
