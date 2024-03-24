//
// Created by rutio on 18.07.19.
//

#ifndef LUAMUSGEN_BUFFERTOWAV_H
#define LUAMUSGEN_BUFFERTOWAV_H

#include <transforms/Transform.h>

class BufferToWAV : public Transform {
public:
  explicit BufferToWAV(std::string filename);

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override;
  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override;

private:
  std::string filename;
};

#endif //LUAMUSGEN_BUFFERTOWAV_H
