//
// Created by rutio on 27.07.19.
//

#ifndef LUAMUSGEN_TEST_PARS_H
#define LUAMUSGEN_TEST_PARS_H


#include <processgraph/ProcessGraph.h>
#include <transforms/kinds/buffer_operations/BufferToWAV.h>
#include <transforms/kinds/generators/ConstantSignal.h>
#include <parameters/kinds/ParAdd.h>
#include <parameters/kinds/ParMul.h>
#include <parameters/kinds/ParExp.h>
#include <parameters/kinds/ParConcat.h>
#include <parameters/kinds/ParToArray.h>
#include <parameters/kinds/ParRepeat.h>
#include <parameters/kinds/ParTanh.h>
#include <parameters/kinds/ParLimit.h>
#include <transforms/kinds/filters/other/SimpleReverb.h>
#include <transforms/kinds/buffer_operations/Normalize.h>
#include <transforms/kinds/stereo/SimpleStereo.h>
#include <transforms/kinds/stereo/LegacyStereo.h>
#include <transforms/kinds/stereo/ImprovedStereo.h>
#include <transforms/kinds/filters/frequency_filters/PhaserLegacy.h>
#include <parameters/kinds/ParOscillator.h>
#include <parameters/kinds/ParVibrato.h>
#include <transforms/kinds/filters/frequency_filters/FlangerFeedforward.h>
#include <transforms/kinds/filters/frequency_filters/FlangerFeedback.h>
#include <transforms/kinds/filters/frequency_filters/FlangerSelfOscillation.h>
#include <transforms/kinds/generators/WaveOscillator.h>
#include <transforms/kinds/generators/WaveOscillatorShaped.h>
#include <transforms/kinds/filters/butterworth/ButterworthLowPass.h>
#include <transforms/kinds/filters/butterworth/ButterworthHighPass.h>
#include <transforms/kinds/filters/moog/MoogLowPass.h>
#include <transforms/kinds/filters/moog/MoogHighPass.h>

namespace test_pars {

  ProcessNode* parConst(ProcessGraph* graph, double v) {
    return graph->addNode(new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParConst>(v), 44100)));
  }

  ProcessNode* parLinear(ProcessGraph* graph, double v1, double v2, double length) {
    auto nodeC0 = graph->addNode(new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParConst>(v1), 44100)));
    auto nodeC1 = graph->addNode(new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParConst>(v2), 44100)));
    auto nodeParLinear = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLerp>(0, length), 44100));
    nodeParLinear->addArgument(0, nodeC0);
    nodeParLinear->addArgument(0, nodeC1);
    return graph->addNode(nodeParLinear);
  }

  ProcessNode* signal(ProcessGraph* graph, BufferNode* buffer, ProcessNode* arg, double start_point, double length,
                      double arg_offset = 0) {
    auto nodeSignal = new TransformNode(std::make_unique<ConstantSignal>(), Transform::MONO_IN_PLACE, 44100, length);
    nodeSignal->addArgument(arg_offset, arg);
    nodeSignal->addBuffer(start_point, buffer);
    buffer->addPrerequisite(nodeSignal);
    return graph->addNode(nodeSignal);
  }

  ProcessNode*
  parAdd(ProcessGraph* graph, ProcessNode* arg1, ProcessNode* arg2, double offset1 = 0.0, double offset2 = 0.0) {
    auto nodeParAdd = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParAdd>(), 44100));
    nodeParAdd->addArgument(offset1, arg1);
    nodeParAdd->addArgument(offset2, arg2);
    return graph->addNode(nodeParAdd);
  }

  ProcessNode*
  parMul(ProcessGraph* graph, ProcessNode* arg1, ProcessNode* arg2, double offset1 = 0.0, double offset2 = 0.0) {
    auto nodeParAdd = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParMul>(), 44100));
    nodeParAdd->addArgument(offset1, arg1);
    nodeParAdd->addArgument(offset2, arg2);
    return graph->addNode(nodeParAdd);
  }

  ProcessNode*
  parExp(ProcessGraph* graph, ProcessNode* arg1, ProcessNode* arg2, double offset1 = 0.0, double offset2 = 0.0) {
    auto nodeParAdd = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParExp>(), 44100));
    nodeParAdd->addArgument(offset1, arg1);
    nodeParAdd->addArgument(offset2, arg2);
    return graph->addNode(nodeParAdd);
  }

  ProcessNode* parConstArray(ProcessGraph* graph, double value) {
    auto nodeC0 = graph->addNode(
        new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParConst>(value), 44100)));
    auto parConstArray = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParToArray>(), 44100));
    parConstArray->addArgument(0, nodeC0);
    return graph->addNode(parConstArray);
  }

  ProcessNode* parToArray(ProcessGraph* graph, ProcessNode* arg, double offset = 0) {
    auto parConstArray = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParToArray>(), 44100));
    parConstArray->addArgument(offset, arg);
    return graph->addNode(parConstArray);
  }

  template<class Oscillator>
  ProcessNode* parOscil(ProcessGraph* graph, double start_phase, ProcessNode* freq, ProcessNode* base, ProcessNode* amp,
                        double freq_offset = 0, double base_offset = 0, double amp_offset = 0) {
    auto nodeParAdd = new ParameterNode(
        std::make_unique<Parameter>(std::make_unique<ParOscillator<Oscillator>>(start_phase), 44100));
    nodeParAdd->addArgument(freq_offset, freq);
    nodeParAdd->addArgument(base_offset, base);
    nodeParAdd->addArgument(amp_offset, amp);
    return graph->addNode(nodeParAdd);
  }

  template<class Oscillator>
  ProcessNode*
  parVibrato(ProcessGraph* graph, double start_phase, ProcessNode* freq, ProcessNode* base, ProcessNode* amp,
             double freq_offset = 0, double base_offset = 0, double amp_offset = 0) {
    auto nodeParAdd = new ParameterNode(
        std::make_unique<Parameter>(std::make_unique<ParVibrato<Oscillator>>(start_phase), 44100));
    nodeParAdd->addArgument(freq_offset, freq);
    nodeParAdd->addArgument(base_offset, base);
    nodeParAdd->addArgument(amp_offset, amp);
    return graph->addNode(nodeParAdd);
  }

  ProcessNode* testConcat(ProcessGraph* graph) {
    auto concat1 = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParConcat>(), 44100));
    auto const1 = parConstArray(graph, 0.5);
    auto line1 = parLinear(graph, 0.5, -1, 1);
    auto line2 = parLinear(graph, 0, 1, 1);
    auto add1 = parAdd(graph, line1, line2);
    auto mul1 = parMul(graph, line1, line2);
    auto exp1 = parExp(graph, line1, line2);
    auto array1 = parToArray(graph, exp1, -0.5);
    concat1->addArgument(0, const1, true, 1);
    concat1->addArgument(0, line1, true, 1);
    concat1->addArgument(0, line2, true, 1);
    concat1->addArgument(0, add1, true, 1);
    concat1->addArgument(0, mul1, true, 1);
    concat1->addArgument(0, array1, true, 1);
    return graph->addNode(concat1);
  }

  ProcessNode* parRepeat(ProcessGraph* graph, double length, ProcessNode* arg, double offset = 0) {
    auto parRepeat = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParRepeat>(), 44100));
    parRepeat->addArgument(offset, arg, true, length);
    return graph->addNode(parRepeat);
  }

  ProcessNode* parTanh(ProcessGraph* graph, ProcessNode* arg, double offset = 0) {
    auto parTanh = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParTanh>(), 44100));
    parTanh->addArgument(offset, arg);
    return graph->addNode(parTanh);
  }

  ProcessNode* parLimit(ProcessGraph* graph, ProcessNode* arg, double offset = 0) {
    auto parLimit = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLimit>(), 44100));
    parLimit->addArgument(offset, arg);
    return graph->addNode(parLimit);
  }

  ProcessNode* parLinExp(ProcessGraph* graph, double v1, double v2, ProcessNode* arg, double offset = 0) {
    auto parLinExp = new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLinearExp>(v1, v2), 44100));
    parLinExp->addArgument(offset, arg);
    return graph->addNode(parLinExp);
  }

  template<class Oscillator>
  ProcessNode* wave(ProcessGraph* graph, BufferNode* buffer, double start_point, double length, double phase,
                    ProcessNode* freq, double freq_offset = 0) {
    auto nodeWave = new TransformNode(std::make_unique<WaveOscillator<Oscillator>>(phase), Transform::STEREO_IN_PLACE,
                                      44100, length);
    nodeWave->addArgument(freq_offset, freq);
    nodeWave->addBuffer(start_point, buffer);
    buffer->addPrerequisite(nodeWave);
    return graph->addNode(nodeWave);
  }

  template<class Oscillator>
  ProcessNode*
  waveStereo(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point, double length,
             double phase, ProcessNode* freq, double freq_offset = 0) {
    auto nodeWave = new TransformNode(std::make_unique<WaveOscillator<Oscillator>>(phase), Transform::STEREO_IN_PLACE,
                                      44100, length);
    nodeWave->addArgument(freq_offset, freq);
    nodeWave->addBuffer(start_point, bufferL);
    nodeWave->addBuffer(start_point, bufferR);
    bufferL->addPrerequisite(nodeWave);
    bufferR->addPrerequisite(nodeWave);
    return graph->addNode(nodeWave);
  }

  ProcessNode* wavePulse(ProcessGraph* graph, BufferNode* buffer, double start_point, double length, double phase,
                         ProcessNode* freq, ProcessNode* shape, double freq_offset = 0, double shape_offset = 0) {
    auto nodeWave = new TransformNode(std::make_unique<WaveOscillatorShaped<oscil::Pulse>>(phase),
                                      Transform::MONO_IN_PLACE, 44100, length);
    nodeWave->addArgument(freq_offset, freq);
    nodeWave->addArgument(shape_offset, shape);
    nodeWave->addBuffer(start_point, buffer);
    buffer->addPrerequisite(nodeWave);
    return graph->addNode(nodeWave);
  }

  ProcessNode* wavePulseStereo(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point,
                               double length, double phase, ProcessNode* freq, ProcessNode* shape,
                               double freq_offset = 0, double shape_offset = 0) {
    auto nodeWave = new TransformNode(std::make_unique<WaveOscillatorShaped<oscil::Pulse>>(phase),
                                      Transform::STEREO_IN_PLACE, 44100, length);
    nodeWave->addArgument(freq_offset, freq);
    nodeWave->addArgument(shape_offset, shape);
    nodeWave->addBuffer(start_point, bufferL);
    nodeWave->addBuffer(start_point, bufferR);
    bufferL->addPrerequisite(nodeWave);
    bufferR->addPrerequisite(nodeWave);
    return graph->addNode(nodeWave);
  }

  ProcessNode* noiseStereo(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point,
                           double length) {
    auto nodeWave = new TransformNode(std::make_unique<WhiteNoise>(0), Transform::STEREO_IN_PLACE, 44100, length);
    nodeWave->addBuffer(start_point, bufferL);
    nodeWave->addBuffer(start_point, bufferR);
    bufferL->addPrerequisite(nodeWave);
    bufferR->addPrerequisite(nodeWave);
    return graph->addNode(nodeWave);
  }

  ProcessNode* simpleReverb(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point,
                            double length, double room_size, double damp, double dry_wet, double space_width,
                            double rel_length, const std::vector<ProcessNode*>& prerequisites = {}) {
    // The arguments for simpleReverb changed, no point going back to this test anymore

//    auto nodeReverb = new TransformNode(
//        std::make_unique<SimpleReverb>(room_size, damp, dry_wet, space_width, rel_length),
//        Transform::STEREO_IN_PLACE, 44100, length);
//    nodeReverb->addBuffer(start_point, bufferL);
//    nodeReverb->addBuffer(start_point, bufferR);
//    for (auto p : prerequisites) {
//      nodeReverb->addPrerequisite(p);
//    }
//    return graph->addNode(nodeReverb);
    return nullptr;
  }

  ProcessNode* normalize(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point,
                         double length, bool allow_volume_up, bool provide_feedback,
                         const std::vector<ProcessNode*>& prerequisites = {}) {
    auto node = new TransformNode(std::make_unique<Normalize>(allow_volume_up, provide_feedback),
                                  Transform::STEREO_IN_PLACE, 44100, length);
    node->addBuffer(start_point, bufferL);
    node->addBuffer(start_point, bufferR);
    for (auto p : prerequisites) {
      node->addPrerequisite(p);
    }
    return graph->addNode(node);
  }

  ProcessNode*
  phaserLegacy(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point, double length,
               int64_t stages, ProcessNode* coefficient, ProcessNode* feedback, ProcessNode* wetDry,
               double coefficient_offset = 0, double feedback_offset = 0, double wetDry_offset = 0,
               const std::vector<ProcessNode*>& prerequisites = {}) {
    auto node = new TransformNode(std::make_unique<PhaserLegacy>(stages),
                                  Transform::STEREO_IN_PLACE, 44100, length);
    node->addBuffer(start_point, bufferL);
    node->addBuffer(start_point, bufferR);
    node->addArgument(coefficient_offset, coefficient);
    node->addArgument(feedback_offset, feedback);
    node->addArgument(wetDry_offset, wetDry);
    for (auto p : prerequisites) {
      node->addPrerequisite(p);
    }
    return graph->addNode(node);
  }

  ProcessNode* flangerFeedforward(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point,
                                  double length, ProcessNode* frequency, ProcessNode* wetDry,
                                  double frequency_offset = 0, double wetDry_offset = 0,
                                  const std::vector<ProcessNode*>& prerequisites = {}) {
    auto node = new TransformNode(std::make_unique<FlangerFeedforward>(44100),
                                  Transform::STEREO_IN_PLACE, 44100, length);
    node->addBuffer(start_point, bufferL);
    node->addBuffer(start_point, bufferR);
    node->addArgument(frequency_offset, frequency);
    node->addArgument(wetDry_offset, wetDry);
    for (auto p : prerequisites) {
      node->addPrerequisite(p);
    }
    return graph->addNode(node);
  }

  ProcessNode* flangerFeedback(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point,
                               double length, ProcessNode* frequency, ProcessNode* wetDry, double frequency_offset = 0,
                               double wetDry_offset = 0, const std::vector<ProcessNode*>& prerequisites = {}) {
    auto node = new TransformNode(std::make_unique<FlangerFeedback>(44100),
                                  Transform::STEREO_IN_PLACE, 44100, length);
    node->addBuffer(start_point, bufferL);
    node->addBuffer(start_point, bufferR);
    node->addArgument(frequency_offset, frequency);
    node->addArgument(wetDry_offset, wetDry);
    for (auto p : prerequisites) {
      node->addPrerequisite(p);
    }
    return graph->addNode(node);
  }

  ProcessNode* flangerSelfOscil(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point,
                                double length, ProcessNode* frequency, ProcessNode* oscil_length,
                                ProcessNode* correction, double frequency_offset = 0, double oscil_length_offset = 0,
                                double correction_offset = 0, const std::vector<ProcessNode*>& prerequisites = {}) {
    auto node = new TransformNode(std::make_unique<FlangerSelfOscillation>(44100),
                                  Transform::STEREO_IN_PLACE, 44100, length);
    node->addBuffer(start_point, bufferL);
    node->addBuffer(start_point, bufferR);
    node->addArgument(frequency_offset, frequency);
    node->addArgument(oscil_length_offset, oscil_length);
    node->addArgument(correction_offset, correction);
    for (auto p : prerequisites) {
      node->addPrerequisite(p);
    }
    return graph->addNode(node);
  }

  ProcessNode* butterworthLowPass(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point,
                                  double length, int64_t iterations, ProcessNode* frequency, ProcessNode* resonance,
                                  ProcessNode* wetDry, double frequency_offset = 0, double resonance_offset = 0,
                                  double wetDry_offset = 0, const std::vector<ProcessNode*>& prerequisites = {}) {
    auto node = new TransformNode(std::make_unique<ButterworthLowPass>(iterations),
                                  Transform::STEREO_IN_PLACE, 44100, length);
    node->addBuffer(start_point, bufferL);
    node->addBuffer(start_point, bufferR);
    node->addArgument(frequency_offset, frequency);
    node->addArgument(resonance_offset, resonance);
    node->addArgument(wetDry_offset, wetDry);
    for (auto p : prerequisites) {
      node->addPrerequisite(p);
    }
    return graph->addNode(node);
  }

  ProcessNode* butterworthHighPass(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point,
                                   double length, int64_t iterations, ProcessNode* frequency, ProcessNode* resonance,
                                   ProcessNode* wetDry, double frequency_offset = 0, double resonance_offset = 0,
                                   double wetDry_offset = 0, const std::vector<ProcessNode*>& prerequisites = {}) {
    auto node = new TransformNode(std::make_unique<ButterworthHighPass>(iterations),
                                  Transform::STEREO_IN_PLACE, 44100, length);
    node->addBuffer(start_point, bufferL);
    node->addBuffer(start_point, bufferR);
    node->addArgument(frequency_offset, frequency);
    node->addArgument(resonance_offset, resonance);
    node->addArgument(wetDry_offset, wetDry);
    for (auto p : prerequisites) {
      node->addPrerequisite(p);
    }
    return graph->addNode(node);
  }

  template<class Saturation>
  ProcessNode* moogLowPass(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point,
                           double length, ProcessNode* frequency, ProcessNode* resonance, ProcessNode* wetDry,
                           double frequency_offset = 0, double resonance_offset = 0, double wetDry_offset = 0,
                           const std::vector<ProcessNode*>& prerequisites = {}) {
    auto node = new TransformNode(std::make_unique<MoogLowPass<Saturation>>(),
                                  Transform::STEREO_IN_PLACE, 44100, length);
    node->addBuffer(start_point, bufferL);
    node->addBuffer(start_point, bufferR);
    node->addArgument(frequency_offset, frequency);
    node->addArgument(resonance_offset, resonance);
    node->addArgument(wetDry_offset, wetDry);
    for (auto p : prerequisites) {
      node->addPrerequisite(p);
    }
    return graph->addNode(node);
  }

  template<class Saturation>
  ProcessNode* moogHighPass(ProcessGraph* graph, BufferNode* bufferL, BufferNode* bufferR, double start_point,
                            double length, ProcessNode* frequency, ProcessNode* resonance, ProcessNode* wetDry,
                            double frequency_offset = 0, double resonance_offset = 0, double wetDry_offset = 0,
                            const std::vector<ProcessNode*>& prerequisites = {}) {
    auto node = new TransformNode(std::make_unique<MoogHighPass<Saturation>>(),
                                  Transform::STEREO_IN_PLACE, 44100, length);
    node->addBuffer(start_point, bufferL);
    node->addBuffer(start_point, bufferR);
    node->addArgument(frequency_offset, frequency);
    node->addArgument(resonance_offset, resonance);
    node->addArgument(wetDry_offset, wetDry);
    for (auto p : prerequisites) {
      node->addPrerequisite(p);
    }
    return graph->addNode(node);
  }

  void parTest(ProcessGraph* graph) {
    auto nodeToWAV = new TransformNode(std::make_unique<BufferToWAV>("test_pars.wav"), Transform::MONO_IN_PLACE, 44100,
                                       9);
    auto nodeToWAV2 = new TransformNode(std::make_unique<BufferToWAV>("test_pars2.wav"), Transform::STEREO_IN_PLACE,
                                        44100, 9);
    auto buffer = new BufferNode(44100);
    auto bufferL = new BufferNode(44100);
    auto bufferR = new BufferNode(44100);
    auto line1 = parLinear(graph, 0.5, -1, 1);
    auto line2 = parLinear(graph, 0, 1, 1);
    auto line3 = parLinear(graph, 1, 0, 6);
    auto line4 = parLinear(graph, 0, 1, 6);
    auto add1 = parAdd(graph, line1, line2);
    auto mul1 = parMul(graph, line1, line2);
    auto exp1 = parExp(graph, line1, line2);
    auto sin1 = parOscil<oscil::Sin>(graph, M_PI, parConst(graph, 1), line3, line4);
    signal(graph, buffer, add1, 0, 1, 0);
    signal(graph, buffer, mul1, 1, 1, 0);
    signal(graph, buffer, exp1, 2, 1, 0);
    signal(graph, buffer, sin1, 3, 6, 0);
    //auto concat1 = testConcat(&graph);
    auto repeat1 = parRepeat(graph, 2, exp1, -0.5);
    //auto tanh1 = parTanh(&graph, repeat1, 0);
    auto limit1 = parLimit(graph, repeat1, 0);
    auto mul2 = parMul(graph, limit1, parConst(graph, 0.5));
    signal(graph, buffer, mul2, 9, 6, 0);
    nodeToWAV->addBuffer(0, buffer);
    nodeToWAV->addPrerequisite(buffer);

    auto nodeStereo = new TransformNode(std::make_unique<SimpleStereo>(), Transform::MONO_TO_STEREO, 44100, 9);
    nodeStereo->addArgument(0, parLinear(graph, -1, 1, 9));
    nodeStereo->addBuffer(0, buffer);
    nodeStereo->addBuffer(0, bufferL);
    nodeStereo->addBuffer(0, bufferR);
    nodeStereo->addPrerequisite(buffer);
    bufferL->addPrerequisite(nodeStereo);
    bufferR->addPrerequisite(nodeStereo);
    nodeToWAV2->addBuffer(0, bufferL);
    nodeToWAV2->addBuffer(0, bufferR);
    nodeToWAV2->addPrerequisite(bufferL);
    nodeToWAV2->addPrerequisite(bufferR);

    graph->addNode(nodeStereo);
    graph->addNode(nodeToWAV);
    graph->addNode(nodeToWAV2);
    graph->addNode(buffer);
    graph->addNode(bufferL);
    graph->addNode(bufferR);
  }

  void pulseTest(ProcessGraph* graph) {
    auto nodeToWAV = new TransformNode(std::make_unique<BufferToWAV>("test_pars.wav"), Transform::STEREO_IN_PLACE,
                                       44100, 10);
    auto bufferL = new BufferNode(44100);
    auto bufferR = new BufferNode(44100);
    auto parLine1 = parLinear(graph, 110, 110 * 4, 9);
    auto parLine1Exp = parLinExp(graph, 110, 110 * 4, parLine1, 0);
    auto parVibrato1 = parVibrato<oscil::Sin>(graph, 0, parConst(graph, 5), parLine1Exp, parConst(graph, 1.015));
    //auto sin1 = parOscil<oscil::Sin>(graph, 0, parConst(graph, 0.5), parConst(graph, 1000), parConst(graph, 100));
    //auto parLine2 = parLinear(graph, 0.1, 1.9, 9);
    //waveSqrtri(&graph, nodeBuf1, 0, 9, 0, parLine1Exp, 0);
    //auto wave1 = wavePulseStereo(graph, bufferL, bufferR, 0, 9, 0, parLine1Exp, 0, parLine2, 0);
    auto wave1 = waveStereo<oscil::Swt>(graph, bufferL, bufferR, 0, 9, 0,
                                        parVibrato1, 0);
    //auto noise1 = noiseStereo(graph, bufferL, bufferR, 0, 0.05);

    /*auto phaser1 = phaserLegacy(graph, bufferL, bufferR, 0, 10, 8,
                                parSin(graph, 0, parConst(graph, 0.5), 0, parConst(graph, 0), 0, parConst(graph, 1), 0), 0,
                                parLinear(graph, 0.9, 0, 10), 0,
                                parLinear(graph, 0, 1, 10), 0,
                                {nodeStereo});*/
    /*auto flanger1 = flangerFeedforward(
        graph, bufferL, bufferR, 0, 10,
        parVibrato1,
        parConst(graph, 1),
        0, 0, {wave1});*/

    auto filter1 = moogLowPass<moog::SaturationNone>(graph, bufferL, bufferR, 0, 10,
                                                     parMul(graph, parLine1Exp, parConst(graph, 2)),
                                                     parConst(graph, 0.3),
                                                     parConst(graph, 1),
                                                     0.5, 0, 0, {wave1});

    auto nodeStereo = new TransformNode(std::make_unique<ImprovedStereo>(), Transform::STEREO_IN_PLACE, 44100, 10);
    nodeStereo->addArgument(0, parLinear(graph, 0.5, -0.5, 10));
    nodeStereo->addBuffer(0, bufferL);
    nodeStereo->addBuffer(0, bufferR);
    bufferL->addPrerequisite(nodeStereo);
    bufferR->addPrerequisite(nodeStereo);
    nodeStereo->addPrerequisite(filter1);

    //auto reverb1 = simpleReverb(graph, bufferL, bufferR, 0, 10, 1, 0.5, 0.5, 1, 1, {nodeStereo});

    //auto normalize1 = normalize(graph, bufferL, bufferR, 0, 10, true, true, {reverb1});
    auto normalize1 = normalize(graph, bufferL, bufferR, 0, 10, true, true, {});

    nodeToWAV->addBuffer(0, bufferL);
    nodeToWAV->addBuffer(0, bufferR);
    nodeToWAV->addPrerequisite(bufferL);
    nodeToWAV->addPrerequisite(bufferR);
    nodeToWAV->addPrerequisite(normalize1);
    graph->addNode(nodeStereo);
    graph->addNode(nodeToWAV);
    graph->addNode(bufferL);
    graph->addNode(bufferR);
  }

  void test() {
    setLoggerLevel(3);

    ProcessGraph graph;

    pulseTest(&graph);

    //graph.printGraph(std::cout);
    graph.execute();

  };

}

#endif //LUAMUSGEN_TEST_PARS_H
