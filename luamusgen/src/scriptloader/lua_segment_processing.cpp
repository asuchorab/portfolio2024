//
// Created by rutio on 16.01.19.
//

#include "lua_segment_processing.h"
#include <parameters/kinds/ParAdd.h>
#include <parameters/kinds/ParConcat.h>
#include <parameters/kinds/ParConst.h>
#include <parameters/kinds/ParConstAt.h>
#include <parameters/kinds/ParDiv.h>
#include <parameters/kinds/ParExp.h>
#include <parameters/kinds/ParLerp.h>
#include <parameters/kinds/ParLimit.h>
#include <parameters/kinds/ParLinearExp.h>
#include <parameters/kinds/ParMul.h>
#include <parameters/kinds/ParOscillator.h>
#include <parameters/kinds/ParOscillatorShaped.h>
#include <parameters/kinds/ParRepeat.h>
#include <parameters/kinds/ParSub.h>
#include <parameters/kinds/ParTanh.h>
#include <parameters/kinds/ParVibrato.h>
#include <parameters/kinds/ParVibratoShaped.h>
#include <transforms/kinds/buffer_operations/BufferFromRaw.h>
#include <transforms/kinds/buffer_operations/BufferFromWAV.h>
#include <transforms/kinds/buffer_operations/BufferToRaw.h>
#include <transforms/kinds/buffer_operations/BufferToWAV.h>
#include <transforms/kinds/buffer_operations/CopyBuffer.h>
#include <transforms/kinds/buffer_operations/Normalize.h>
#include <transforms/kinds/buffer_operations/SoftClipCompressor.h>
#include <transforms/kinds/filters/butterworth/ButterworthBandPass.h>
#include <transforms/kinds/filters/butterworth/ButterworthBandStop.h>
#include <transforms/kinds/filters/butterworth/ButterworthHighPass.h>
#include <transforms/kinds/filters/butterworth/ButterworthLowPass.h>
#include <transforms/kinds/filters/frequency_filters/FlangerFeedback.h>
#include <transforms/kinds/filters/frequency_filters/FlangerFeedforward.h>
#include <transforms/kinds/filters/frequency_filters/FlangerSelfOscillation.h>
#include <transforms/kinds/filters/frequency_filters/PhaserLegacy.h>
#include <transforms/kinds/filters/moog/MoogHighPass.h>
#include <transforms/kinds/filters/moog/MoogLowPass.h>
#include <transforms/kinds/filters/other/ChangeVolume.h>
#include <transforms/kinds/filters/other/ExpShape.h>
#include <transforms/kinds/filters/other/Limiter.h>
#include <transforms/kinds/filters/other/RingMod.h>
#include <transforms/kinds/filters/other/SaturationTanh.h>
#include <transforms/kinds/filters/other/SimpleReverb.h>
#include <transforms/kinds/filters/other/SimpleReverseReverb.h>
#include <transforms/kinds/generators/ConstantSignal.h>
#include <transforms/kinds/generators/PolysynthSwtPulse.h>
#include <transforms/kinds/generators/SuperwaveOscillator.h>
#include <transforms/kinds/generators/SuperwaveOscillatorShaped.h>
#include <transforms/kinds/generators/WaveOscillator.h>
#include <transforms/kinds/generators/WaveOscillatorShaped.h>
#include <transforms/kinds/generators/WhiteNoise.h>
#include <transforms/kinds/stereo/ImprovedStereo.h>
#include <transforms/kinds/stereo/LegacyStereo.h>
#include <transforms/kinds/stereo/SimpleStereo.h>

thread_local ProcessGraph* lua_segment_processing::thread_graph = nullptr;

void lua_segment_processing::load(lua_State* L) {
  // Because you can't just insert something into a LuaBridge table later
  // And enums don't exist
  // And constants don't exist
  static int SIN = 0;
  static int SQR = 1;
  static int SWT = 2;
  static int TRI = 3;
  static int SQRTRI = 4;
  static int PULSE = 0;
  static int NONE = 0;
  static int LIMIT = 1;
  static int TANH = 2;
  static int MONO_IN_PLACE = 0;
  static int STEREO_IN_PLACE = 1;
  static int MONO_TO_MONO = 2;
  static int STEREO_TO_STEREO = 3;
  static int MONO_TO_STEREO = 4;
  static int STEREO_TO_MONO = 5;

  luabridge::getGlobalNamespace(L)
      .beginClass<ProcessNode>("ProcessNode")
      .endClass()
      .deriveClass<ParameterNode, ProcessNode>("ParameterNode")
      .endClass()
      .deriveClass<TransformNode, ProcessNode>("TransformNode")
      .endClass()
      .deriveClass<BufferNode, ProcessNode>("BufferNode")
      .endClass()
      .deriveClass<DummyNode, ProcessNode>("DummyNode")
      .endClass()
      .beginNamespace("lua_segment_processing")
      .addProperty("SIN", &SIN, false)
      .addProperty("SQR", &SQR, false)
      .addProperty("SWT", &SWT, false)
      .addProperty("TRI", &TRI, false)
      .addProperty("SQRTRI", &SQRTRI, false)
      .addProperty("PULSE", &PULSE, false)
      .addProperty("NONE", &NONE, false)
      .addProperty("LIMIT", &LIMIT, false)
      .addProperty("TANH", &TANH, false)
      .addProperty("MONO_IN_PLACE", &MONO_IN_PLACE, false)
      .addProperty("STEREO_IN_PLACE", &STEREO_IN_PLACE, false)
      .addProperty("MONO_TO_MONO", &MONO_TO_MONO, false)
      .addProperty("STEREO_TO_STEREO", &STEREO_TO_STEREO, false)
      .addProperty("MONO_TO_STEREO", &MONO_TO_STEREO, false)
      .addProperty("STEREO_TO_MONO", &STEREO_TO_MONO, false)
      .addFunction("addNode", addNode)
      .addFunction("getSampleRate", getSampleRate)
      .addFunction("addArgument", addArgument)
      .addFunction("addBuffer", addBuffer)
      .addFunction("addPrerequisite", addPrerequisite)
      .addFunction("bufferNode", bufferNode)
      .addFunction("dummyNode", dummyNode)
      .addFunction("parAdd", parAdd)
      .addFunction("parConcat", parConcat)
      .addFunction("parConst", parConst)
      .addFunction("parConstAt", parConstAt)
      .addFunction("parDiv", parDiv)
      .addFunction("parExp", parExp)
      .addFunction("parLerp", parLerp)
      .addFunction("parLimit", parLimit)
      .addFunction("parLinearExp", parLinearExp)
      .addFunction("parMul", parMul)
      .addFunction("parOscillator", parOscillator)
      .addFunction("parOscillatorShaped", parOscillatorShaped)
      .addFunction("parRepeat", parRepeat)
      .addFunction("parSub", parSub)
      .addFunction("parTanh", parTanh)
      .addFunction("parVibrato", parVibrato)
      .addFunction("parVibratoShaped", parVibratoShaped)
      .addFunction("bufferFromRaw", bufferFromRaw)
      .addFunction("bufferFromWAV", bufferFromWAV)
      .addFunction("bufferToRaw", bufferToRaw)
      .addFunction("bufferToWAV", bufferToWAV)
      .addFunction("copyBuffer", copyBuffer)
      .addFunction("normalize", normalize)
      .addFunction("softClipCompressor", softClipCompressor)
      .addFunction("butterworthBandPass", butterworthBandPass)
      .addFunction("butterworthBandStop", butterworthBandStop)
      .addFunction("butterworthHighPass", butterworthHighPass)
      .addFunction("butterworthLowPass", butterworthLowPass)
      .addFunction("flangerFeedback", flangerFeedback)
      .addFunction("flangerFeedforward", flangerFeedforward)
      .addFunction("flangerSelfOscillation", flangerSelfOscillation)
      .addFunction("phaserLegacy", phaserLegacy)
      .addFunction("moogHighPass", moogHighPass)
      .addFunction("moogLowPass", moogLowPass)
      .addFunction("changeVolume", changeVolume)
      .addFunction("expShape", expShape)
      .addFunction("limiter", limiter)
      .addFunction("ringMod", ringMod)
      .addFunction("saturationTanh", saturationTanh)
      .addFunction("simpleReverb", simpleReverb)
      .addFunction("simpleReverseReverb", simpleReverseReverb)
      .addFunction("constantSignal", constantSignal)
      .addFunction("polysynthSwtPulse", polysynthSwtPulse)
      .addFunction("superwaveOscillator", superwaveOscillator)
      .addFunction("superwaveOscillatorShaped", superwaveOscillatorShaped)
      .addFunction("waveOscillator", waveOscillator)
      .addFunction("waveOscillatorShaped", waveOscillatorShaped)
      .addFunction("whiteNoise", whiteNoise)
      .addFunction("improvedStereo", improvedStereo)
      .addFunction("legacyStereo", legacyStereo)
      .addFunction("simpleStereo", simpleStereo)
      .endNamespace();
}

ProcessNode* addNode(ProcessNode* node) {
  return lua_segment_processing::thread_graph->addNode(node);
}

double getSampleRate(ProcessNode* node) {
  if (!node) {
    logErrorF("node is nullptr (nil)");
    throw std::runtime_error("node is nullptr (nil)");
  }
  return node->getSampleRate();
}

void addArgument(ProcessNode* node, double start_point, ProcessNode* argument, bool fixed_start_point,
                 double other_length, bool touch_zero) {
  if (!node) {
    logErrorF("node is nullptr (nil)");
    throw std::runtime_error("node is nullptr (nil)");
  }
  node->addArgument(start_point, argument, fixed_start_point, other_length, touch_zero);
}

void addBuffer(ProcessNode* node, double start_point, ProcessNode* buffer, double other_length,
               bool allow_other_sample_rate) {
  if (!node) {
    logErrorF("node is nullptr (nil)");
    throw std::runtime_error("node is nullptr (nil)");
  }
  node->addBuffer(start_point, buffer, other_length, allow_other_sample_rate);
}

void addPrerequisite(ProcessNode* node, ProcessNode* prerequisite) {
  if (!node) {
    logErrorF("node is nullptr (nil)");
    throw std::runtime_error("node is nullptr (nil)");
  }
  node->addPrerequisite(prerequisite);
}

ProcessNode* bufferNode(double sample_rate) {
  return new BufferNode(sample_rate);
}

ProcessNode* dummyNode() {
  return new DummyNode();
}

ProcessNode* parAdd(double sample_rate) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParAdd>(), sample_rate));
}

ProcessNode* parConcat(double sample_rate) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParConcat>(), sample_rate));
}

ProcessNode* parConst(double sample_rate, double value) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParConst>(value), sample_rate));
}

ProcessNode* parConstAt(double sample_rate) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParConstAt>(), sample_rate));
}

ProcessNode* parDiv(double sample_rate) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParDiv>(), sample_rate));
}

ProcessNode* parExp(double sample_rate) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParExp>(), sample_rate));
}

ProcessNode* parLerp(double sample_rate, double t1, double t2) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLerp>(t1, t2), sample_rate));
}

ProcessNode* parLimit(double sample_rate) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLimit>(), sample_rate));
}

ProcessNode* parLinearExp(double sample_rate, double v1, double v2) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParLinearExp>(v1, v2), sample_rate));
}

ProcessNode* parMul(double sample_rate) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParMul>(), sample_rate));
}

ProcessNode* parOscillator(double sample_rate, OscillatorType type, double start_phase) {
  switch (type) {
    case SIN:
      return new ParameterNode(
          std::make_unique<Parameter>(std::make_unique<ParOscillator<oscil::Sin>>(start_phase), sample_rate));
    case SQR:
      return new ParameterNode(
          std::make_unique<Parameter>(std::make_unique<ParOscillator<oscil::Sqr>>(start_phase), sample_rate));
    case SWT:
      return new ParameterNode(
          std::make_unique<Parameter>(std::make_unique<ParOscillator<oscil::Swt>>(start_phase), sample_rate));
    case TRI:
      return new ParameterNode(
          std::make_unique<Parameter>(std::make_unique<ParOscillator<oscil::Tri>>(start_phase), sample_rate));
    case SQRTRI:
      return new ParameterNode(
          std::make_unique<Parameter>(std::make_unique<ParOscillator<oscil::Sqrtri>>(start_phase), sample_rate));
  }
  logErrorF("unknown OscillatorType %d", (int) type);
  return nullptr;
}

ProcessNode* parOscillatorShaped(double sample_rate, ShapedOscillatorType type, double start_phase) {
  switch (type) {
    case PULSE:
      return new ParameterNode(
          std::make_unique<Parameter>(std::make_unique<ParOscillatorShaped<oscil::Pulse>>(start_phase), sample_rate));
  }
  logErrorF("unknown ShapedOscillatorType %d", (int) type);
  return nullptr;
}

ProcessNode* parRepeat(double sample_rate) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParRepeat>(), sample_rate));
}

ProcessNode* parSub(double sample_rate) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParSub>(), sample_rate));
}

ProcessNode* parTanh(double sample_rate) {
  return new ParameterNode(std::make_unique<Parameter>(std::make_unique<ParTanh>(), sample_rate));
}

ProcessNode* parVibrato(double sample_rate, OscillatorType type, double start_phase) {
  switch (type) {
    case SIN:
      return new ParameterNode(
          std::make_unique<Parameter>(std::make_unique<ParVibrato<oscil::Sin>>(start_phase), sample_rate));
    case SQR:
      return new ParameterNode(
          std::make_unique<Parameter>(std::make_unique<ParVibrato<oscil::Sqr>>(start_phase), sample_rate));
    case SWT:
      return new ParameterNode(
          std::make_unique<Parameter>(std::make_unique<ParVibrato<oscil::Swt>>(start_phase), sample_rate));
    case TRI:
      return new ParameterNode(
          std::make_unique<Parameter>(std::make_unique<ParVibrato<oscil::Tri>>(start_phase), sample_rate));
    case SQRTRI:
      return new ParameterNode(
          std::make_unique<Parameter>(std::make_unique<ParVibrato<oscil::Sqrtri>>(start_phase), sample_rate));
  }
  logErrorF("unknown OscillatorType %d", (int) type);
  return nullptr;
}

ProcessNode* parVibratoShaped(double sample_rate, ShapedOscillatorType type, double start_phase) {
  switch (type) {
    case PULSE:
      return new ParameterNode(
          std::make_unique<Parameter>(std::make_unique<ParVibratoShaped<oscil::Pulse>>(start_phase), sample_rate));
  }
  logErrorF("unknown ShapedOscillatorType %d", (int) type);
  return nullptr;
}

ProcessNode* bufferFromRaw(double sample_rate, double length, TransformType mode, const char* filename) {
  return new TransformNode(std::make_unique<BufferFromRaw>(filename), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* bufferFromWAV(double sample_rate, double length, TransformType mode, const char* filename) {
  return new TransformNode(std::make_unique<BufferFromWAV>(filename), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* bufferToRaw(double sample_rate, double length, TransformType mode, const char* filename) {
  return new TransformNode(std::make_unique<BufferToRaw>(filename), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* bufferToWAV(double sample_rate, double length, TransformType mode, const char* filename) {
  return new TransformNode(std::make_unique<BufferToWAV>(filename), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* copyBuffer(double sample_rate, double length, TransformType mode) {
  return new TransformNode(std::make_unique<CopyBuffer>(), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* normalize(double sample_rate, double length, TransformType mode, bool allow_volume_up,
                       bool provide_feedback) {
  return new TransformNode(std::make_unique<Normalize>(allow_volume_up, provide_feedback), (Transform::Type) mode,
                           sample_rate, length);
}

ProcessNode* softClipCompressor(double sample_rate, double length, TransformType mode) {
  return new TransformNode(std::make_unique<SoftClipCompressor>(), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* butterworthBandPass(double sample_rate, double length, TransformType mode, int64_t iterations) {
  return new TransformNode(
      std::make_unique<ButterworthBandPass>(iterations), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* butterworthBandStop(double sample_rate, double length, TransformType mode, int64_t iterations) {
  return new TransformNode(
      std::make_unique<ButterworthBandStop>(iterations), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* butterworthHighPass(double sample_rate, double length, TransformType mode, int64_t iterations) {
  return new TransformNode(
      std::make_unique<ButterworthHighPass>(iterations), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* butterworthLowPass(double sample_rate, double length, TransformType mode, int64_t iterations) {
  return new TransformNode(
      std::make_unique<ButterworthLowPass>(iterations), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* flangerFeedback(double sample_rate, double length, TransformType mode, int64_t buffer_length) {
  return new TransformNode(
      std::make_unique<FlangerFeedback>(buffer_length), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* flangerFeedforward(double sample_rate, double length, TransformType mode, int64_t buffer_length) {
  return new TransformNode(
      std::make_unique<FlangerFeedforward>(buffer_length), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* flangerSelfOscillation(double sample_rate, double length, TransformType mode, int64_t buffer_length) {
  return new TransformNode(
      std::make_unique<FlangerSelfOscillation>(buffer_length), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* phaserLegacy(double sample_rate, double length, TransformType mode, int64_t stages) {
  return new TransformNode(
      std::make_unique<PhaserLegacy>(stages), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* moogHighPass(double sample_rate, double length, TransformType mode, SaturationType saturation_mode) {
  switch (saturation_mode) {
    case NONE:
      return new TransformNode(
          std::make_unique<MoogHighPass<moog::SaturationNone>>(), (Transform::Type) mode, sample_rate, length);
    case LIMIT:
      return new TransformNode(
          std::make_unique<MoogHighPass<moog::SaturationLimit>>(), (Transform::Type) mode, sample_rate, length);
    case TANH:
      return new TransformNode(
          std::make_unique<MoogHighPass<moog::SaturationTanh>>(), (Transform::Type) mode, sample_rate, length);
  }
  logErrorF("unknown SaturationType %d", (int) saturation_mode);
  return nullptr;
}

ProcessNode* moogLowPass(double sample_rate, double length, TransformType mode, SaturationType saturation_mode) {
  switch (saturation_mode) {
    case NONE:
      return new TransformNode(
          std::make_unique<MoogLowPass<moog::SaturationNone>>(), (Transform::Type) mode, sample_rate, length);
    case LIMIT:
      return new TransformNode(
          std::make_unique<MoogLowPass<moog::SaturationLimit>>(), (Transform::Type) mode, sample_rate, length);
    case TANH:
      return new TransformNode(
          std::make_unique<MoogLowPass<moog::SaturationTanh>>(), (Transform::Type) mode, sample_rate, length);
  }
  logErrorF("unknown SaturationType %d", (int) saturation_mode);
  return nullptr;
}

ProcessNode* changeVolume(double sample_rate, double length, TransformType mode) {
  return new TransformNode(std::make_unique<ChangeVolume>(), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* expShape(double sample_rate, double length, TransformType mode) {
  return new TransformNode(std::make_unique<ExpShape>(), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* limiter(double sample_rate, double length, TransformType mode) {
  return new TransformNode(std::make_unique<Limiter>(), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* ringMod(double sample_rate, double length, TransformType mode) {
  return new TransformNode(std::make_unique<RingMod>(), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* saturationTanh(double sample_rate, double length, TransformType mode) {
  return new TransformNode(std::make_unique<SaturationTanh>(), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* simpleReverb(double sample_rate, double length, TransformType mode, double rel_length) {
  return new TransformNode(std::make_unique<SimpleReverb>(rel_length), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* simpleReverseReverb(double sample_rate, double length, TransformType mode, double rel_length) {
  return new TransformNode(std::make_unique<SimpleReverseReverb>(rel_length), (Transform::Type) mode, sample_rate,
                           length);
}

ProcessNode* constantSignal(double sample_rate, double length, TransformType mode) {
  return new TransformNode(std::make_unique<ConstantSignal>(), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* polysynthSwtPulse(double sample_rate, double length, TransformType mode, double start_phase_swt,
                               double start_phase_pulse) {
  return new TransformNode(std::make_unique<PolysynthSwtPulse>(start_phase_swt, start_phase_pulse),
                           (Transform::Type) mode, sample_rate, length);
}

ProcessNode* superwaveOscillator(double sample_rate, double length, TransformType mode, OscillatorType type,
                                 double start_phase, uint32_t seed) {
  switch (type) {
    case SIN:
      return new TransformNode(
          std::make_unique<SuperwaveOscillator<oscil::Sin>>(start_phase, seed), (Transform::Type) mode, sample_rate,
          length);
    case SQR:
      return new TransformNode(
          std::make_unique<SuperwaveOscillator<oscil::Sqr>>(start_phase, seed), (Transform::Type) mode, sample_rate,
          length);
    case SWT:
      return new TransformNode(
          std::make_unique<SuperwaveOscillator<oscil::Swt>>(start_phase, seed), (Transform::Type) mode, sample_rate,
          length);
    case TRI:
      return new TransformNode(
          std::make_unique<SuperwaveOscillator<oscil::Tri>>(start_phase, seed), (Transform::Type) mode, sample_rate,
          length);
    case SQRTRI:
      return new TransformNode(
          std::make_unique<SuperwaveOscillator<oscil::Sqrtri>>(start_phase, seed), (Transform::Type) mode, sample_rate,
          length);
  }
  logErrorF("unknown OscillatorType %d", (int) type);
  return nullptr;
}

ProcessNode* superwaveOscillatorShaped(double sample_rate, double length, TransformType mode, ShapedOscillatorType type,
                                       double start_phase,
                                       uint32_t seed) {
  switch (type) {
    case PULSE:
      return new TransformNode(
          std::make_unique<SuperwaveOscillatorShaped<oscil::Pulse>>(start_phase, seed), (Transform::Type) mode,
          sample_rate,
          length);
  }
  logErrorF("unknown ShapedOscillatorType %d", (int) type);
  return nullptr;
}

ProcessNode* waveOscillator(double sample_rate, double length, TransformType mode, OscillatorType type,
                            double start_phase) {
  switch (type) {
    case SIN:
      return new TransformNode(
          std::make_unique<WaveOscillator<oscil::Sin>>(start_phase), (Transform::Type) mode, sample_rate, length);
    case SQR:
      return new TransformNode(
          std::make_unique<WaveOscillator<oscil::Sqr>>(start_phase), (Transform::Type) mode, sample_rate, length);
    case SWT:
      return new TransformNode(
          std::make_unique<WaveOscillator<oscil::Swt>>(start_phase), (Transform::Type) mode, sample_rate, length);
    case TRI:
      return new TransformNode(
          std::make_unique<WaveOscillator<oscil::Tri>>(start_phase), (Transform::Type) mode, sample_rate, length);
    case SQRTRI:
      return new TransformNode(
          std::make_unique<WaveOscillator<oscil::Sqrtri>>(start_phase), (Transform::Type) mode, sample_rate, length);
  }
  logErrorF("unknown OscillatorType %d", (int) type);
  return nullptr;
}

ProcessNode* waveOscillatorShaped(double sample_rate, double length, TransformType mode, ShapedOscillatorType type,
                                  double start_phase) {
  switch (type) {
    case PULSE:
      return new TransformNode(
          std::make_unique<WaveOscillatorShaped<oscil::Pulse>>(start_phase), (Transform::Type) mode, sample_rate,
          length);
  }
  logErrorF("unknown ShapedOscillatorType %d", (int) type);
  return nullptr;
}

ProcessNode* whiteNoise(double sample_rate, double length, TransformType mode, uint32_t seed) {
  return new TransformNode(std::make_unique<WhiteNoise>(seed), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* improvedStereo(double sample_rate, double length, TransformType mode) {
  return new TransformNode(std::make_unique<ImprovedStereo>(), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* legacyStereo(double sample_rate, double length, TransformType mode) {
  return new TransformNode(std::make_unique<LegacyStereo>(), (Transform::Type) mode, sample_rate, length);
}

ProcessNode* simpleStereo(double sample_rate, double length, TransformType mode) {
  return new TransformNode(std::make_unique<SimpleStereo>(), (Transform::Type) mode, sample_rate, length);
}
