//
// Created by rutio on 28.04.19.
//

#ifndef LUAMUSGEN_EMPTYNODE_H
#define LUAMUSGEN_EMPTYNODE_H


#include <processgraph/ProcessNode.h>

/**
 * Node that does nothing.
 */
class DummyNode : public ProcessNode {
protected:
  void process(const std::vector<const Buffer::TransformData*>& buffers,
               const std::vector<const ParData*>& arguments) override;
  void deallocate() override;
};


#endif //LUAMUSGEN_EMPTYNODE_H
