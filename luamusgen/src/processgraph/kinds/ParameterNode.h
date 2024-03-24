//
// Created by rutio on 26.04.19.
//

#ifndef LUAMUSGEN_PARAMETERNODE_H
#define LUAMUSGEN_PARAMETERNODE_H

#include <parameters/Parameter.h>
#include <processgraph/ProcessNode.h>

/**
 * Node that describes parameters for transforms.
 */
class ParameterNode : public ProcessNode {
public:
  explicit ParameterNode(std::unique_ptr<Parameter> parameter);
  double getSampleRate() override;
  int64_t getLengthSamples() override;
  int64_t getOffset() override;
  void registerInterval(int64_t start_sample, int64_t length);
  std::unique_ptr<ParData> getData(int64_t start_sample, int64_t length) const;
  std::pair<bool, double> getConstValue();

protected:
  void process(const std::vector<const Buffer::TransformData*>& buffers,
               const std::vector<const ParData*>& arguments) override;
  void deallocate() override;

private:
  std::unique_ptr<Parameter> parameter;
  ParameterNode* parameter_view_node = nullptr;

  friend class ProcessGraph;
  friend class ProcessNode;
};


#endif //LUAMUSGEN_PARAMETERNODE_H
