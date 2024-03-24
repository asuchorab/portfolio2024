//
// Created by rutio on 31.03.19.
//

#include <util/Logger.h>
#include <util/MathUtil.h>
#include <parameters/kinds/ParConst.h>
#include "Parameter.h"
#include "ParDataConst.h"
#include "ParDataView.h"

Parameter::Parameter(std::unique_ptr<ParGenerator> generator, double sample_rate)
    : generator(std::move(generator)),
      view_target(nullptr),
      sample_rate(sample_rate) {
  if (!this->generator) {
    logErrorC("parameter generator is null")
  }
}

Parameter::Parameter(Parameter* view_target, double start_point)
    : view_target(view_target) {
  if (!view_target) {
    logErrorC("parameter view target is null");
  } else {
    sample_rate = view_target->sample_rate;
    offset = static_cast<int64_t>(sample_rate * start_point);
  }
}

void Parameter::registerInterval(int64_t start_sample, int64_t length) {
  if (length <= 0) {
    logWarningC(("requested nonpositive length: " + std::to_string(length)).c_str());
    return;
  }
  if (generator) {
    int64_t end = start_sample + length;
    if (offset == std::numeric_limits<int64_t>::max()) {
      offset = start_sample;
      end_offset = end;
    } else {
      offset = std::min(offset, start_sample);
      end_offset = std::max(end_offset, end);
    }
  } else {
    view_target->registerInterval(start_sample + offset, length);
  }
}

ParGenerator::MergeResultType Parameter::merge(Parameter& other) {
  if (!mathut::equals(sample_rate, other.sample_rate)) {
    return ParGenerator::FAILURE;
  }

  if (generator) {
    if (other.generator) {
      auto result = other.generator->merge(*generator);
      if (result.type != ParGenerator::FAILURE) {
        this->generator = nullptr;
        view_target = &other;
        if (result.type == ParGenerator::PARTIAL) {
          offset = static_cast<int64_t>(sample_rate * result.new_start);
        } else {
          offset = 0;
        }
      }
      return result.type;
    }
  }
  return ParGenerator::FAILURE;
}

void Parameter::compute(const std::vector<const ParData*>& arguments) {
  if (generator) {
    if (offset != std::numeric_limits<int64_t>::max()) {
      data = generator->generate(arguments, sample_rate, offset, end_offset-offset);
    } else {
      logWarningC("no registered interval (maybe the parameter is not used by anything)");
    }
  } else {
    data = view_target->getData(offset, end_offset-offset);
    if (!data) {
      logWarningC("view target did not return any data");
    }
  }
}

std::unique_ptr<ParData> Parameter::getData(int64_t start_sample, int64_t length) const {
  if (generator) {
    if (data) {
      if (auto p = dynamic_cast<ParDataConst*>(data.get())) {
        return std::make_unique<ParDataConst>(p->data.value, length, start_sample);
      } else {
        if (start_sample < offset) {
          logErrorC("requested start is below the lower limit of the time interval")
          return nullptr;
        }
        if (start_sample + length > end_offset) {
          logErrorC("requested end is above the upper limit of the time interval")
          return nullptr;
        }
        double* ptr = data->data.array + start_sample - offset;
        return std::make_unique<ParDataView>(ptr, length, start_sample);
      }
    } else {
      logErrorC("requested data before computing");
      return nullptr;
    }
  } else {
    if (!view_target) {
      logErrorC("Parameter has no parameter generator and no view target (does not point to any other)");
      return std::make_unique<ParDataConst>(1);
    }
    std::unique_ptr<ParData> result = view_target->getData(start_sample + offset, length);
    if (!result) {
      logWarningC("view target did not return any data");
    }
    return result;
  }
}

double Parameter::getSampleRate() {
  return sample_rate;
}

std::pair<bool, double> Parameter::getConstValue() {
  if (auto p = dynamic_cast<ParConst*>(generator.get())) {
    return {true, p->getValue()};
  } else {
    return {false, std::numeric_limits<double>::quiet_NaN()};
  }
}

int64_t Parameter::getLengthSamples() {
  if (offset == std::numeric_limits<int64_t>::max()) {
    return 0;
  } else {
    return end_offset - offset;
  }
}

std::string Parameter::getType() {
  if (generator) {
    return CLASS_NAME(*generator);
  } else {
    std::string acc;
    Parameter* temp = this;
    while (temp && !temp->generator) {
      acc += "view->";
      temp = temp->view_target;
    }
    if (temp) {
      acc += temp->getType();
    } else {
      acc += "nullptr";
    }
    return acc;
  }
}

int64_t Parameter::getOffset() {
  if (offset == std::numeric_limits<int64_t>::max()) {
    return 0;
  } else {
    return offset;
  }
}

bool Parameter::isView() {
  return view_target;
}
