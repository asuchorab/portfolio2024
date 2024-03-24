local api = {
  SIN = {
    type = "number"
  },
  SQR = {
    type = "number"
  },
  SWT = {
    type = "number"
  },
  TRI = {
    type = "number"
  },
  SQRTRI = {
    type = "number"
  },
  PULSE = {
    type = "number"
  },
  NONE = {
    type = "number"
  },
  LIMIT = {
    type = "number"
  },
  TANH = {
    type = "number"
  },
  sample_rate = {
    type = "number",
    description = "global sample rate setting, default: 44100",
  },
  default_base = {
    type = "number",
    description = "global base frequency, used by some functions if another was not provided, it is recommended to set it to the key/scale of the musical piece, for C major/minor, use halftones(-9):getFrequency(440) / 9 halftones below 440 (A), default: 440",
  },
  default_timescale = {
    type = "Timescale",
    description = "global timescale, used by some functions if another was not provided, it is recommended to set it to the metre/bpm of the musical piece, default: ConstantTimescale.new(0, 120)",
  },
  default_scale = {
    type = "Scale",
    description = "global sacle, used bu some functions if another was not provided, it is recommended to set it to the scale of the musical piece, default: scales.major",
  },
  bpm_length = {
    type = "function",
    args = "(bpm: number)",
    valuetype = "number",
    description = "get beat length for the bpm",
  },
  dB_amp = {
    type = "function",
    args = "(dB: number)",
    valuetype = "number",
    description = "convert dB to amplitude",
  },
  amp_dB = {
    type = "function",
    args = "(amp: number)",
    valuetype = "number",
    description = "convert amplitude to dB",
  },
  lerp = {
    type = "function",
    args = "(v1: number, v2: number, t: number)",
    valuetype = "number",
    description = "linear interpolation from v1 to v2 at position t (from 0 to 1)",
  },
  lerp_exp = {
    type = "function",
    args = "(v1: number, v2: number, t: number)",
    valuetype = "number",
    description = "linear interpolation on the exponential scale from v1 to v2 at position t (from 0 to 1)",
  },
  map_lin_exp = {
    type = "function",
    args = "(v1: number, v2: number, t: number)",
    valuetype = "number",
    description = "some math fuckery, not sure",
  },
  ParWrapper = {
    type = "class",
    description = "Represents a C++ parameter object, used as arguments for transforms, supports +, -, *, /, ^, .. with numbers and ParWrapper",
    childs = {
      sample_rate = {
        type = "function",
        args = "(new_sample_rate: number)",
        valuetype = "ParWrapper",
        description = "Set sample rate",
      },
      len = {
        type = "function",
        args = "(new_length: number)",
        valuetype = "ParWrapper",
        description = "Set length",
      },
      start_point = {
        type = "function",
        args = "(new_start_point: number)",
        valuetype = "ParWrapper",
        description = "Returns the par with start point changed to the provided value",
      },
      ret = {
        type = "function",
        args = "(point: number|nil)",
        valuetype = "ParWrapper",
        description = "Returns the par shifted in a way that its start is the parameter at provided time point, if point is nil, returns self",
      },
      offset = {
        type = "function",
        args = "(offset: number)",
        valuetype = "ParWrapper",
        description = "Aliast to ret but cannot take nil",
      },
      interval = {
        type = "function",
        args = "(offset: number, length: number)",
        valuetype = "ParWrapper",
        description = "Returns the interval starting at offset with requested length",
      },
    },
  },
  ParAdd = {
    type = "function",
    args = "(p1: ParWrapper|number, p2: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns the sum of the both arguments, called for + operator",
  },
  ParConcat = {
    type = "function",
    args = "(...)",
    valuetype = "ParWrapper",
    description = "Returns all the parameters in succession, all parameters need to have specified length, called for .. operator",
  },
  ParConst = {
    type = "function",
    args = "(value: number, other_sample_rate: number|nil)",
    valuetype = "ParWrapper",
    description = "Returns constant parameter with the specified value, without length, if other_sample_rate is nil, uses global sample_rate, used by other functions if number is provided instead of a ParWrapper",
  },
  ParConstAt = {
    type = "function",
    args = "(p1: ParWrapper)",
    valuetype = "ParWrapper",
    description = "Returns a paramterer that evaulates to a constant with value ",
  },
  ParDiv = {
    type = "function",
    args = "(p1: ParWrapper|number, p2: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns one parameter divided for the second one, called for / operator",
  },
  ParExp = {
    type = "function",
    args = "(p1: ParWrapper|number, p2: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns one parameter raised to power of the other, mirrored for negative numbers, called for ^ operator",
  },
  ParLerp = {
    type = "function",
    args = "(t1: number, p1: ParWrapper|number, t2: number, p2: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns the linear interpolation of p1 (100% at t1) and p2 (100% at t2)",
  },
  ParLimit = {
    type = "function",
    args = "(p1: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns the parameter limited to  <-1, 1>",
  },
  ParLinearRoExp = {
    type = "function",
    args = "(p1: ParWrapper|number, v1: number, v2: number)",
    valuetype = "ParWrapper",
    description = "Returns parameter converted from linear scale to exponential, in a way that v1 and v2 are values where the original and result are equal",
  },
  ParMul = {
    type = "function",
    args = "(p1: ParWrapper|number, p2: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns the product of the both arguments, called for * operator",
  },
  ParOscillator = {
    type = "function",
    args = "(freq: ParWrapper|number, base: ParWrapper|number|nil, amp: ParWrapper|number|nil, type: SIN|SQR|SWT|TRI|SQRTRI|nil, start_phase: number|nil)",
    valuetype = "ParWrapper",
    description = "Returns a oscillator/LFO parameter, of frequency freq, oscillating around base (default 0) with amplitude amp (default 1), with waveshape specified by type (default SIN), with start phase start_phase (default 0)",
  },
  ParOscillatorShaped = {
    type = "function",
    args = "(freq: ParWrapper|number, base: ParWrapper|number, amp: ParWrapper|number, shape: ParWrapper|number, type: PULSE|nil, start_phase: number|nil)",
    valuetype = "ParWrapper",
    description = "Returns a oscillator/LFO parameter with a shaped wave type, of frequency freq, oscillating around base with amplitude amp, with waveshape specified by shape and type (default PULSE), with start phase start_phase (default 0)",
  },
  ParRepeat = {
    type = "function",
    args = "(p1: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns the parameter p1 repeated indefinitely, including for the negative arguments",
  },
  ParSub = {
    type = "function",
    args = "(p1: ParWrapper|number, p2: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns the difference of the both arguments, called for - operator",
  },
  ParTanh = {
    type = "function",
    args = "(p1: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns the hyperbolic tangent (saturation/soft limiter) of the parameter",
  },
  ParVibrato = {
    type = "function",
    args = "(freq: ParWrapper|number, base: ParWrapper|number|nil, exp_amp: ParWrapper|number|nil, type: SIN|SQR|SWT|TRI|SQRTRI|nil, start_phase: number|nil)",
    valuetype = "ParWrapper",
    description = "Returns a oscillator/LFO parameter in exponential scale (useful from frequency parameters), of frequency freq, oscillating around base (default 0) with amplitude on exponential scale exp_amp (default 1), with waveshape specified by type (default SIN), with start phase start_phase (default 0)",
  },
  ParVibratoShaped = {
    type = "function",
    args = "(freq: ParWrapper|number, base: ParWrapper|number, exp_amp: ParWrapper|number, shape: ParWrapper|number, type: PULSE|nil, start_phase: number|nil)",
    valuetype = "ParWrapper",
    description = "Returns a oscillator/LFO parameter in exponential scale (useful from frequency parameters), oscillating around base with amplitude on the exponential scale amp, with waveshape specified by shape and type (default PULSE), with start phase start_phase (default 0)",
  },
  ParFragment = {
    type = "function",
    args = "(p1: ParWrapper|number, offset: number, length: number)",
    valuetype = "ParWrapper",
    description = "Returns the parameter p1 with specified length offset by offset, repeating indefinitely",
  },
  ParConstMin = {
    type = "function",
    args = "(value: number, other_sample_rate: number|nil)",
    valuetype = "ParWrapper",
    description = "Returns constant parameter with the specified value, with minimal nonzero sample length for the sample rate",
  },
  ParLinear = {
    type = "function",
    args = "(length: number, p1: ParWrapper|number, p2: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns linear interpolation parameter starting at 0 with given length, with value starting at p1 and ending at p2",
  },
  ParLinearExp = {
    type = "function",
    args = "(length: number, p1: ParWrapper|number, p2: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns linear interpolation parameter in exponential scale starting at 0 with given length, with value starting at p1 and ending at p2",
  },
  ParParabola = {
    type = "function",
    args = "(length: number, p1: ParWrapper|number, p2: ParWrapper|number, shape: number)",
    valuetype = "ParWrapper",
    description = "Returns parabola starting at 0 with given length, with value starting at p1 and ending at p2, with shape specifying the fraction of difference between p2 and p1 that it the parabola reaches at half time point (0.5 = linear)",
  },
  ParParabolaExp = {
    type = "function",
    args = "(length: number, p1: ParWrapper|number, p2: ParWrapper|number, shape: number)",
    valuetype = "ParWrapper",
    description = "Returns parabola in exponential scale starting at 0 with given length, with value starting at p1 and ending at p2, with shape specifying the fraction of difference between p2 and p1 that it the parabola reaches at half time point (0.5 = linear)",
  },
  ParSmoothChange = {
    type = "function",
    args = "(length: number, p1: ParWrapper|number, p2: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns a parameter going from p1 to p2 over length, smoothness refers to the slope at the beginning and the end, the derivative at the beginning and the end is equal 0",
  },
  ParSmoothChangeExp = {
    type = "function",
    args = "(length: number, p1: ParWrapper|number, p2: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns a parameter going from p1 to p2 over length in exponential scale, smoothness refers to the slope at the beginning and the end, the derivative at the beginning and the end is equal 0",
  },
  ParSlide = {
    type = "function",
    args = "(length: number, p1: ParWrapper|number, p2: ParWrapper|number)",
    valuetype = "ParWrapper",
    description = "Returns a parameter suited for creating a slide note, from p1 to p2 over length, technically it's LinearExp from p1 to p2 and then constant of p2",
  },
  ParSmoothBeginningEnding = {
    type = "function",
    args = "(length: number, begin_length: number, end_length: number|nil)",
    valuetype = "ParWrapper",
    description = "Returns a parameter that is almost a constant 1 but goes from 0 to 1 for begin_length and from 1 to 0 at the end for end_length, if end_length is nil, begin_length is used can be useful as a simple way of declicking",
  },
  ParSmoothEnding = {
    type = "function",
    args = "(length: number, end_length: number)",
    valuetype = "ParWrapper",
    description = "Returns a parameter that is almost a constant 1 but goes from 1 to 0 at the end for end_length, can be useful as a simple way of declicking",
  },
  ParSmoothBeginning = {
    type = "function",
    args = "(length: number, begin_length: number)",
    valuetype = "ParWrapper",
    description = "Returns a parameter that is almost a constant 1 but goes from 0 to 1 for begin_length, can be useful as a simple way of declicking",
  },
  EnvelopeSimple = {
    type = "function",
    args = "(duration: number, attack: number, decay: number, sustain: number, release: number, attack_level: number|nil)",
    valuetype = "ParWrapper",
    description = "Returns a parameter for simple envelope, it rises to attack_level or 1 during attack, then decays to sustain and after duration, releases to 0",
  },
  EnvelopeControl = {
    type = "class"
  },
  envConst = {
    type = "function",
    args = "(duration: number)",
    valuetype = "EnvelopeControl",
    description = "constant fragment in Envelope",
  },
  envLinear = {
    type = "function",
    args = "(duration: number)",
    valuetype = "EnvelopeControl",
    description = "linear fragment in Envelope (uses ParLinear)",
  },
  envParabola = {
    type = "function",
    args = "(duration: number, shape: number)",
    valuetype = "EnvelopeControl",
    description = "parabola fragment in Envelope (uses ParParabola with shape)",
  },
  envSmooth = {
    type = "function",
    args = "(duration: number)",
    valuetype = "EnvelopeControl",
    description = "smooth fragment in Envelope (uses ParSmoothChange)",
  },
  envExp = {
    type = "function",
    args = "(duration: number)",
    valuetype = "EnvelopeControl",
    description = "smooth fragment in Envelope (uses ParLinearExp)",
  },
  envParabolaExp = {
    type = "function",
    args = "(duration: number, shape: number)",
    valuetype = "EnvelopeControl",
    description = "smooth fragment in Envelope (uses ParParabolaExp with shape)",
  },
  envSmoothExp = {
    type = "function",
    args = "(duration: number)",
    valuetype = "EnvelopeControl",
    description = "smooth fragment in Envelope (uses ParSmoothChangeExp)",
  },
  Envelope = {
    type = "function",
    args = "(duration: number, segments: table, start_point: number|nil, timescale: Timescale|nil)",
    valuetype = "ParWrapper",
    description = "creates a envelope with custom shape, starting at start_point or 0, ending after given duration, segments table specifies the shape of the envelope; segments table can contain keys: 'before_begin', 'after_begin', 'before_end', 'after_end', each of these can be an array (keys 1,2,3...) of numbers and EnvelopeControl (functions prefixed env), each EnvelopeControl except envConst requires a number before and after it, which are its starting and ending values, envConst only requires a number after it, excess numbers do not cause errors but some of them will be ignored; all durations are specified in beats, not seconds and provided timescale or default_timescale is used",
  },
  BufferWrapper = {
    type = "class",
    description = "Represents a C++ buffer object; all sound generation and processing involves a transform on a buffer, each transform has its method in the BufferWrapper class, a transform will always have a start_point and length that represent the time interval on the buffer where the transform is executed, if they are not given as arguments, they will be equal to the current interval of the buffer, or will produce error if no interval is present (hasLength() == false); buffer transforms are organized in levels, a level is a set of transforms on the buffer that can be computed in any order and benefit from parallel processing if they don't intersect (checking if they intersect is handled by the C++ library); two notes that are copied into the buffer can be copied independently from each other so they can be on the same level; two filters usually cannot be executed in any order, so they should be on separate levels; all transform functions and note outputs automatically handle levels in a way that lets the user think as if everything happens sequentially, but the user can still try to optimize levels using dont_end_level argument of transforms and calling endLevel manually",
    childs = {
      newMono = {
        type = "function",
        args = "()",
        valuetype = "BufferWrapper",
        description = "Creates a new mono buffer",
      },
      newStereo = {
        type = "function",
        args = "()",
        valuetype = "BufferWrapper",
        description = "Creates a new stereo buffer",
      },
      getLength = {
        type = "method",
        args = "()",
        valuetype = "number",
        description = "Returns length of the buffer or NaN (0/0); length usually depends on transforms/output notes on this buffer",
      },
      getOffset = {
        type = "method",
        args = "()",
        valuetype = "number",
        description = "Returns length of the buffer or NaN (0/0); offset usually depends on transforms/output notes on this buffer",
      },
      hasLength = {
        type = "method",
        args = "()",
        valuetype = "boolean",
        description = "Whether the buffer has specified length and offset, will be false if there was no transform registered on it, if it's false, getLength and getOffset return NaN (0/0) and transforms cannot be registered on the buffer without specifying offset/start_point and length",
      },
      isMono = {
        type = "method",
        args = "()",
        valuetype = "boolean",
        description = "Whather the buffer is mono",
      },
      isStereo = {
        type = "method",
        args = "()",
        valuetype = "boolean",
        description = "Whather the buffer is stereo",
      },
      endLevel = {
        type = "method",
        args = "()",
        valuetype = "nil",
        description = "Ends a level in buffer",
      },
      bufferFromRaw = {
        type = "method",
        args = "(filename: string, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Reads a raw file (sequence of double-precision floating numbers) into the buffer, due to design of the library, start point and length are specified during reading of the script, so they cannot be read from the file; the program will log a warning if the buffer is too short for the file",
      },
      bufferFromWAV = {
        type = "method",
        args = "(filename: string, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Reads a WAV file (can't handle all possible formats, can read basic 16bit signed data which is the most common) into the buffer, due to design of the library, start point and length are specified during reading of the script, so they cannot be read from the file; the program will log a warning if the buffer is too short for the file",
      },
      bufferToRaw = {
        type = "method",
        args = "(filename: string, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Saves a raw file (sequence of double-precision floating numbers)",
      },
      bufferToWAV = {
        type = "method",
        args = "(filename: string, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Saves a WAV file in the 16bit signed format",
      },
      copyBuffer = {
        type = "method",
        args = "(destination: BufferWrapper, dest_start_point: number, multiplier: ParWrapper|number|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Copies contents of this buffer to the destination, the zero point of this buffer will correspond to dest_start_point on the destination buffer; if this buffer starts from 30 and dest_start_point is 30, the beginning of this buffer will be copied to point 60 on the destination buffer; multiplier changes volume of the copied data in the destination buffer without changing this buffer, default: 1 (0dB), by default, this function ends levels in both buffers, which may make it undesirable for copying individual notes (use stereo functions instead)",
      },
      normalize = {
        type = "method",
        args = "(allow_volume_up: boolean|nil, provide_feedback: boolean|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Changes volume of the buffer so that the absolute maximum doesn't exceed 1; if allow_volume_up is true (default: false), it will also increase volume to make the absolute maximum 1 if the absolute maximum was less than 1; if provide_feedback is true (default: false), the program will log how was the volume changed during execution",
      },
      softClipCompressor = {
        type = "method",
        args = "(linear_threshold: ParWrapper|number, linear_amp: ParWrapper|number, tanh_wet_dry: ParWrapper|number, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "applies a soft clipping compression based on hyperbolic tangent saturation for the sound that exceeds the linear_threshold, linear_amp is multiplier to the sound applied before the compression, tanh_wet_dry is the mix ratio of the hyperbolic tangent, if it's 0, nothing happens, if it's 1, the sound is limited to maximum 1 (practically always lower because hyperbolic tangent approaches -1/1 for -inf/inf",
      },
      butterworthBandPass = {
        type = "method",
        args = "(iterations: number, freq1: ParWrapper|number, freq2: ParWrapper|number, resonance: ParWrapper|number|nil, wet_dry: ParWrapper|number|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies a butterworth band pass filter, from freq1 to freq2, with given resonance (default: 0) and wet_dry mix (default: 1), iterations is the strength/roll-off/poles of the filter (4 poles = 1 iteration), resonance of 1 and higher will result in filter instability",
      },
      butterworthBandStop = {
        type = "method",
        args = "(iterations: number, freq1: ParWrapper|number, freq2: ParWrapper|number, resonance: ParWrapper|number|nil, wet_dry: ParWrapper|number|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies a butterworth band stop filter, from freq1 to freq2, with given resonance (default: 0) and wet_dry mix (default: 1), iterations is the strength/roll-off/poles of the filter (4 poles = 1 iteration), resonance of 1 and higher will result in filter instability",
      },
      butterworthHighPass = {
        type = "method",
        args = "(iterations: number, freq: ParWrapper|number, resonance: ParWrapper|number|nil, wet_dry: ParWrapper|number|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies a butterworth high pass filter, with cutoff of freq, with given resonance (default: 0) and wet_dry mix (default: 1), iterations is the strength/roll-off/poles of the filter (2 poles = 1 iteration), resonance of 1 and higher will result in filter instability",
      },
      butterworthLowPass = {
        type = "method",
        args = "(iterations: number, freq: ParWrapper|number, resonance: ParWrapper|number|nil, wet_dry: ParWrapper|number|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies a butterworth low pass filter, with cutoff of freq, with given resonance (default: 0) and wet_dry mix (default: 1), iterations is the strength/roll-off/poles of the filter (2 poles = 1 iteration), resonance of 1 and higher will result in filter instability",
      },
      flangerFeedback = {
        type = "method",
        args = "(freq: ParWrapper|number, wet_dry: ParWrapper|number, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies a flanger with a feedback loop, with delay period of given freq, with given wet_dry mix, wet_dry of 1 and higher will result in filter instability",
      },
      flangerFeedforward = {
        type = "method",
        args = "(freq: ParWrapper|number, wet_dry: ParWrapper|number, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies a flanger, with delay period of given freq, with given wet_dry mix",
      },
      flangerSelfOscillation = {
        type = "method",
        args = "(freq: ParWrapper|number, oscil_length: ParWrapper|number, correction: ParWrapper|number, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies a flanger with a feedback loop with auto-adjusting wet_dry that can produce interesting effects as the flanger self-oscilates long after any input is gone, it oscillates with base frequency of freq, with approximate time of being loud based on oscil_length, the correction parameter describes the correction to parameter based on frequency, at correction = 1, the self-oscillations will decay at the same rate for the whole frequency spectrum, at correction = 0, no correction will be applied and higher frequencies will decay much faster than lower ones, which may be desirable or not",
      },
      phaser = {
        type = "method",
        args = "(stages: number, coefficient: ParWrapper|number, feedback: ParWrapper|number|nil, wet_dry: ParWrapper|number|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies a phaser with given amount of stages (typically an even number, at least 2), this implementation cannot be tied to one frequency and instead uses the coefficient for the all-pass filter in <-1, 1>, with given feedback (default:0) and wet_dry mix ratio (default: 1), coefficient and feedback of absolute value over 1 will result in filter instability",
      },
      moogHighPass = {
        type = "method",
        args = "(saturation: NONE|LIMIT|TANH, freq: ParWrapper|number, resonance: ParWrapper|number|nil, wet_dry: ParWrapper|number|nil, inner_multiplier: ParWrapper|number|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies an approximation of moog high pass filter, using given inner saturation type, with given cutoff frequency, with given resonance (default: 0) and given wet_dry mix ratio (default: 1), with given inner_multiplier which changes the gain before the saturation between stages (default: 1), very high frequency values and resonance greater or equal 1 will result in filter instability",
      },
      moogLowPass = {
        type = "method",
        args = "(saturation: NONE|LIMIT|TANH, freq: ParWrapper|number, resonance: ParWrapper|number|nil, wet_dry: ParWrapper|number|nil, inner_multiplier: ParWrapper|number|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies an approximation of moog low pass filter, using given inner saturation type, with given cutoff frequency, with given resonance (default: 0) and given wet_dry mix ratio (default: 1), with given inner_multiplier which changes the gain before the saturation between stages (default: 1), very high frequency values and resonance greater or equal 1 will result in filter instability",
      },
      changeVolume = {
        type = "method",
        args = "(multiplier: ParWrapper|number, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Changes the volume of the sound, multiplying samples by given multiplier, to use dB, see dB_amp()",
      },
      expShape = {
        type = "method",
        args = "(exponent: ParWrapper|number, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Shapes the waveform, raising samples to the given power (exponent), mirrored for negative numbers, this can result in distortion/saturation",
      },
      limiter = {
        type = "method",
        args = "(limit: ParWrapper|number|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Cuts all samples that exceed <-limit, limit> (default: <-1, 1>)",
      },
      ringMod = {
        type = "method",
        args = "(other: BufferWrapper, other_start_point: number|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies ring modulation by multiplying samples of this by samples of other, other_start_point is the point on the other buffer corresponding to zero on this buffer (default: 0)",
      },
      saturationTanh = {
        type = "method",
        args = "(start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies hyperbolic tangent to samples of this buffer, resulting in saturation/distortion/compression",
      },
      simpleReverb = {
        type = "method",
        args = "(room_size: ParWrapper|number, damping: ParWrapper|number, wet_dry: ParWrapper|number, space_width: ParWrapper|number, rel_length: number|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies freeverb reverb to the buffer with given parameters, rel_length changes the periods of the comb filters to make the reverb shorter or longer (default: 1)",
      },
      simpleReverseReverb = {
        type = "method",
        args = "(room_size: ParWrapper|number, damping: ParWrapper|number, wet_dry: ParWrapper|number, space_width: ParWrapper|number, rel_length: number|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Applies freeverb reverb to the buffer with given parameters in reverse, rel_length changes the periods of the comb filters to make the reverb shorter or longer (default: 1)",
      },
      constantSignal = {
        type = "method",
        args = "(value: ParWrapper|number, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Adds the value directly to the buffer",
      },
      polysynthSwtPulse = {
        type = "method",
        args = "(freq_swt: ParWrapper|number, freq_pulse: ParWrapper|number, freq_mod_swt: ParWrapper|number, freq_mod_pulse: ParWrapper|number, swt_pulse_bias: ParWrapper|number, pulse_shape: ParWrapper|number, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Generates a cross-modulated sawtooth/pulse polysynth, with given frequencies freq_swt, freq_pulse, with cross-modulation coefficients of freq_mod_swt, freq_mod_pulse, with swt_pulse_bias describing balance between sawtooth and pulse (0 = sawtooth, 1 = pulse, 0.5 = balanced), and pulse_shape describing width of the pulse wave (1 = square wave, can approach 0 or 2)",
      },
      superwaveOscillator = {
        type = "method",
        args = "(freq: ParWrapper|number, mix: ParWrapper|number, detune: ParWrapper|number, type: SIN|SQR|SWT|TRI|SQRTRI|nil, start_point: number|nil, length: number|nil, seed: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Generates a 7-component superwave based on classic supersaw model, with given frequency, mix and detune, with given type (default: SWT), uses RNG based on seed to randomise phase offsets, if seed is not provided, it will be generated based on default lua generator and therefore can be deterministic depending on how the user seeds the default generator",
      },
      superwaveOscillatorShaped = {
        type = "method",
        args = "(freq: ParWrapper|number, mix: ParWrapper|number, detune: ParWrapper|number, shape: ParWrapper|number, type: PULSE|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Generates a 7-component superwave with parametrized type based on classic supersaw model, with given frequency, mix and detune, with given type (default: PULSE), parametrized with shape (pulse width is 1 = square wave, can approach 0 or 2), uses RNG based on seed to randomise phase offsets, if seed is not provided, it will be generated based on default lua generator and therefore can be deterministic depending on how the user seeds the default generator",
      },
      waveOscillator = {
        type = "method",
        args = "(freq: ParWrapper|number, type: SIN|SQR|SWT|TRI|SQRTRI|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Generates a simple wave with given frequency and type (default: SIN)",
      },
      waveOscillatorShaped = {
        type = "method",
        args = "(freq: ParWrapper|number, shape: ParWrapper|number, type: PULSE|nil, start_point: number|nil, length: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Generates a simple wave with given frequency and type (default: PULSE), parametrized with shape (pulse width is 1 = square wave, can approach 0 or 2)",
      },
      whiteNoise = {
        type = "method",
        args = "(start_point: number|nil, length: number|nil, seed: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Generates white noise with given random seed (converted to 32bit int), if seed is not provided, it will be generated based on default lua generator and therefore can be deterministic depending on how the user seeds the default generator",
      },
      improvedStereo = {
        type = "method",
        args = "(pan: ParWrapper|number, start_point: number|nil, length: number|nil, destination: BufferWrapper|nil, dest_start_point: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Changes pan of the sound, for 0, nothing happens, for negative values, there is more left and for positive values, there is more right, this model of stereo can raise volume of one of the channels over the original value and will never make one of the channels silent, works well for values <-10, 10>, rate of change falls exponentially the further the parameter is from 0, if destination is not nil, it doesn't modify the buffer but instead copies it to destination; by default, the function doesn't end level of the destination buffer, so it's best suited for copying individual notes allowing for parallelization",
      },
      legacyStereo = {
        type = "method",
        args = "(pan: ParWrapper|number, start_point: number|nil, length: number|nil, destination: BufferWrapper|nil, dest_start_point: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Changes pan of the sound, for 0, nothing happens, for negative values, there is more left and for positive values, there is more right, this model of stereo will never make one of the channels silent, it is the previous version of improvedStereo function and is probably worse, works well for values <-10, 10>, rate of change falls exponentially the further the parameter is from 0, if destination is not nil, it doesn't modify the buffer but instead copies it to destination; by default, the function doesn't end level of the destination buffer, so it's best suited for copying individual notes allowing for parallelization",
      },
      simpleStereo = {
        type = "method",
        args = "(pan: ParWrapper|number, start_point: number|nil, length: number|nil, destination: BufferWrapper|nil, dest_start_point: number|nil, dont_end_level: boolean|nil)",
        valuetype = "nil",
        description = "Changes pan of the sound, for 0, nothing happens, for negative values, there is more left and for positive values, there is more right, this model of stereo will simply decrease volume of one of the channels, reaching 0 for L in pan = 1 and 0 for R in pan = -1, I think this is what audacity uses, works well for values <-1, 1>, if destination is not nil, it doesn't modify the buffer but instead copies it to destination; by default, the function doesn't end level of the destination buffer, so it's best suited for copying individual notes allowing for parallelization",
      },
    },
  },
  FunctionObject = {
    type = "class",
    description = "A class of objects for building expressions, they cover basically all parameter types (ParWrapper), and can be queried for value of the function at given time point or parameter, supports operators +, -, *, /, ^",
    childs = {
      ret = {
        type = "method",
        args = "(argument: number|nil)",
        valuetype = "number",
        description = "returns the value of the function for given argument, nil is a valid argument for Const FunctionObject and all time-invariant operations on Const FunctionObjects",
      },
      par = {
        type = "method",
        args = "()",
        valuetype = "ParWrapper",
        description = "returns the parameter (ParWrapper) representing the function",
      },
      offset = {
        type = "method",
        args = "(start: number)",
        valuetype = "FunctionObject",
        description = "returns the function but with given argument offset",
      },
    },
  },
  Var = {
    type = "function",
    args = "()",
    valuetype = "FunctionObject",
    description = "returns FunctionObject for identity function",
  },
  SetStart = {
    type = "function",
    args = "(p1: FunctionObject|number, offset: number)",
    valuetype = "FunctionObject",
    description = "returns p1 with given argument offset, so that it starts at the new start",
  },
  Add = {
    type = "function",
    args = "(p1: FunctionObject|number, p2: FunctionObject|number)",
    valuetype = "FunctionObject",
    description = "returns addition of p1 and p2",
  },
  Concat = {
    type = "function",
    args = "(...)",
    valuetype = "FunctionObject",
    description = "Returns concatenation of function objects with given length, similar to ParConcat; FunctionObjects without innate length require length (number) as the next positional argument",
  },
  Const = {
    type = "function",
    args = "(v: number)",
    valuetype = "FunctionObject",
    description = "Returns the FunctionObject for constant function of value v, for most uses it is not required, as most functions will implicitly convert a number to Const",
  },
  ConstAt = {
    type = "function",
    args = "(p1: FunctionObject|number, offset: number)",
    valuetype = "FunctionObject",
    description = "Returns the FunctionObject that returns opposite of SetStart when asked for a number, but ParConstAt when asked for a parameter",
  },
  Div = {
    type = "function",
    args = "(p1: FunctionObject|number, p2: FunctionObject|number)",
    valuetype = "FunctionObject",
    description = "returns division of p1 and p2",
  },
  Exp = {
    type = "function",
    args = "(p1: FunctionObject|number, p2: FunctionObject|number)",
    valuetype = "FunctionObject",
    description = "returns p1 to the power of p2, mirrored for negative p1",
  },
  Lerp = {
    type = "function",
    args = "(t1: number, p1: FunctionObject|number, t1: number, p2: FunctionObject|number)",
    valuetype = "FunctionObject",
    description = "Returns linear interpolation of p1 and p2, with 100% p1 at argument/time t1 and 100% p2 at argument/time t2, similar to ParLerp",
  },
  Linear = {
    type = "function",
    args = "(len: number, p1: FunctionObject|number, p2: FunctionObject|number)",
    valuetype = "FunctionObject",
    description = "Returns linear interpolation of p1 and p2, with 100% p1 at argument/time 0 and 100% p2 at argument/time len, similar to ParLinear",
  },
  LinearExp = {
    type = "function",
    args = "(len: number, p1: FunctionObject|number, p2: FunctionObject|number)",
    valuetype = "FunctionObject",
    description = "Returns linear interpolation on the exponential scale of p1 and p2, with 100% p1 at argument/time 0 and 100% p2 at argument/time len, similar to LinearExp",
  },
  LinearToExp = {
    type = "function",
    args = "(p1: FunctionObject|number, v1: number, v2: number)",
    valuetype = "FunctionObject",
    description = "Returns mapping from linear to exponential scale, with input and output being equal at v1 and v2, similar to ParLinearToExp",
  },
  Limit = {
    type = "function",
    args = "(p1: FunctionObject|number)",
    valuetype = "FunctionObject",
    description = "Returns argument limited to <-1, 1>, similar to ParLimit",
  },
  Mul = {
    type = "function",
    args = "(p1: FunctionObject|number, p2: FunctionObject|number)",
    valuetype = "FunctionObject",
    description = "returns multiplication of p1 and p2",
  },
  Oscillator = {
    type = "function",
    args = "(freq: FunctionObject|number, base: FunctionObject|number, amp: FunctionObject|number, type: SIN|SQR|SWT|TRI|SQRTRI|nil, start_phase: number)",
    valuetype = "FunctionObject",
    description = "returns a LFO parameter, similar to ParOscillator, due to problems with computing the value for variable frequencies (would require integration), attempting to :ret this type of FunctionObject results in an error, prompting the user to use :par instead",
  },
  OscillatorShaped = {
    type = "function",
    args = "(freq: FunctionObject|number, base: FunctionObject|number, amp: FunctionObject|number, shape: FunctionObject|number, type: PULSE|nil, start_phase: number)",
    valuetype = "FunctionObject",
    description = "returns a LFO parameter, similar to ParOscillatorShaped, due to problems with computing the value for variable frequencies (would require integration), attempting to :ret this type of FunctionObject results in an error, prompting the user to use :par instead",
  },
  Parabola = {
    type = "function",
    args = "(len: number, v1: number, v2: number, shape: number)",
    valuetype = "FunctionObject",
    description = "returns parabola between v1 and v2 over len, shape in what point between v1 and v2 the value is at half time point, similar to ParParabola",
  },
  ParabolaExp = {
    type = "function",
    args = "(len: number, v1: number, v2: number, shape: number)",
    valuetype = "FunctionObject",
    description = "returns parabola on exponential scale between v1 and v2 over len, shape in what point between v1 and v2 the value is at half time point, similar to ParParabolaExp",
  },
  Sub = {
    type = "function",
    args = "(p1: FunctionObject|number, p2: FunctionObject|number)",
    valuetype = "FunctionObject",
    description = "returns subtraction of p1 and p2",
  },
  Tanh = {
    type = "function",
    args = "(p1: FunctionObject|number)",
    valuetype = "FunctionObject",
    description = "returns hyperbolic tangent of p1",
  },
  Vibrato = {
    type = "function",
    args = "(freq: FunctionObject|number, base: FunctionObject|number, exp_amp: FunctionObject|number, type: SIN|SQR|SWT|TRI|SQRTRI|nil, start_phase: number)",
    valuetype = "FunctionObject",
    description = "returns a LFO parameter on exponential scale, similar to ParVibrato, due to problems with computing the value for variable frequencies (would require integration), attempting to :ret this type of FunctionObject results in an error, prompting the user to use :par instead",
  },
  VibratoShaped = {
    type = "function",
    args = "(freq: FunctionObject|number, base: FunctionObject|number, exp_amp: FunctionObject|number, shape: FunctionObject|number, type: PULSE|nil, start_phase: number)",
    valuetype = "FunctionObject",
    description = "returns a LFO parameter, similar to ParVibratoShaped, due to problems with computing the value for variable frequencies (would require integration), attempting to :ret this type of FunctionObject results in an error, prompting the user to use :par instead",
  },
  Timbre = {
    type = "class",
    childs = {
      new = {
        type = "function",
        args = "(func: function)",
        valuetype = "Timbre",
      },
      vars = {
        type = "table"
      },
      output = {
        type = "method",
        args = "(buf: BufferWrapper, offset: number, melody: Melody, base: number|nil, timescale = Metre|nil, scale: Scale|nil)"
      }
    }
  },
}

local interpreter = {
  name = "luamusgen",
  description = "luamusgen interpreter",
  api = {"luamusgen"},
  frun = function(self,wfilename,rundebug)
    --if rundebug then
    --  ide:GetDebugger():SetOptions({})
    --end
    CommandLineRun(('luamusgen "%s"'):format(wfilename:GetFullPath()),self:fworkdir(wfilename),true,false)
  end,
  hasdebugger = false,
}

return {
  name = "luamusgen",
  description = "luamusgen interpreter",
  author = "AS",
  version = 0.1,

  onRegister = function(self)
    ide:AddAPI("lua", "luamusgen", api)
    ide:AddInterpreter("luamusgen", interpreter)
  end,

  onUnRegister = function(self)
    ide:RemoveAPI("lua", "luamusgen")
    ide:RemoveInterpreter("luamusgen")
  end,
}