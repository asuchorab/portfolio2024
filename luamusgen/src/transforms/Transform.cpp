//
// Created by rutio on 26.04.19.
//

#include <util/Logger.h>
#include "Transform.h"

void Transform::verifyAndApply(Transform::Type type, double sample_rate, int64_t length,
                               const std::vector<const Buffer::TransformData*>& buffers,
                               const std::vector<const ParData*>& arguments) {
  size_t buf_count = buffers.size();
  switch (type) {
    case MONO_IN_PLACE: {
      if (buf_count == 1) {
        applyMonoInPlace(sample_rate, length, buffers[0], arguments);
      } else {
        logErrorC(("bad number of buffers for type mono in place (expected 1, got " + std::to_string(buf_count) + ")").c_str());
      }
      return;
    }
    case STEREO_IN_PLACE: {
      if (buf_count == 2) {
        applyStereoInPlace(sample_rate, length, buffers[0], buffers[1], arguments);
      } else {
        logErrorC(("bad number of buffers for type stereo in place (expected 2, got " + std::to_string(buf_count) + ")").c_str());
      }
      return;
    }
    case MONO_TO_MONO: {
      if (buf_count == 2) {
        applyMonoToMono(sample_rate, length, buffers[0], buffers[1], arguments);
      } else {
        logErrorC(("bad number of buffers for type mono to mono (expected 2, got " + std::to_string(buf_count) + ")").c_str());
      }
      return;
    }
    case STEREO_TO_STEREO: {
      if (buf_count == 4) {
        applyStereoToStereo(sample_rate, length, buffers[0], buffers[1], buffers[2], buffers[3], arguments);
      } else {
        logErrorC(
            ("bad number of buffers for type stereo to stereo (expected 4, got " + std::to_string(buf_count) + ")").c_str());
      }
      return;
    }
    case MONO_TO_STEREO: {
      if (buf_count == 3) {
        applyMonoToStereo(sample_rate, length, buffers[0], buffers[1], buffers[2], arguments);
      } else {
        logErrorC(("bad number of buffers for type mono to stereo (expected 3, got " + std::to_string(buf_count) + ")").c_str());
      }
      return;
    }
    case STEREO_TO_MONO: {
      if (buf_count == 3) {
        applyStereoToMono(sample_rate, length, buffers[0], buffers[1], buffers[2], arguments);
      } else {
        logErrorC(("bad number of buffers for type stereo to mono (expected 3, got " + std::to_string(buf_count) + ")").c_str());
      }
      return;
    }
  }
}

void Transform::applyMonoInPlace(double sample_rate, int64_t length, buf_t buf, const std::vector<arg_t>& arguments) {
  logErrorC("not defined for this type");
}

void Transform::applyStereoInPlace(double sample_rate, int64_t length, buf_t bufL, buf_t bufR,
                                   const std::vector<arg_t>& arguments) {
  logErrorC("not defined for this type");
}

void Transform::applyMonoToMono(double sample_rate, int64_t length, buf_t in, buf_t out,
                                const std::vector<arg_t>& arguments) {
  logErrorC("not defined for this type");
}

void Transform::applyStereoToStereo(double sample_rate, int64_t length, buf_t inL, buf_t inR,
                                    buf_t outL, buf_t outR, const std::vector<arg_t>& arguments) {
  logErrorC("not defined for this type");
}

void Transform::applyMonoToStereo(double sample_rate, int64_t length, buf_t in, buf_t outL, buf_t outR,
                                  const std::vector<arg_t>& arguments) {
  logErrorC("not defined for this type");
}

void Transform::applyStereoToMono(double sample_rate, int64_t length, buf_t inL, buf_t inR, buf_t out,
                                  const std::vector<arg_t>& arguments) {
  logErrorC("not defined for this type");
}

void Transform::getStepMono() {
  logErrorC("step not defined for this type");
}

void Transform::getStepStereo() {
  logErrorC("step not defined for this type");
}
