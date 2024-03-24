//
// Created by rutio on 26.04.19.
//

#ifndef LUAMUSGEN_PARAMETERTRANSFORM_H
#define LUAMUSGEN_PARAMETERTRANSFORM_H

#include <transforms/Transform.h>
#include <processgraph/ProcessNode.h>

/**
 * Node that describes an action done on audio buffer
 */
class TransformNode : public ProcessNode {
public:
  TransformNode(std::unique_ptr<Transform> transform, Transform::Type type, double sample_rate, double length);
  double getSampleRate() override;
  int64_t getLengthSamples() override;

protected:
  void process(const std::vector<const Buffer::TransformData*>& buffers,
               const std::vector<const ParData*>& arguments) override;
  void deallocate() override;

private:
  std::unique_ptr<Transform> transform;
  Transform::Type type;
  double sample_rate;
  int64_t length_samples;

  friend class ProcessGraph;
};


#endif //LUAMUSGEN_PARAMETERTRANSFORM_H
