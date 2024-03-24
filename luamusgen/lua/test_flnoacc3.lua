--if not lua_segment_processing then
--  require "test_project"
--  return
--end
require "luamusgen"
local c = require "common"
local ro = c.defaults()
local m = require "test_melodies"

math.randomseed(10004)

local flnoacc = Timbre.new(function(buf, frequency, duration, start_point, duration_beats, start_beat, timbre_vars, note_vars)
  local buf_temp = BufferWrapper.newMono()
  buf_temp:whiteNoise(start_point - ro:dur(0.5), ro:dur(2.5))
  buf_temp:changeVolume(Envelope(duration_beats + 0.5, {after_begin = {envConst(0.5), 1, envParabola(0.5, 0.75), 0}}))
  --buf_temp:changeVolume(ParConst(1):len(ro:dur(0.5)) .. ParParabola(ro:dur(1), 1, 0, 0.75))
  --buf_temp:flangerFeedback(frequency * math.pow(2, 0.01 * pseudo_exp(math.sin(3.2 * math.log(frequency, 2)), 2)), 0.993, nil, ro:dur(16))
  buf_temp:flangerSelfOscillation(frequency * math.pow(2, 0.01 * pseudo_exp(math.sin(3.2 * math.log(frequency, 2)), 2)), 0.005, 0.001, nil, ro:dur(16))
  buf_temp:changeVolume(0.5 * Envelope(duration_beats + 0.5, {after_begin = {envConst(0.5), 0, 2, envParabola(1, 0.75), 1}, after_end = {1, envLinear(2), 0}}))
  buf_temp:butterworthHighPass(2, frequency * 0.9, 0.3)
  buf_temp:moogLowPass(NONE, frequency * 1.01, 0.9, 0.3)
  buf_temp:moogLowPass(NONE, frequency * 3.03, 0.85, 0.2)
  local v = math.pow(frequency / (halftones(3) + octaves(-1)):getFrequency(440), -0.5)
  local a = math.pow(math.random() * 18, 3) + math.pow(math.random() * 5, 5)
  local freq_add = a * math.pow(frequency / (halftones(3) + octaves(2)):getFrequency(440), 0.5)
  buf_temp:butterworthLowPass(1, frequency * 2 + freq_add * Envelope(duration_beats + 0.5, {after_begin = {envConst(0.5), 1, envParabola(4, 0.6), 0}} ))
  local panOffset = 0.4 * pseudo_exp(math.sin(2 * math.log(frequency, 2)), 2)
  buf_temp:changeVolume((v * math.pow(0.998, a / 16)) * timbre_vars.vol:ret(start_point))
  buf_temp:improvedStereo(timbre_vars.pan:ret(start_point) + panOffset, start_point, ro:dur(16), buf, 0)
end)

--local test_buf = BufferWrapper.newMono()
--test_buf:constantSignal(Envelope(10, {after_begin = {1, envParabola(0.75, 4), 0}} ), 0, 10)
--test_buf:normalize()
--test_buf:bufferToWAV("test_buf.wav")

flnoacc.vars.pan = 0

do
  local buf = BufferWrapper.newStereo()

  local function generate()
    flnoacc.vars.vol = 1
    flnoacc:outputRepeat(buf, ro:beats(0, 0), 3, ro:beats(8, 0), m.bmel1)
    --flnoacc:outputRepeat(buf, ro:beats(0, 0), 3, ro:beats(8, 0), m.bmel2)
    flnoacc:outputRepeat(buf, ro:beats(0, 0), 2, ro:beats(16, 0), m.bmel2v2)
    flnoacc:outputRepeat(buf, ro:beats(8, 0), 1, ro:beats(16, 0), m.bmel2)
    flnoacc:output(buf, ro:beats(24, 0), m.bmel1v)
    flnoacc:output(buf, ro:beats(24, 0), m.bmel2v)
    flnoacc:output(buf, ro:beats(0, 0), m.bmelc2ds)
    flnoacc:output(buf, ro:beats(16, 0), m.bmelc2ds)
    flnoacc:output(buf, ro:beats(0, 0), m.bmelcwv:transpose(-1))
    flnoacc:output(buf, ro:beats(16, 0), m.bmelcwv2:transpose(-1):fragment(ro:beats(0, 0), ro:beats(14, 2)))
    
    flnoacc.vars.vol = 1.3
    flnoacc:outputRepeat(buf, ro:beats(0, 0), 2, ro:beats(16, 0), m.bmel2v2:transpose(-2))
    flnoacc:output(buf, ro:beats(8, 0), m.bmel2:transpose(-2))
    flnoacc:output(buf, ro:beats(24, 0), m.bmel2:transpose(-2):fragment(ro:beats(0, 0), ro:beats(6, 2)))
  end
    
  flnoacc.vars.pan = 0.3
    generate()
  flnoacc.vars.pan = -0.3
    generate()

  c.reverbBuf(buf, ro)

  buf:normalize(true)
  buf:bufferToWAV("o_flnoacc3_1.wav")
end

