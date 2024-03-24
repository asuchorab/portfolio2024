//
// Created by rutio on 13.08.2019.
//

#ifndef LUAMUSGEN_BUFFERFROMRAW_H
#define LUAMUSGEN_BUFFERFROMRAW_H

#include <transforms/Transform.h>

class BufferFromRaw : public Transform {
public:
  explicit BufferFromRaw(std::string filename);

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override;
  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override;

private:
  std::string filename;
};

#endif //LUAMUSGEN_BUFFERFROMRAW_H
