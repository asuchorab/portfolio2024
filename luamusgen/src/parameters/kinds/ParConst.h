//
// Created by rutio on 2019-05-15.
//

#ifndef LUAMUSGEN_PARCONST_H
#define LUAMUSGEN_PARCONST_H

#include <parameters/Parameter.h>

class ParConst : public ParGenerator {
public:
  explicit ParConst(double value);

  std::unique_ptr<ParData> generate(const std::vector<const ParData*>& arguments, double sample_rate,
                                    int64_t start_point, int64_t length) override;

  MergeResult merge(const ParGenerator& other) override;

  double getValue() const;

private:
  double value;
};


#endif //LUAMUSGEN_PARCONST_H
