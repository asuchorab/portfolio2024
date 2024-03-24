//
// Created by rutio on 26.04.19.
//

#include "ProcessNode.h"
#include "kinds/ParameterNode.h"
#include "kinds/BufferNode.h"
#include <util/Logger.h>
#include <util/MathUtil.h>

std::pair<int64_t, int64_t> ProcessNode::ArgumentData::getLimits(int64_t offset, int64_t base_length) {
  offset = fixed_start_point ? rel_start_point : rel_start_point + offset;
  base_length = hasLength() ? alt_length : base_length;
  int64_t end_point = offset + base_length;
  int64_t start_point;
  if (touch_zero) {
    start_point = std::min((int64_t) 0, offset);
    end_point = std::max((int64_t) 0, end_point);
  } else {
    start_point = offset;
  }
  return std::make_pair(start_point, end_point - start_point);
}

int64_t ProcessNode::ArgumentData::getStartPoint(int64_t offset) {
  offset = fixed_start_point ? rel_start_point : rel_start_point + offset;
  return touch_zero ? std::min((int64_t) 0, offset) : offset;
}

int64_t ProcessNode::ArgumentData::getLength(int64_t base_length) {
  base_length = hasLength() ? alt_length : base_length;
  return touch_zero ? std::max((int64_t) 0, base_length) : base_length;
}

void ProcessNode::addArgument(double start_point, ProcessNode* node, bool fixed_start_point, double other_length,
                              bool touch_zero) {
  if (!node) {
    logErrorC("argument is nullptr (nil)");
    return;
  }
  int64_t offset = getOffset();
  int64_t length_samples = getLengthSamples();
  /*
  if (locked_prerequisites) {
    logErrorC("cannot add an argument because prerequisites are locked (node is already inserted)");
    return;
  }
  if (!node->locked_prerequisites) {
    logErrorC("cannot add an argument that doesn't have its prerequisites locked (isn't inserted yet)");
    return;
  }*/
  if (auto p = dynamic_cast<ParameterNode*>(node)) {
    double sample_rate = getSampleRate();
    double other_sample_rate = node->getSampleRate();
    if (sample_rate <= 0) {
      logErrorC("node has no sample rate");
      return;
    }
    if (!mathut::equals(sample_rate, other_sample_rate)) {
      logErrorC("sample rates don't match (%f != %f)", sample_rate, other_sample_rate);
      return;
    }
    int64_t start_sample = static_cast<int64_t>(start_point * sample_rate);
    if (other_length > -1e17) {
      if (other_length <= 0) {
        logErrorC("desired other length is less or equal 0");
        return;
      }
      int64_t actual_length = static_cast<int64_t>(sample_rate * other_length);
      if (actual_length == 0) {
        logWarningC("parameter will be 0 samples long at this sample rate using other_length");
      }
      arguments.emplace_back(p, start_sample, fixed_start_point, actual_length, touch_zero);
      ArgumentData& data = arguments.back();
      auto limits = data.getLimits(offset, actual_length);
      p->registerInterval(limits.first, limits.second);
    } else {
      arguments.emplace_back(p, start_sample, fixed_start_point, std::numeric_limits<int64_t>::max(), touch_zero);
      if (length_samples > 0) {
        ArgumentData& data = arguments.back();
        auto limits = data.getLimits(offset, length_samples);
        p->registerInterval(limits.first, limits.second);
      }
    }
    ParameterNode* temp = p;
    while (temp) {
      addPrerequisite(temp);
      temp = temp->parameter_view_node;
    }
  } else {
    logErrorC("node is not a parameter");
  }
}

void ProcessNode::addBuffer(double start_point, ProcessNode* node, double other_length, bool allow_other_sample_rate) {
  if (!node) {
    logErrorC("buffer is nullptr (nil)");
    return;
  }
  int64_t length_samples = getLengthSamples();
  if (length_samples == 0) {
    logWarningC("Node has no length");
  }
  if (auto p = dynamic_cast<BufferNode*>(node)) {
    int64_t final_start;
    int64_t final_length;
    double final_sample_rate;
    double sample_rate = getSampleRate();
    double other_sample_rate = node->getSampleRate();
    if (sample_rate != other_sample_rate) {
      if (allow_other_sample_rate) {
        final_sample_rate = other_sample_rate;
      } else {
        logErrorC(("sample rates don't match (" + std::to_string(sample_rate)
                  + " != " + std::to_string(other_sample_rate)).c_str());
        return;
      }
    } else {
      final_sample_rate = sample_rate;
    }
    final_start = static_cast<int64_t>(start_point * final_sample_rate);
    if (other_length > 0) {
      final_length = static_cast<int64_t>(other_length * final_sample_rate);
    } else {
      if (other_length > -1e17) {
        logWarningC("other length is less or equal 0")
      }
      final_length = length_samples;
    }
    //p->addPrerequisite(this);
    p->getBuffer()->requestSpace(final_start, final_length);
    buffers.emplace_back(p->getBuffer(), p, final_start, final_length);
  } else {
    logErrorC("node is not a buffer");
  }
}

void ProcessNode::addPrerequisite(ProcessNode* node) {
  if (!node) {
    logErrorC("prerequisite is nullptr (nil)");
    return;
  }
  /*
  if (locked_prerequisites) {
    logErrorC("cannot add a prerequisite because prerequisites are locked (node is already inserted)");
    return;
  }*/
  state = INITIAL;
  if (prerequisites.find(node) != prerequisites.end()) {
    return;
  }
  prerequisites.insert(node);
  not_completed_prerequisites_counter++;
  node->dependent_nodes.insert(this);
}

bool ProcessNode::complete() {
  {
    std::lock_guard<std::mutex> lock(state_mutex);

    switch (state) {
      case INITIAL:debugProcessC("tried to complete %s while INITIAL", display_code.c_str());
        return false;
      case COMPLETED:debugProcessC("tried to complete %s while COMPLETE", display_code.c_str());
        return false;
      case DEAD:debugProcessC("tried to complete %s while DEAD", display_code.c_str());
        return false;
      default:;
    }

    if (!is_being_processed) {
      is_being_processed = true;
    } else { debugProcessC("tried to complete %s while it was being processed", display_code.c_str());
      return false;
    }
  }


  std::vector<const ParData*> temp_arguments;
  std::vector<std::unique_ptr<ParData>> temp_parameter_data; //for storage
  std::vector<const Buffer::TransformData*> temp_buffers;

  bool arg_fail = false;
  bool buf_fail = false;

  int64_t offset = getOffset();
  int64_t length_samples = getLengthSamples();

  for (auto arg : arguments) {
    auto limits = arg.getLimits(offset, length_samples);
    std::unique_ptr<ParData> arg_data = arg.node->getData(limits.first, limits.second);
    if (arg_data) {
      temp_arguments.push_back(arg_data.get());
      temp_parameter_data.emplace_back(std::move(arg_data));
    } else {
      logErrorC("could not get argument data for node %s from argument %s",
                display_code.c_str(), arg.node->display_code.c_str());
      arg_fail = true;
      break;
    }
  }
  for (auto buf : buffers) {
    auto tr_data = buf.ptr->registerTransform(buf.start_point, buf.length);
    if (tr_data) {
      temp_buffers.push_back(tr_data);
    } else {
      for (auto t : temp_buffers) {
        t->release();
      }

      debugProcessC(
          "could not register transform for node %s in buffer %s, probably due to a conflict with another thread",
          display_code.c_str(), buf.node->display_code.c_str());
      buf_fail = true;
      break;
    }
  }

  if (buf_fail) {
    std::lock_guard<std::mutex> lock(state_mutex);
    is_being_processed = false;
    return false;
  }

  if (!arg_fail) {

    process(temp_buffers, temp_arguments);
  } else { debugProcessC("Failed to process node %s", display_code.c_str());
  }

  for (auto t : temp_buffers) {
    t->release();
  }

  debugProcessC("Completed node %s", display_code.c_str());

  for (auto node : prerequisites) {
    std::lock_guard<std::mutex> lock(node->state_mutex);
    std::lock_guard<std::mutex> lock2(node->dependent_nodes_mutex);
    switch (node->state) {
      case INITIAL:
        logWarningC("tried to update dependent list of INITIAL type node %s from %s", node->display_code.c_str(),
                    display_code.c_str());
        break;
      case READY:
        logWarningC("tried to update dependent list of READY type node %s from %s, is being processed: %s",
                    node->display_code.c_str(), display_code.c_str(), node->is_being_processed ? "true" : "false");
        break;
      case DEAD:
        logWarningC("tried to update dependent list of DEAD type node %s from %s", node->display_code.c_str(),
                    display_code.c_str());
        break;
      default:
        auto it = node->dependent_nodes.find(this);
        if (it == node->dependent_nodes.end()) {
          logWarningC("not in dependent list of a prerequisite");
          continue;
        }
        node->dependent_nodes.erase(it);
        if (node->dependent_nodes.empty()) {
          node->state = DEAD;
          node->deallocate();debugProcessC("Deallocated node %s from %s", node->display_code.c_str(),
                                           display_code.c_str());
        }
    }
  }

  {
    std::lock_guard<std::mutex> lock(state_mutex);

    prerequisites.clear();
    is_being_processed = false;
    if (dependent_nodes.empty()) {
      state = DEAD;
      deallocate();debugProcessC("Deallocated node " + display_code);
    } else {
      state = COMPLETED;
    }
  }

  {
    std::lock_guard<std::mutex> lock(dependent_nodes_mutex);

    for (auto node : dependent_nodes) {
      std::lock_guard<std::mutex> lock2(node->prerequisites_mutex);
      std::lock_guard<std::mutex> lock3(node->state_mutex);
      switch (node->state) {
        case READY:
          logWarningC("tried to update prerequisite counter of READY type node %s from %s", node->display_code.c_str(),
                      display_code.c_str());
          break;
        case COMPLETED:
          logWarningC("tried to update prerequisite counter of COMPLETED type node %s from %s",
                      node->display_code.c_str(), display_code.c_str());
          break;
        case DEAD:
          logWarningC("tried to update prerequisite counter of DEAD type node %s from %s", node->display_code.c_str(),
                      display_code.c_str());
          break;
        default:
          auto it = node->prerequisites.find(this);
          if (it == node->prerequisites.end()) {
            logWarningC(("tried to update prerequisite counter of " + node->display_code + " but it doesn't have " +
                        display_code + " in its prerequisite list").c_str());
            continue;
          }
          node->not_completed_prerequisites_counter--;
          if (node->not_completed_prerequisites_counter == 0) {
            node->state = READY;
          }

      }
    }
  }
  return true;
}

double ProcessNode::getSampleRate() {
  return 0;
}

int64_t ProcessNode::getLengthSamples() {
  return 0;
}

int64_t ProcessNode::getOffset() {
  return 0;
}

/*
bool ProcessNode::setSampleRate(double new_sample_rate) {
  if (!std::isfinite(new_sample_rate)) {
    logErrorC("needs to be finite");
    return false;
  }
  if (sample_rate > 0) {
    logErrorC("already has a sample rate");
    return false;
  }
  if (new_sample_rate <= 0) {
    logErrorC("needs to be positive");
    return false;
  }
  sample_rate = new_sample_rate;
  return true;
}

bool ProcessNode::setLength(double new_length) {
  if (!std::isfinite(new_length)) {
    logErrorC("needs to be finite");
    return false;
  }
  if (length_samples > 0) {
    logErrorC("already has a length");
    return false;
  }
  if (new_length <= 0) {
    logErrorC("needs to be positive");
    return false;
  }
  if (sample_rate <= 0) {
    logErrorC("no sample rate");
    return false;
  }
  length = new_length;
  length_samples = static_cast<int64_t>(new_length * sample_rate);
  return true;
}

double ProcessNode::getLength() {
  return sample_rate * getLengthSamples();
}
*/
