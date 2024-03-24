//
// Created by rutio on 27.07.19.
//

#include <processgraph/kinds/BufferNode.h>
#include <parameters/kinds/ParLinearExp.h>

namespace test_linexp {

  void test() {

    setLoggerLevel(3);

    ProcessGraph graph;

    auto nodeToWAV = new TransformNode(std::make_unique<BufferToWAV>("test_linexp.wav"), Transform::MONO_IN_PLACE, 44100, 4);
    auto nodeBuf1 = new BufferNode(44100);

    {
      double length = 1;
      auto nodeSignal = new TransformNode(std::make_unique<ConstantSignal>(), Transform::MONO_IN_PLACE, 44100, length);
      auto nodeParC_0 = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParConst>(0.1), 44100));
      auto nodeParC_1 = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParConst>(1), 44100));
      auto nodeParLinear = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLerp>(0, length), 44100));
      auto nodeParLerp2 = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLerp>(0, length), 44100));
      auto nodeParLerp3 = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLerp>(0, length), 44100));
      auto nodeParLerp4 = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLerp>(0, length), 44100));
      auto nodeParLinexp = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLinearExp>(0.1, 1), 44100));

      auto par1 = graph.addNode(nodeParC_0);
      auto par2 = graph.addNode(nodeParC_1);

      nodeParLinear->addArgument(0, par1);
      nodeParLinear->addArgument(0, par2);

      auto par3 = graph.addNode(nodeParLinear);

      nodeParLerp2->addArgument(0, par1);
      nodeParLerp2->addArgument(0, par3);

      auto par4 = graph.addNode(nodeParLerp2);

      nodeParLerp3->addArgument(0, par3);
      nodeParLerp3->addArgument(0, par2);

      auto par5 = graph.addNode(nodeParLerp3);

      nodeParLerp4->addArgument(0, par4);
      nodeParLerp4->addArgument(0, par5);

      auto par6 = graph.addNode(nodeParLerp4);

      nodeParLinexp->addArgument(0, par6);

      auto par7 = graph.addNode(nodeParLinexp);

      nodeSignal->addArgument(0, par7);
      nodeSignal->addBuffer(0, nodeBuf1);
      nodeBuf1->addPrerequisite(nodeSignal);

      graph.addNode(nodeSignal);
    }

    nodeToWAV->addBuffer(0, nodeBuf1);
    nodeToWAV->addPrerequisite(nodeBuf1);

    graph.addNode(nodeBuf1);
    graph.addNode(nodeToWAV);

    graph.execute();
  }

}

