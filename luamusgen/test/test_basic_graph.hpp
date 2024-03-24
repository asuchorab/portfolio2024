//
// Created by rutio on 19.07.19.
//

#include <processgraph/ProcessGraph.h>
#include <parameters/kinds/ParConst.h>
#include <transforms/kinds/generators/WhiteNoise.h>
#include <transforms/kinds/buffer_operations/BufferToWAV.h>
#include <iostream>
#include <thread>
#include <transforms/kinds/filters/other/ChangeVolume.h>
#include <parameters/kinds/ParLerp.h>
#include <transforms/kinds/generators/ConstantSignal.h>

namespace test_basic_graph {

  void test_thread_function(ProcessGraph* graph) {
    setLoggerLevel(3);
    graph->execute();
  }

  void lerp_test_signal(ProcessGraph* graph, BufferNode* buffer, double start_point, double length, double arg_offset = 0) {
    auto nodeSignal = new TransformNode(std::make_unique<ConstantSignal>(), Transform::MONO_IN_PLACE, 44100, length);
    auto nodeParC_0 = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParConst>(0.0), 44100));
    auto nodeParC_1 = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParConst>(0.25), 44100));
    auto nodeParLinear = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLerp>(0, length), 44100));
    auto nodeParLerp2 = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLerp>(0, length), 44100));
    auto nodeParLerp3 = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLerp>(0, length), 44100));
    auto nodeParLerp4 = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLerp>(-arg_offset, length-arg_offset), 44100));

    auto par1 = graph->addNode(nodeParC_0);
    auto par2 = graph->addNode(nodeParC_1);

    nodeParLinear->addArgument(0, par1);
    nodeParLinear->addArgument(0, par2);

    auto par3 = graph->addNode(nodeParLinear);

    nodeParLerp2->addArgument(0, par1);
    nodeParLerp2->addArgument(0, par3);

    auto par4 = graph->addNode(nodeParLerp2);

    nodeParLerp3->addArgument(0, par3);
    nodeParLerp3->addArgument(0, par2);

    auto par5 = graph->addNode(nodeParLerp3);

    nodeParLerp4->addArgument(0, par4);
    nodeParLerp4->addArgument(0, par5);

    auto par6 = graph->addNode(nodeParLerp4);

    nodeSignal->addArgument(0, par6);
    nodeSignal->addBuffer(start_point, buffer);
    buffer->addPrerequisite(nodeSignal);

    graph->addNode(nodeSignal);
  }

  void test() {

    setLoggerLevel(4);

    ProcessGraph graph;

    auto nodeToWAV = new TransformNode(std::make_unique<BufferToWAV>("test_basic_graph"), Transform::MONO_IN_PLACE, 44100, 40);
    auto nodeBuf1 = new BufferNode(44100);
    //auto nodeSignal = std::make_unique<TransformNode>(std::make_unique<ConstantSignal>(), Transform::MONO_IN_PLACE, 44100, 1);
    //auto nodsSignal2 = std::make_unique<TransformNode>(std::make_unique<ConstantSignal>(), Transform::MONO_IN_PLACE, 44100, 3);
    //auto nodeParC_0_5 = std::make_unique<ParameterNode>(std::make_unique<Parameter>(std::make_unique<ParConst>(0.0), 44100));
    //auto nodeParC_1 = std::make_unique<ParameterNode>(std::make_unique<Parameter>(std::make_unique<ParConst>(0.1), 44100));
    //auto nodeParLinear = std::make_unique<ParameterNode>(std::make_unique<Parameter>(std::make_unique<ParLerp>(0, 3), 44100));
    //auto nodeParLerp2 = std::make_unique<ParameterNode>(std::make_unique<Parameter>(std::make_unique<ParLerp>(0, 3), 44100));
    //auto nodeParLerp3 = std::make_unique<ParameterNode>(std::make_unique<Parameter>(std::make_unique<ParLerp>(0, 3), 44100));
    //auto nodeParLerp4 = std::make_unique<ParameterNode>(std::make_unique<Parameter>(std::make_unique<ParLerp>(0, 3), 44100));
    //auto nodeChangeVol = std::make_unique<TransformNode>(std::make_unique<ChangeVolume>(), Transform::MONO_IN_PLACE, 44100, 3);



    for (int i = 0; i < 2000; ++i) {
      lerp_test_signal(&graph, nodeBuf1, i*0.01, 0.01, 0.000002*i);
    }

    for (int i = 0; i < 4000; ++i) {
      lerp_test_signal(&graph, nodeBuf1, i*0.005, 0.005, 0.000001*i);
    }
    for (int i = 0; i < 8000; ++i) {
      lerp_test_signal(&graph, nodeBuf1, i*0.0025, 0.0025, 0.0000005*i);
    }


    /*
    for (int i = 0; i < 4; ++i) {
      lerp_test_signal(&graph, nodeBuf1, 0, 1+i, -0.5*i);
    }*/

    nodeToWAV->addBuffer(0, nodeBuf1);
    nodeToWAV->addPrerequisite(nodeBuf1);

    graph.addNode(nodeBuf1);
    graph.addNode(nodeToWAV);

    //graph.printGraph(std::cout);

    //graph.execute();

    const int NUM_THREADS = 12;

    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; ++i) {
      threads.emplace_back(test_thread_function, &graph);
    }

    for (auto& th : threads) {
      th.join();
    }

  };

}