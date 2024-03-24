local ffi = require "ffi"

ffi.cdef [[
typedef struct ProcessNode {} ProcessNode;

typedef enum OscillatorType {
  SIN, SQR, SWT, TRI, SQRTRI
} OscillatorType;

typedef enum ShapedOscillatorType {
  PULSE
} ShapedOscillatorType;

typedef enum SaturationType {
  NONE, LIMIT, TANH
} SaturationType;

typedef enum TransformType {
  MONO_IN_PLACE, STEREO_IN_PLACE, MONO_TO_MONO, STEREO_TO_STEREO, MONO_TO_STEREO, STEREO_TO_MONO
} TransformType;


ProcessNode* addNode(ProcessNode* node);
double getSampleRate(ProcessNode* node);

void addArgument(ProcessNode* node, double start_point, ProcessNode* argument, bool fixed_start_point,
                 double other_length, bool touch_zero);
void addBuffer(ProcessNode* node, double start_point, ProcessNode* buffer, double other_length,
               bool allow_other_sample_rate);
void addPrerequisite(ProcessNode* node, ProcessNode* prerequisite);

ProcessNode* bufferNode(double sample_rate);

ProcessNode* dummyNode();

ProcessNode* parAdd(double sample_rate);
ProcessNode* parConcat(double sample_rate);
ProcessNode* parConst(double sample_rate, double value);
ProcessNode* parConstAt(double sample_rate);
ProcessNode* parDiv(double sample_rate);
ProcessNode* parExp(double sample_rate);
ProcessNode* parLerp(double sample_rate, double t1, double t2);
ProcessNode* parLimit(double sample_rate);
ProcessNode* parLinearExp(double sample_rate, double v1, double v2);
ProcessNode* parMul(double sample_rate);
ProcessNode* parOscillator(double sample_rate, OscillatorType type, double start_phase);
ProcessNode* parOscillatorShaped(double sample_rate, ShapedOscillatorType type, double start_phase);
ProcessNode* parRepeat(double sample_rate);
ProcessNode* parSub(double sample_rate);
ProcessNode* parTanh(double sample_rate);
ProcessNode* parVibrato(double sample_rate, OscillatorType type, double start_phase);
ProcessNode* parVibratoShaped(double sample_rate, ShapedOscillatorType type, double start_phase);

ProcessNode* bufferFromRaw(double sample_rate, double length, TransformType mode, const char* filename);
ProcessNode* bufferFromWAV(double sample_rate, double length, TransformType mode, const char* filename);
ProcessNode* bufferToRaw(double sample_rate, double length, TransformType mode, const char* filename);
ProcessNode* bufferToWAV(double sample_rate, double length, TransformType mode, const char* filename);
ProcessNode* copyBuffer(double sample_rate, double length, TransformType mode);
ProcessNode* normalize(double sample_rate, double length, TransformType mode, bool allow_volume_up,
                       bool provide_feedback);
ProcessNode* softClipCompressor(double sample_rate, double length, TransformType mode);

ProcessNode* butterworthBandPass(double sample_rate, double length, TransformType mode, int64_t iterations);
ProcessNode* butterworthBandStop(double sample_rate, double length, TransformType mode, int64_t iterations);
ProcessNode* butterworthHighPass(double sample_rate, double length, TransformType mode, int64_t iterations);
ProcessNode* butterworthLowPass(double sample_rate, double length, TransformType mode, int64_t iterations);

ProcessNode* flangerFeedback(double sample_rate, double length, TransformType mode, int64_t buffer_length);
ProcessNode* flangerFeedforward(double sample_rate, double length, TransformType mode, int64_t buffer_length);
ProcessNode* flangerSelfOscillation(double sample_rate, double length, TransformType mode, int64_t buffer_length);
ProcessNode* phaserLegacy(double sample_rate, double length, TransformType mode, int64_t stages);

ProcessNode* moogHighPass(double sample_rate, double length, TransformType mode, SaturationType saturation_mode);
ProcessNode* moogLowPass(double sample_rate, double length, TransformType mode, SaturationType saturation_mode);

ProcessNode* changeVolume(double sample_rate, double length, TransformType mode);
ProcessNode* expShape(double sample_rate, double length, TransformType mode);
ProcessNode* limiter(double sample_rate, double length, TransformType mode);
ProcessNode* ringMod(double sample_rate, double length, TransformType mode);
ProcessNode* saturationTanh(double sample_rate, double length, TransformType mode);
ProcessNode* simpleReverb(double sample_rate, double length, TransformType mode, double rel_length);
ProcessNode* simpleReverseReverb(double sample_rate, double length, TransformType mode, double rel_length);

ProcessNode* constantSignal(double sample_rate, double length, TransformType mode);
ProcessNode* polysynthSwtPulse(double sample_rate, double length, TransformType mode, double start_phase_swt,
                               double start_phase_pulse);
ProcessNode* superwaveOscillator(double sample_rate, double length, TransformType mode, OscillatorType type,
                                 double start_phase, uint32_t seed);
ProcessNode* superwaveOscillatorShaped(double sample_rate, double length, TransformType mode, ShapedOscillatorType type,
                                       double start_phase, uint32_t seed);
ProcessNode* waveOscillator(double sample_rate, double length, TransformType mode, OscillatorType type,
                            double start_phase);
ProcessNode* waveOscillatorShaped(double sample_rate, double length, TransformType mode, ShapedOscillatorType type,
                                  double start_phase);
ProcessNode* whiteNoise(double sample_rate, double length, TransformType mode, uint32_t seed);

ProcessNode* improvedStereo(double sample_rate, double length, TransformType mode);
ProcessNode* legacyStereo(double sample_rate, double length, TransformType mode);
ProcessNode* simpleStereo(double sample_rate, double length, TransformType mode);

]]

return ffi.C