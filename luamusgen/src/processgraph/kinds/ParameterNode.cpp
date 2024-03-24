//
// Created by rutio on 26.04.19.
//

#include <util/Logger.h>
#include <parameters/kinds/ParConst.h>
#include "ParameterNode.h"

ParameterNode::ParameterNode(std::unique_ptr<Parameter> parameter)
    : parameter(std::move(parameter)) {
  if (!this->parameter) {
    logErrorC("provided null transform");
  }
}

void ParameterNode::process(const std::vector<const Buffer::TransformData*>& buffers,
                            const std::vector<const ParData*>& arguments) {
  if (!parameter) {
    logErrorC("null parameter");
    return;
  }
  parameter->compute(arguments);
}

void ParameterNode::registerInterval(int64_t start_sample, int64_t length) {
  if (length <= 0) {
    logErrorC("length is nonpositive");
    return;
  }
  parameter->registerInterval(start_sample, length);
  for (auto& arg : arguments) {
    auto limits = arg.getLimits(start_sample, length);
    arg.node->registerInterval(limits.first, limits.second);
  }
  if (parameter->isView() && parameter_view_node) {
    parameter_view_node->registerInterval(start_sample + parameter->getOffset(), length);
  }
}

std::unique_ptr<ParData> ParameterNode::getData(int64_t start_sample, int64_t length) const {
  if (!parameter) {
    logErrorC("null parameter");
    return nullptr;
  }
  return parameter->getData(start_sample, length);
}

std::pair<bool, double> ParameterNode::getConstValue() {
  return parameter->getConstValue();
}

void ParameterNode::deallocate() {
  parameter.reset();
}

double ParameterNode::getSampleRate() {
  return parameter->getSampleRate();
}

int64_t ParameterNode::getLengthSamples() {
  return parameter->getLengthSamples();
}

int64_t ParameterNode::getOffset() {
  return parameter->getOffset();
}
