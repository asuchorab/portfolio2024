//
// Created by rutio on 27.04.19.
//

#ifndef LUAMUSGEN_BUFFERNODE_H
#define LUAMUSGEN_BUFFERNODE_H

#include <processgraph/ProcessNode.h>

/**
 * Node that holds audio buffer
 */
class BufferNode : public ProcessNode {
public:
  explicit BufferNode(double sample_rate);
  Buffer* getBuffer();
  double getSampleRate() override;
  int64_t getLengthSamples() override;

protected:
  void process(const std::vector<const Buffer::TransformData*>& buffers,
               const std::vector<const ParData*>& arguments) override;
  void deallocate() override;

private:
  Buffer buffer;
  double sample_rate;
};

#endif //LUAMUSGEN_BUFFERNODE_H
