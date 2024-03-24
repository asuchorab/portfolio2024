//
// Created by rutio on 26.04.19.
//

#include <util/Logger.h>
#include "TransformNode.h"

TransformNode::TransformNode(std::unique_ptr<Transform> transform, Transform::Type type,
                             double sample_rate, double length)
    : transform(std::move(transform)), type(type),
      sample_rate(sample_rate),
      length_samples(static_cast<int64_t>(length*sample_rate)) {
  if (!this->transform) {
    logErrorC("provided null pointer");
  }
  if (length <= 0) {
    logWarningC("Length is nonpositive");
  }
  if (sample_rate <= 0) {
    logWarningC("Sample rate is nonpositive");
  }
  if (length_samples == 0) {
    logWarningC("Length in samples is 0 (sample rate: %f, length: %f)", sample_rate, length);
  }
}

void TransformNode::process(const std::vector<const Buffer::TransformData*>& buffers,
                            const std::vector<const ParData*>& arguments) {
  if (!transform) {
    logErrorC("null transform");
    return;
  }
  transform->verifyAndApply(type, getSampleRate(), getLengthSamples(), buffers, arguments);
}

void TransformNode::deallocate() {
  transform.reset();
}

double TransformNode::getSampleRate() {
  return sample_rate;
}

int64_t TransformNode::getLengthSamples() {
  return length_samples;
}
