--if not lua_segment_processing then
--  require "test_project"
--  return
--end
require "luamusgen"
local c = require "common"
local ro = c.defaults()
local m = require "test_melodies"

local poly3accsm = Timbre.new(function(buf, frequency, duration, start_point, duration_beats, start_beat, timbre_vars, note_vars)
  local buf_temp = BufferWrapper.newMono()
  buf_temp:polysynthSwtPulse(ParVibrato(1/ro:dur(0.23), frequency, 0.01), ParVibrato(1/ro:dur(0.34), frequency, 0.01), 0.002, 0.002, 0.4, 0.8, start_point, duration + ro:dur(2.2))
  buf_temp:changeVolume(ParLinear(ro:dur(0.7), 0, 1) .. ParParabola(ro:dur(0.2), 1, 0, 0.75))
  buf_temp:moogLowPass(LIMIT, frequency * Envelope(duration_beats, {after_begin = {8, envParabola(3, 0.65), 2.02}}), 0.4)
  buf_temp:changeVolume(3.4)
  buf_temp:moogLowPass(LIMIT, frequency *Envelope(duration_beats, {after_begin = {4, envParabola(3, 0.65), 1.01}}), 0.4, 0.9)
  buf_temp:changeVolume(3.4 * 1.5)
  buf_temp:saturationTanh()
  buf_temp:changeVolume(timbre_vars.vol:ret(start_point) * 0.7 * Envelope(duration_beats, {after_end = {1, envLinear(1), 0}}))
  buf_temp:butterworthHighPass(2, frequency)
  local panOffset = 0.3 * pseudo_exp(math.sin(5 * math.log(frequency, 2)), 2)
  --buf_temp:changeVolume(ParConst(0):len(ro:dur(0.5)) .. ParSmoothBeginningEnding(ro:dur(16), 0.001))
  buf_temp:improvedStereo(timbre_vars.pan:ret(start_point) + panOffset, nil, nil, buf, 0)
end)

poly3accsm.vars.vol = 1
poly3accsm.vars.pan = 0

do
  local buf = BufferWrapper.newStereo()

  poly3accsm:output(buf, ro:beats(0, 0), m.bmel1)
  poly3accsm:output(buf, ro:beats(0, 0), m.bmel2)
  poly3accsm:output(buf, ro:beats(0, 0), m.bmelc)
  poly3accsm:output(buf, ro:beats(0, 0), m.bmelc:transpose(-1))
  poly3accsm:output(buf, ro:beats(8, 0), m.bmel1v)
  poly3accsm:output(buf, ro:beats(8, 0), m.bmel2v)
  poly3accsm:output(buf, ro:beats(8, 0), m.bmelcv)
  poly3accsm:output(buf, ro:beats(8, 0), m.bmelcv:transpose(-1))
  
  poly3accsm:output(buf, ro:beats(16, 0), m.bmel1)
  poly3accsm:output(buf, ro:beats(16, 0), m.bmel2)
  poly3accsm:output(buf, ro:beats(16, 0), m.bmelc)
  poly3accsm:output(buf, ro:beats(16, 0), m.bmelc:transpose(-1))
  poly3accsm:output(buf, ro:beats(24, 0), m.bmel1v)
  poly3accsm:output(buf, ro:beats(24, 0), m.bmel2v)
  poly3accsm:output(buf, ro:beats(24, 0), m.bmelcv)
  poly3accsm:output(buf, ro:beats(24, 0), m.bmelcv:transpose(-1))
  
  poly3accsm.vars.vol = 0.5
  poly3accsm:output(buf, ro:beats(16, 1), m.bmel1:transpose(1))
  poly3accsm:output(buf, ro:beats(16, 1), m.bmel2:transpose(1))
  poly3accsm:output(buf, ro:beats(16, 1), m.bmelc:transpose(1))
  poly3accsm:output(buf, ro:beats(24, 1), m.bmel1v:transpose(1))
  poly3accsm:output(buf, ro:beats(24, 1), m.bmel2v:transpose(1))
  poly3accsm:output(buf, ro:beats(24, 1), m.bmelcv:transpose(1))

  local buf_2 = BufferWrapper.newStereo()
  buf:copyBuffer(buf_2, 0)
  c.reverbBuf2(buf_2, ro)
  buf_2:copyBuffer(buf, ro:dur(1), 0.5)
  buf_2:copyBuffer(buf, ro:dur(2), 0.25)
  buf_2:copyBuffer(buf, ro:dur(3), 0.15)
  
  buf:normalize(true)
  buf:bufferToWAV("o_poly3accsm_1.wav")
end

do
  local buf = BufferWrapper.newStereo()
  
  poly3accsm:output(buf, ro:beats(0, 0), m.bmel1)
  poly3accsm:output(buf, ro:beats(0, 0), m.bmel2)
  poly3accsm:output(buf, ro:beats(0, 0), m.bmelc)
  poly3accsm:output(buf, ro:beats(0, 0), m.bmelc:transpose(-1))
  poly3accsm:output(buf, ro:beats(8, 0), m.bmel1v)
  poly3accsm:output(buf, ro:beats(8, 0), m.bmel2v)
  poly3accsm:output(buf, ro:beats(8, 0), m.bmelcv)
  poly3accsm:output(buf, ro:beats(8, 0), m.bmelcv:transpose(-1))

  local buf_2 = BufferWrapper.newStereo()
  buf:copyBuffer(buf_2, 0)
  c.reverbBuf2(buf_2, ro)
  buf_2:copyBuffer(buf, ro:dur(1), 0.5)
  buf_2:copyBuffer(buf, ro:dur(2), 0.25)
  buf_2:copyBuffer(buf, ro:dur(3), 0.15)
  
  buf:normalize(true)
  buf:bufferToWAV("o_poly3accsm_2.wav")
end
