//
// Created by rutio on 27.04.19.
//

#include "BufferNode.h"

BufferNode::BufferNode(double sample_rate)
    : buffer(sample_rate),
      sample_rate(sample_rate) {
}

Buffer* BufferNode::getBuffer() {
  return &buffer;
}

void BufferNode::process(const std::vector<const Buffer::TransformData*>& buffers,
                         const std::vector<const ParData*>& arguments) {

}

void BufferNode::deallocate() {
  buffer.deallocate();
}

double BufferNode::getSampleRate() {
  return buffer.getSampleRate();
}

int64_t BufferNode::getLengthSamples() {
  return buffer.getSize();
}
