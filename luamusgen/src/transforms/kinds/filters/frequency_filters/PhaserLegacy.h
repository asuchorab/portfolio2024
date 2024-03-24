//
// Created by rutio on 2019-08-01.
//

#ifndef LUAMUSGEN_PHASERLEGACY_H
#define LUAMUSGEN_PHASERLEGACY_H

#include <transforms/Transform.h>

class PhaserLegacy : public Transform {
public:
  explicit PhaserLegacy(int64_t stages);

  void applyMonoInPlace(double sample_rate, int64_t length, buf_t buf,
                        const std::vector<arg_t>& arguments) override;

  void applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                          const std::vector<arg_t>& arguments) override;

private:
  size_t stages;

  template<class T1, class T2, class T3>
  struct FunctorMono;
  template<class T1, class T2, class T3>
  struct FunctorStereo;
};

#endif //LUAMUSGEN_PHASERLEGACY_H
