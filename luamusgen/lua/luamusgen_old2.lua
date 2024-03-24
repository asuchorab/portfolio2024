-- Refer to luamusgen_plugin.lua (ZeroBrane plugin) for documentation

local cpp = luamusgen_use_ffi and require "luamusgen_ffi" or lua_segment_processing

SIN = cpp.SIN
SQR = cpp.SQR
SWT = cpp.SWT
TRI = cpp.TRI
SQRTRI = cpp.SQRTRI
PULSE = cpp.PULSE
NONE = cpp.NONE
LIMIT = cpp.LIMIT
TANH = cpp.TANH
MONO_IN_PLACE = cpp.MONO_IN_PLACE
STEREO_IN_PLACE = cpp.STEREO_IN_PLACE
MONO_TO_MONO = cpp.MONO_TO_MONO
STEREO_TO_STEREO = cpp.STEREO_TO_STEREO
MONO_TO_STEREO = cpp.MONO_TO_STEREO
STEREO_TO_MONO = cpp.STEREO_TO_MONO

--Util

local NaN = 0 / 0
local NO_LEN = -1e18
sample_rate = 44100

function bpm_length(bpm)
    return 60 / bpm
end

function dB_amp(dB)
    return 10 ^ (dB / 20)
end

function amp_dB(amp)
    if not verify_pnum(amp) then
        error("amplitude <= 0")
    end
    return 20 * math.log10(amp)
end

function lerp(v1, v2, t)
    return v1 + (v2 - v1) * t
end

function lerp_exp(v1, v2, t)
    local add_coeff = math.log(v1)
    local mul_coeff = math.log(v2) - add_coeff
    return math.exp(t * mul_coeff + add_coeff)
end

function map_lin_exp(v1, v2, t)
    local mul_coeff = (math.log(v2 / v1)) / (v2 - v1);
    local add_coeff = math.log(v1) - v1 * mul_coeff;
    return math.exp(t * mul_coeff + add_coeff)
end

local function concat_tables(t1, t2)
    local len1 = #t1
    for i, v in ipairs(t2) do
        t1[len1 + i] = v
    end
    return t1
end

local function verify_num(num)
    if type(num) ~= "number" or num ~= num then
        return nil
    else
        return num
    end
end

local function verify_unum(num)
    if type(num) ~= "number" or (not (num >= 0)) then
        return nil
    else
        return num
    end
end

local function verify_pnum(num)
    if type(num) ~= "number" or (not (num > 0)) then
        return nil
    else
        return num
    end
end


--Par wrapper

ParWrapper = {}
ParWrapper.__index = ParWrapper

function ParWrapper.new(node, length)
    node = cpp.addNode(node)
    return setmetatable({
        node = node,
        start = 0,
        length = length or NaN,
        sample_rate = cpp.getSampleRate(node)
    }, ParWrapper)
end

function ParWrapper.empty()
    return setmetatable({
        start = 0,
        length = NaN,
        sample_rate = NaN
    }, ParWrapper)
end

function ParWrapper:sample_rate(new_sample_rate)
    return setmetatable({
        node = self.node,
        start = self.start,
        length = self.length,
        sample_rate = new_sample_rate
    }, ParWrapper)
end

function ParWrapper:len(new_length)
    return setmetatable({
        node = self.node,
        start = self.start,
        length = new_length,
        sample_rate = self.sample_rate
    }, ParWrapper)
end

function ParWrapper:start_point(new_start_point)
    return setmetatable({
        node = self.node,
        start = -new_start_point,
        length = self.length,
        sample_rate = self.sample_rate
    }, ParWrapper)
end

function ParWrapper:offset(offset)
    return setmetatable({
        node = self.node,
        start = self.start + offset,
        length = self.length,
        sample_rate = self.sample_rate
    }, ParWrapper)
end

function ParWrapper:ret(point)
    if point and point ~= 0 then
        return setmetatable({
            node = self.node,
            start = self.start - point,
            length = self.length,
            sample_rate = self.sample_rate
        }, ParWrapper)
    else
        return self
    end
end

function ParWrapper:interval(offset, new_length)
    return setmetatable({
        node = self.node,
        start = self.start - offset,
        length = new_length,
        sample_rate = self.sample_rate
    }, ParWrapper)
end

local function verify_par(par)
    if verify_num(par) then
        return ParWrapper.new(cpp.parConst(sample_rate, par))
    elseif getmetatable(par) == ParWrapper then
        return par
    else
        return nil
    end
end

function ParAdd(p1, p2)
    p1 = verify_par(p1)
    p2 = verify_par(p2)
    local node = cpp.parAdd(p1.sample_rate)
    cpp.addArgument(node, p1.start or 0, p1.node, false, NO_LEN, false)
    cpp.addArgument(node, p2.start or 0, p2.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

function ParConcat(...)
    local verified = {}
    local i = 1
    for np, par in ipairs { ... } do
        verified[i] = verify_par(par)
        if verified[i] then
            if verified[i].node then
                if verified[i].length ~= verified[i].length then
                    error("Parameter " .. np .. " has no length")
                end
                i = i + 1
            else
                verified[i] = nil
            end
        else
            error("Invalid parameter " .. np)
        end
    end
    if #verified == 0 then
        error("no arguments provided")
    end
    local node = cpp.parConcat(verified[1] and verified[1].sample_rate)
    local total_length = 0
    for i, par in ipairs(verified) do
        cpp.addArgument(node, par.start or 0, par.node, true, par.length, false)
        total_length = total_length + par.length
    end
    return ParWrapper.new(node, total_length)
end

function ParConst(value, other_sample_rate)
    return ParWrapper.new(cpp.parConst(other_sample_rate or sample_rate, value))
end

function ParConstAt(p1)
    p1 = verify_par(p1)
    local node = cpp.parConstAt(p1.sample_rate)
    cpp.addArgument(node, p1.start or 0, p1.node, true, 1.99 / p1.sample_rate, false)
    return ParWrapper.new(node)
end

function ParDiv(p1, p2)
    p1 = verify_par(p1)
    p2 = verify_par(p2)
    local node = cpp.parDiv(p1.sample_rate)
    cpp.addArgument(node, p1.start or 0, p1.node, false, NO_LEN, false)
    cpp.addArgument(node, p2.start or 0, p2.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

function ParExp(p1, p2)
    p1 = verify_par(p1)
    p2 = verify_par(p2)
    local node = cpp.parExp(p1.sample_rate)
    cpp.addArgument(node, p1.start or 0, p1.node, false, NO_LEN, false)
    cpp.addArgument(node, p2.start or 0, p2.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

function ParLerp(t1, p1, t2, p2)
    p1 = verify_par(p1)
    p2 = verify_par(p2)
    local node = cpp.parLerp(p1.sample_rate, t1, t2)
    cpp.addArgument(node, p1.start or 0, p1.node, false, NO_LEN, false)
    cpp.addArgument(node, p2.start or 0, p2.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

function ParLimit(p1)
    p1 = verify_par(p1)
    local node = cpp.parLimit(p1.sample_rate)
    cpp.addArgument(node, p1.start or 0, p1.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

function ParLinearToExp(p1, v1, v2)
    p1 = verify_par(p1)
    local node = cpp.parLinearExp(p1.sample_rate, v1, v2)
    cpp.addArgument(node, p1.start or 0, p1.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

function ParMul(p1, p2)
    p1 = verify_par(p1)
    p2 = verify_par(p2)
    local node = cpp.parMul(p1.sample_rate)
    cpp.addArgument(node, p1.start or 0, p1.node, false, NO_LEN, false)
    cpp.addArgument(node, p2.start or 0, p2.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

function ParOscillator(freq, base, amp, type, start_phase)
    freq = verify_par(freq)
    base = verify_par(base) or ParConst(0, freq.sample_rate)
    amp = verify_par(amp) or ParConst(1, freq.sample_rate)
    type = type or cpp.SIN
    start_phase = start_phase or 0
    local node = cpp.parOscillator(freq.sample_rate, type, start_phase)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, true)
    cpp.addArgument(node, base.start or 0, base.node, false, NO_LEN, false)
    cpp.addArgument(node, amp.start or 0, amp.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

function ParOscillatorShaped(freq, base, amp, shape, type, start_phase)
    freq = verify_par(freq)
    base = verify_par(base)
    amp = verify_par(amp)
    shape = verify_par(shape)
    type = type or PULSE
    start_phase = start_phase or 0
    local node = cpp.parOscilltorShaped(freq.sample_rate, type, start_phase)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, true)
    cpp.addArgument(node, base.start or 0, base.node, false, NO_LEN, false)
    cpp.addArgument(node, amp.start or 0, amp.node, false, NO_LEN, false)
    cpp.addArgument(node, shape.start or 0, shape.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

function ParRepeat(p1)
    p1 = verify_par(p1)
    if not p1.length then
        p1 = p1:len(1)
    end
    local node = cpp.parRepeat(p1.sample_rate)
    cpp.addArgument(node, p1.start or 0, p1.node, true, p1.length, false)
    return ParWrapper.new(node)
end

function ParSub(p1, p2)
    p1 = verify_par(p1)
    p2 = verify_par(p2)
    local node = cpp.parSub(p1.sample_rate)
    cpp.addArgument(node, p1.start or 0, p1.node, false, NO_LEN, false)
    cpp.addArgument(node, p2.start or 0, p2.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

function ParTanh(p1)
    p1 = verify_par(p1)
    local node = cpp.parTanh(p1.sample_rate)
    cpp.addArgument(node, p1.start or 0, p1.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

--[[
function ParToArray(p1)
    p1 = verify_par(p1)
    local node = cpp.parToArray(p1.sample_rate)
    cpp.addArgument(node, p1.start or 0, p1.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end]]

function ParVibrato(freq, base, exp_amp, type, start_phase)
    freq = verify_par(freq)
    base = verify_par(base)
    exp_amp = verify_par(exp_amp)
    type = type or cpp.SIN
    start_phase = start_phase or 0
    local node = cpp.parVibrato(freq.sample_rate, type, start_phase)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, true)
    cpp.addArgument(node, base.start or 0, base.node, false, NO_LEN, false)
    cpp.addArgument(node, exp_amp.start or 0, exp_amp.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

function ParVibratoShaped(freq, base, exp_amp, shape, type, start_phase)
    freq = verify_par(freq)
    base = verify_par(base)
    exp_amp = verify_par(exp_amp)
    shape = verify_par(shape)
    type = type or PULSE
    start_phase = start_phase or 0
    local node = cpp.parVibratoShaped(freq.sample_rate, type, start_phase)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, true)
    cpp.addArgument(node, base.start or 0, base.node, false, NO_LEN, false)
    cpp.addArgument(node, exp_amp.start or 0, exp_amp.node, false, NO_LEN, false)
    cpp.addArgument(node, shape.start or 0, shape.node, false, NO_LEN, false)
    return ParWrapper.new(node)
end

function ParWrapper.__unm(p1)
    return ParSub(0, p1)
end

function ParWrapper.__add(p1, p2)
    return ParAdd(p1, p2)
end

function ParWrapper.__sub(p1, p2)
    return ParSub(p1, p2)
end

function ParWrapper.__mul(p1, p2)
    return ParMul(p1, p2)
end

function ParWrapper.__div(p1, p2)
    return ParDiv(p1, p2)
end

function ParWrapper.__pow(p1, p2)
    return ParExp(p1, p2)
end

function ParWrapper.__concat(p1, p2)
    return ParConcat(p1, p2)
end

function ConstantSignal(value)
    value = verify_par(value)
    local node = cpp.constantSignal()
end

--High level parameters

function ParFragment(p1, offset, length)
    return ParRepeat(p1:offset(offset)):len(length)
end

function ParConstMin(value, other_sample_rate)
    other_sample_rate = other_sample_rate or sample_rate
    return ParConst(value, other_sample_rate):len(1.99 / other_sample_rate)
end

function ParLinear(length, p1, p2)
    return ParLerp(0, p1, length, p2):len(length)
end

function ParLinearExp(length, p1, p2)
    return ParLinearToExp(ParLerp(0, p1, length, p2), p1, p2):len(length)
end

function ParParabola(length, p1, p2, shape)
    if (not shape) or shape == 0.5 then
        return ParLinear(length, p1, p2)
    end
    local diff = p2 - p1
    local b = p1 - 2 * diff * (1 - 2 * shape)
    local c = (4 * shape - 1) * diff + 2 * p1 - b
    return ParLerp(0, ParLerp(0, p1, length, b), length, c):len(length)
end

function ParParabolaExp(length, p1, p2, shape)
    if (not shape) or shape == 0.5 then
        return ParLinear(length, p1, p2)
    end
    local diff = p2 - p1
    local b = p1 - 2 * diff * (1 - 2 * shape)
    local c = (4 * shape - 1) * diff + 2 * p1 - b
    return ParLinearToExp(ParLerp(0, ParLerp(0, p1, length, b), length, c), p1, p2):len(length)
end

function ParSmoothChange(length, p1, p2)
    local diff = p2 - p1
    local b = p1 - 2 * diff
    local c = 3 * p1 - 2 * b - 3 * diff
    local d = 3 * p1 - b - c
    return ParLerp(0, ParLerp(0, ParLerp(0, p1, length, b), length, c), length, d):len(length)
end

function ParSmoothChangeExp(length, p1, p2)
    local diff = p2 - p1
    local b = p1 - 2 * diff
    local c = 3 * p1 - 2 * b - 3 * diff
    local d = 3 * p1 - b - c
    return ParLinearToExp(ParLerp(0, ParLerp(0, ParLerp(0, p1, length, b), length, c), length, d), p1, p2):len(length)
end

function ParSlide(length, p1, p2)
    if not p2 then
        return p1
    else
        return ParLinearExp(length, p1, p2) .. ParConstMin(p2)
    end
end

function ParSmoothBeginningEnding(length, begin_length, end_length)
    end_length = end_length or begin_length
    return ParLinear(begin_length, 0, 1) .. ParConst(1):len(length - begin_length - end_length) .. ParLinear(end_length, 1, 0)
end

function ParSmoothEnding(length, end_length)
    return ParConst(1):len(length - end_length) .. ParLinear(end_length, 1, 0)
end

function ParSmoothBeginning(length, begin_length)
    return ParLinear(begin_length, 0, 1) .. ParConst(1):len(length - begin_length)
end

--

--Envelopes

function EnvelopeSimple(duration, attack, decay, sustain, release, attack_level)
    attack_level = attack_level or 1
    local seg1 = ParLerp(0, 0, attack, attack_level):len(attack)
    local seg2 = ParLerp(0, attack_level, decay, sustain):len(decay)
    local seg3 = ParConst(sustain):len(math.max(0, duration - attack - sustain))
    local seg4 = ParLerp(0, sustain, release, 0):len(release)
    return ParConcat { seg1, seg2, seg3, seg4 }
end

EnvelopeControl = {}

function envConst(duration)
    return setmetatable({ const = duration }, EnvelopeControl)
end

function envLine(duration)
    return setmetatable({ line = duration }, EnvelopeControl)
end

function envParabola(duration, shape)
    return setmetatable({ line = duration, shape = shape }, EnvelopeControl)
end

function envSmooth(duration)
    return setmetatable({ line = duration, smooth = true }, EnvelopeControl)
end

function envExp(duration)
    return setmetatable({ exp = duration }, EnvelopeControl)
end

function envParabolaExp(duration, shape)
    return setmetatable({ exp = duration, shape = shape }, EnvelopeControl)
end

function envSmoothExp(duration)
    return setmetatable({ exp = duration, smooth = true }, EnvelopeControl)
end

local function process_env_segment(seg, sustain, timescale, start_point)
    if not seg then
        return {}, 0
    end
    local pars = {}
    local point_beats = start_point
    local len_seconds = 0
    local i = 1
    local num
    local control
    for _, elem in ipairs(seg) do
        if verify_num(elem) then
            if control then
                if (not control.const) and num then
                    if control.line then
                        local actual_dur = timescale:dur(control.line, point_beats)
                        if control.smooth then
                            pars[i] = ParSmoothChange(actual_dur, num - sustain, elem - sustain)
                        elseif control.shape then
                            pars[i] = ParParabola(actual_dur, num - sustain, elem - sustain, control.shape)
                        else
                            pars[i] = ParLinear(actual_dur, num - sustain, elem - sustain)
                        end
                        point_beats = point_beats + control.line
                        len_seconds = len_seconds + actual_dur
                    else
                        local actual_dur = timescale:dur(control.exp, point_beats)
                        if control.smooth then
                            pars[i] = ParSmoothChangeExp(actual_dur, num - sustain, elem - sustain)
                        elseif control.shape then
                            pars[i] = ParParabolaExp(actual_dur, num - sustain, elem - sustain, control.shape)
                        else
                            pars[i] = ParLinearExp(actual_dur, num - sustain, elem - sustain)
                        end
                        point_beats = point_beats + control.exp
                        len_seconds = len_seconds + actual_dur
                    end
                    i = i + 1
                elseif control.const then
                    local actual_dur = timescale:dur(control.const, point_beats)
                    pars[i] = ParConst(elem - sustain):len(actual_dur)
                    point_beats = point_beats + control.const
                    len_seconds = len_seconds + actual_dur
                    i = i + 1
                end
            end
            num = elem
        elseif getmetatable(elem) == EnvelopeControl then
            control = elem
        else
            error("Expected array of num or EnvelopeControl")
        end
    end
    if #pars == 0 then
        num = nil
    end
    return pars, num
end

local function process_env_segment_backwards(seg, sustain, timescale, start_point)
    if not seg then
        return {}, 0
    end
    local point_beats = start_point
    local num
    local control
    for _, elem in ipairs(seg) do
        if verify_num(elem) then
            if control then
                if (not control.const) and num then
                    if control.line then
                        point_beats = point_beats - control.line
                    else
                        point_beats = point_beats - control.exp
                    end
                elseif control.const then
                    point_beats = point_beats - control.const
                end
            end
            num = elem
        elseif getmetatable(elem) == EnvelopeControl then
            control = elem
        else
            error("Expected array of num or EnvelopeControl")
        end
    end

    local pars = {}
    local i = 1
    local len_seconds = 0
    num = nil
    control = nil

    for _, elem in ipairs(seg) do
        if verify_num(elem) then
            if control then
                if (not control.const) and num then
                    if control.line then
                        local actual_dur = timescale:dur(control.line, point_beats)
                        if control.smooth then
                            pars[i] = ParSmoothChange(actual_dur, num - sustain, elem - sustain)
                        elseif control.shape then
                            pars[i] = ParParabola(actual_dur, num - sustain, elem - sustain, control.shape)
                        else
                            pars[i] = ParLinear(actual_dur, num - sustain, elem - sustain)
                        end
                        point_beats = point_beats + control.line
                        len_seconds = len_seconds + actual_dur
                    else
                        local actual_dur = timescale:dur(control.exp, point_beats)
                        if control.smooth then
                            pars[i] = ParSmoothChangeExp(actual_dur, num - sustain, elem - sustain)
                        elseif control.shape then
                            pars[i] = ParParabolaExp(actual_dur, num - sustain, elem - sustain, control.shape)
                        else
                            pars[i] = ParLinearExp(actual_dur, num - sustain, elem - sustain)
                        end
                        point_beats = point_beats + control.exp
                        len_seconds = len_seconds + actual_dur
                    end
                    i = i + 1
                elseif control.const then
                    local actual_dur = timescale:dur(control.const, point_beats)
                    pars[i] = ParConst(elem - sustain):len(actual_dur)
                    point_beats = point_beats + control.const
                    len_seconds = len_seconds + actual_dur
                    i = i + 1
                end
            end
            num = elem
        elseif getmetatable(elem) == EnvelopeControl then
            control = elem
        end
    end
    if #pars == 0 then
        num = nil
    end
    return pars, num, len_seconds
end

function Envelope(duration, segments, start_point, timescale)
    start_point = start_point or 0
    timescale = timescale or default_timescale
    local before, last_end, before_len = process_env_segment_backwards(segments.before_begin or segments[1], 0, timescale, start_point)
    local after, after_end = process_env_segment(segments.after_begin or segments[2], 0, timescale, start_point)
    local result
    local segs1 = concat_tables(before, after)
    before_len = before_len or 0
    if #segs1 > 0 then
        if after_end then
            last_end = after_end
        end
        result = ParConcat(unpack(concat_tables(segs1, { ParConstMin(last_end) }))):offset(before_len)
    end
    local last_end_old = last_end or 0
    before, last_end, before_len = process_env_segment_backwards(segments.before_end or segments[3], last_end_old, timescale, start_point)
    after, after_end = process_env_segment(segments.after_end or segments[4], last_end_old, timescale, start_point)
    local segs2 = concat_tables(before, after)
    before_len = before_len or 0
    if #segs2 > 0 then
        if after_end then
            last_end = after_end
        end
        if result then
            result = result + ParConcat(unpack(concat_tables(segs2, { ParConstMin(last_end - last_end_old) }))):offset(before_len - timescale:dur(duration, start_point))
        else
            result = ParConcat(unpack(concat_tables(segs2, { ParConstMin(last_end - last_end_old) }))):offset(before_len - timescale:dur(duration, start_point))
        end
    end
    if not result then
        error("no segments found")
    end
    return result
end

--Buffer wrapper

BufferWrapper = {}
BufferWrapper.__index = BufferWrapper

function BufferWrapper.newMono(other_sample_rate)
    other_sample_rate = other_sample_rate or sample_rate
    return setmetatable({
        sample_rate = other_sample_rate,
        start_point = NaN,
        end_point = NaN,
        current_level = 1,
        level_nodes = {},
        nodes = { {} },
        transform_mode = cpp.MONO_IN_PLACE,
        bufferL = cpp.addNode(cpp.bufferNode(other_sample_rate))
    }, BufferWrapper)
end

function BufferWrapper.newStereo(other_sample_rate)
    other_sample_rate = other_sample_rate or sample_rate
    return setmetatable({
        sample_rate = other_sample_rate,
        start_point = NaN,
        end_point = NaN,
        current_level = 1,
        level_nodes = {},
        nodes = { {} },
        transform_mode = cpp.STEREO_IN_PLACE,
        bufferL = cpp.addNode(cpp.bufferNode(other_sample_rate)),
        bufferR = cpp.addNode(cpp.bufferNode(other_sample_rate))
    }, BufferWrapper)
end

function BufferWrapper:getLength()
    return self.end_point - self.start_point
end

function BufferWrapper:getOffset()
    return self.start_point
end

function BufferWrapper:hasLength()
    return self.start_point == self.start_point
end

function BufferWrapper:registerNode(node)
    --[[level = level or self.current_level
    if level < 1 or level > self.current_level then
        error("level " .. level .. " is not valid")
    end]]
    local t = self.nodes[self.current_level]
    t[#t + 1] = node
    if self.current_level > 1 then
        cpp.addPrerequisite(node, self.level_nodes[self.current_level - 1])
    end
    --[[if level < self.current_level then
        cpp.addPrerequisite(self.level_nodes[level+1], node)
    end]]
end

function BufferWrapper:endLevel()
    local new_level_node
    if #self.nodes[self.current_level] == 0 then
        return
    elseif #self.nodes[self.current_level] == 1 then
        new_level_node = self.nodes[self.current_level][1]
        self.level_nodes[self.current_level] = new_level_node
        self.nodes[self.current_level] = nil
    else
        new_level_node = cpp.dummyNode()
        self.level_nodes[self.current_level] = cpp.addNode(new_level_node)
        for _, node in ipairs(self.nodes[self.current_level]) do
            cpp.addPrerequisite(new_level_node, node)
        end
    end
    cpp.addPrerequisite(new_level_node, self.bufferL)
    if self.bufferR then
        cpp.addPrerequisite(new_level_node, self.bufferR)
    end
    self.current_level = self.current_level + 1
    self.nodes[self.current_level] = {}
end

function BufferWrapper:getLastLevel()
    return self.level_nodes[self.current_node]
end

function BufferWrapper:registerInterval(start_point, length)
    if not (self.start_point < start_point) then
        self.start_point = start_point
    end
    local end_point = start_point + length
    if not (self.end_point > end_point) then
        self.end_point = end_point
    end
end

function BufferWrapper:registerRegularTransform(node, start_point, length)
    cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
    if self.bufferR then
        cpp.addBuffer(node, start_point, self.bufferR, NO_LEN, false)
    end
    self:registerNode(node)
    self:registerInterval(start_point, length)
    cpp.addNode(node)
end

function BufferWrapper:verifyInterval(start_point, length)
    start_point = start_point or self.start_point
    length = length or self.end_point - start_point
    if start_point ~= start_point or length ~= length then
        error("interval was not provided and the buffer has none")
    end
    return start_point, length
end

function BufferWrapper:verifyInterval2(start_point, length)
    start_point = start_point or self.start_point
    length = length or self.end_point - start_point
    return start_point, length, start_point == start_point and length == length
end

--Low level transforms

function BufferWrapper:bufferFromRaw(filename, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    start_point, length = self:verifyInterval(start_point, length, dont_end_level)
    local node = cpp.bufferFromRaw(self.sample_rate, length, self.transform_mode, filename)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:bufferFromWAV(filename, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.bufferFromWAV(self.sample_rate, length, self.transform_mode, filename)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:bufferToRaw(filename, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.bufferToRaw(self.sample_rate, length, self.transform_mode, filename)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:bufferToWAV(filename, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.bufferToWAV(self.sample_rate, length, self.transform_mode, filename)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:copyBuffer(destination, dest_start_point, multiplier, start_point, length, dont_end_level)
    if getmetatable(destination) ~= BufferWrapper then
        error("destination is not a BufferWrapper")
    end
    if self.sample_rate ~= destination.sample_rate then
        error("sample rates don't match")
    end
    multiplier = verify_par(multiplier) or ParConst(1, self.sample_rate)
    local exists
    start_point, length, exists = self:verifyInterval2(start_point, length)
    if not exists then
        return
    end
    if not dont_end_level then
        self:endLevel()
        destination:endLevel()
    end
    dest_start_point = dest_start_point + start_point
    local node
    if self.bufferR then
        if destination.bufferR then
            node = cpp.copyBuffer(self.sample_rate, length, cpp.STEREO_TO_STEREO)
            cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
            cpp.addBuffer(node, start_point, self.bufferR, NO_LEN, false)
            cpp.addBuffer(node, dest_start_point, destination.bufferL, NO_LEN, false)
            cpp.addBuffer(node, dest_start_point, destination.bufferR, NO_LEN, false)
        else
            node = cpp.copyBuffer(self.sample_rate, length, cpp.STEREO_TO_MONO)
            cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
            cpp.addBuffer(node, start_point, self.bufferR, NO_LEN, false)
            cpp.addBuffer(node, dest_start_point, destination.bufferL, NO_LEN, false)
        end
    else
        if destination.bufferR then
            node = cpp.copyBuffer(self.sample_rate, length, cpp.MONO_TO_STEREO)
            cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
            cpp.addBuffer(node, dest_start_point, destination.bufferL, NO_LEN, false)
            cpp.addBuffer(node, dest_start_point, destination.bufferR, NO_LEN, false)
        else
            node = cpp.copyBuffer(self.sample_rate, length, cpp.MONO_TO_MONO)
            cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
            cpp.addBuffer(node, dest_start_point, destination.bufferL, NO_LEN, false)
        end
    end
    cpp.addArgument(node, multiplier.start or 0, multiplier.node, false, NO_LEN, false)
    self:registerNode(node)
    self:registerInterval(start_point, length)
    destination:registerNode(node)
    destination:registerInterval(dest_start_point, length)
    if not dont_end_level then
        self:endLevel()
        destination:endLevel()
    end
    cpp.addNode(node)
end

function BufferWrapper:normalize(allow_volume_up, provide_feedback, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    allow_volume_up = allow_volume_up or false
    provide_feedback = provide_feedback or false
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.normalize(self.sample_rate, length, self.transform_mode, allow_volume_up, provide_feedback)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:softClipCompressor(linear_threshold, linear_amp, tanh_wet_dry, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    linear_threshold = verify_par(linear_threshold)
    linear_amp = verify_par(linear_amp)
    tanh_wet_dry = verify_par(tanh_wet_dry)
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.softClipCompressor(self.sample_rate, length, self.transform_mode)
    cpp.addArgument(node, linear_threshold.start or 0, linear_threshold.node, false, NO_LEN, false)
    cpp.addArgument(node, linear_amp.start or 0, linear_amp.node, false, NO_LEN, false)
    cpp.addArgument(node, tanh_wet_dry.start or 0, tanh_wet_dry.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:butterworthBandPass(iterations, freq1, freq2, resonance, wet_dry, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq1 = verify_par(freq1)
    freq2 = verify_par(freq2)
    resonance = verify_par(resonance) or ParConst(0, freq1.sample_rate)
    wet_dry = verify_par(wet_dry) or ParConst(1, freq1.sample_rate)
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.butterworthBandPass(self.sample_rate, length, self.transform_mode, iterations)
    cpp.addArgument(node, freq1.start or 0, freq1.node, false, NO_LEN, false)
    cpp.addArgument(node, freq2.start or 0, freq2.node, false, NO_LEN, false)
    cpp.addArgument(node, resonance.start or 0, resonance.node, false, NO_LEN, false)
    cpp.addArgument(node, wet_dry.start or 0, wet_dry.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:butterworthBandStop(iterations, freq1, freq2, resonance, wet_dry, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq1 = verify_par(freq1)
    freq2 = verify_par(freq2)
    resonance = verify_par(resonance) or ParConst(0, freq1.sample_rate)
    wet_dry = verify_par(wet_dry) or ParConst(1, freq1.sample_rate)
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.butterworthBandStop(self.sample_rate, length, self.transform_mode, iterations)
    cpp.addArgument(node, freq1.start or 0, freq1.node, false, NO_LEN, false)
    cpp.addArgument(node, freq2.start or 0, freq2.node, false, NO_LEN, false)
    cpp.addArgument(node, resonance.start or 0, resonance.node, false, NO_LEN, false)
    cpp.addArgument(node, wet_dry.start or 0, wet_dry.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:butterworthHighPass(iterations, freq, resonance, wet_dry, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq = verify_par(freq)
    resonance = verify_par(resonance) or ParConst(0, freq.sample_rate)
    wet_dry = verify_par(wet_dry) or ParConst(1, freq.sample_rate)
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.butterworthHighPass(self.sample_rate, length, self.transform_mode, iterations)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, false)
    cpp.addArgument(node, resonance.start or 0, resonance.node, false, NO_LEN, false)
    cpp.addArgument(node, wet_dry.start or 0, wet_dry.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:butterworthLowPass(iterations, freq, resonance, wet_dry, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq = verify_par(freq)
    resonance = verify_par(resonance) or ParConst(0, freq.sample_rate)
    wet_dry = verify_par(wet_dry) or ParConst(1, freq.sample_rate)
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.butterworthLowPass(self.sample_rate, length, self.transform_mode, iterations)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, false)
    cpp.addArgument(node, resonance.start or 0, resonance.node, false, NO_LEN, false)
    cpp.addArgument(node, wet_dry.start or 0, wet_dry.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:flangerFeedback(freq, wet_dry, start_point, length, buffer_length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq = verify_par(freq)
    wet_dry = verify_par(wet_dry)
    start_point, length = self:verifyInterval(start_point, length)
    buffer_length = buffer_length or self.sample_rate
    local node = cpp.flangerFeedback(self.sample_rate, length, self.transform_mode, buffer_length)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, false)
    cpp.addArgument(node, wet_dry.start or 0, wet_dry.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:flangerFeedforward(freq, wet_dry, start_point, length, buffer_length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq = verify_par(freq)
    wet_dry = verify_par(wet_dry)
    start_point, length = self:verifyInterval(start_point, length)
    buffer_length = buffer_length or self.sample_rate
    local node = cpp.flangerFeedforward(self.sample_rate, length, self.transform_mode, buffer_length)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, false)
    cpp.addArgument(node, wet_dry.start or 0, wet_dry.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:flangerSelfOscillation(freq, oscil_length, correction, start_point, length, buffer_length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq = verify_par(freq)
    oscil_length = verify_par(oscil_length)
    correction = verify_par(correction)
    start_point, length = self:verifyInterval(start_point, length)
    buffer_length = buffer_length or self.sample_rate
    local node = cpp.flangerSelfOscillation(self.sample_rate, length, self.transform_mode, buffer_length)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, false)
    cpp.addArgument(node, oscil_length.start or 0, oscil_length.node, false, NO_LEN, false)
    cpp.addArgument(node, correction.start or 0, correction.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:phaser(stages, coefficient, feedback, wet_dry, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    coefficient = verify_par(coefficient)
    feedback = verify_par(feedback)
    wet_dry = verify_par(wet_dry)
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.phaserLegacy(self.sample_rate, length, self.transform_mode, stages)
    cpp.addArgument(node, coefficient.start or 0, coefficient.node, false, NO_LEN, false)
    cpp.addArgument(node, feedback.start or 0, feedback.node, false, NO_LEN, false)
    cpp.addArgument(node, wet_dry.start or 0, wet_dry.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:moogHighPass(saturation, freq, resonance, wet_dry, inner_multiplier, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq = verify_par(freq)
    resonance = verify_par(resonance) or ParConst(0, freq.sample_rate)
    wet_dry = verify_par(wet_dry) or ParConst(1, freq.sample_rate)
    inner_multiplier = verify_par(inner_multiplier) or ParConst(1, freq.sample_rate)
    start_point = start_point or self.start_point
    length = length or self.end_point - start_point
    local node = cpp.moogHighPass(self.sample_rate, length, self.transform_mode, saturation)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, false)
    cpp.addArgument(node, resonance.start or 0, resonance.node, false, NO_LEN, false)
    cpp.addArgument(node, wet_dry.start or 0, wet_dry.node, false, NO_LEN, false)
    cpp.addArgument(node, inner_multiplier.start or 0, inner_multiplier.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:moogLowPass(saturation, freq, resonance, wet_dry, inner_multiplier, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq = verify_par(freq)
    resonance = verify_par(resonance) or ParConst(0, freq.sample_rate)
    wet_dry = verify_par(wet_dry) or ParConst(1, freq.sample_rate)
    inner_multiplier = verify_par(inner_multiplier) or ParConst(1, freq.sample_rate)
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.moogLowPass(self.sample_rate, length, self.transform_mode, saturation)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, false)
    cpp.addArgument(node, resonance.start or 0, resonance.node, false, NO_LEN, false)
    cpp.addArgument(node, wet_dry.start or 0, wet_dry.node, false, NO_LEN, false)
    cpp.addArgument(node, inner_multiplier.start or 0, inner_multiplier.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:changeVolume(multiplier, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    multiplier = verify_par(multiplier)
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.changeVolume(self.sample_rate, length, self.transform_mode)
    cpp.addArgument(node, multiplier.start or 0, multiplier.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:expShape(exponent, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    exponent = verify_par(exponent)
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.expShape(self.sample_rate, length, self.transform_mode)
    cpp.addArgument(node, exponent.start or 0, exponent.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:limiter(limit, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    limit = verify_par(limit) or ParConst(1)
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.limiter(self.sample_rate, length, self.transform_mode)
    cpp.addArgument(node, limit.start or 0, limit.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:ringMod(length, other, other_start_point, start_point, dont_end_level)
    if getmetatable(other) ~= BufferWrapper then
        error("destination is not a BufferWrapper")
    end
    if self.sample_rate ~= other.sample_rate then
        error("sample rates don't match")
    end
    start_point, length = self:verifyInterval(start_point, length)
    if not dont_end_level then
        self:endLevel()
        other:endLevel()
    end
    local node
    if self.bufferR then
        if other.bufferR then
            node = cpp.ringMod(self.sample_rate, length, cpp.STEREO_TO_STEREO)
            cpp.addBuffer(node, other_start_point, other.bufferL, NO_LEN, false)
            cpp.addBuffer(node, other_start_point, other.bufferR, NO_LEN, false)
            cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
            cpp.addBuffer(node, start_point, self.bufferR, NO_LEN, false)
        else
            error("this is stereo but other is mono")
        end
    else
        if other.bufferR then
            error("this is mono but other is stereo")
        else
            node = cpp.ringMod(self.sample_rate, length, cpp.MONO_TO_MONO)
            cpp.addBuffer(node, other_start_point, other.bufferL, NO_LEN, false)
            cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
        end
    end
    self:registerNode(node)
    self:registerInterval(start_point, length)
    other:registerNode(node)
    other:registerInterval(other_start_point, length)
    if not dont_end_level then
        self:endLevel()
        other:endLevel()
    end
    cpp.addNode(node)
end

function BufferWrapper:saturationTanh(start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.saturationTanh(self.sample_rate, length, self.transform_mode)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:simpleReverb(room_size, damping, wet_dry, space_width, rel_length, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    room_size = verify_par(room_size)
    damping = verify_par(damping)
    wet_dry = verify_par(wet_dry)
    space_width = verify_par(space_width)
    rel_length = rel_length or 1
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.simpleReverb(self.sample_rate, length, self.transform_mode, rel_length)
    cpp.addArgument(node, room_size.start or 0, room_size.node, false, NO_LEN, false)
    cpp.addArgument(node, damping.start or 0, damping.node, false, NO_LEN, false)
    cpp.addArgument(node, wet_dry.start or 0, wet_dry.node, false, NO_LEN, false)
    cpp.addArgument(node, space_width.start or 0, space_width.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:simpleReverseReverb(room_size, damping, wet_dry, space_width, rel_length, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    room_size = verify_par(room_size)
    damping = verify_par(damping)
    wet_dry = verify_par(wet_dry)
    space_width = verify_par(space_width)
    rel_length = rel_length or 1
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.simpleReverseReverb(self.sample_rate, length, self.transform_mode, rel_length)
    cpp.addArgument(node, room_size.start or 0, room_size.node, false, NO_LEN, false)
    cpp.addArgument(node, damping.start or 0, damping.node, false, NO_LEN, false)
    cpp.addArgument(node, wet_dry.start or 0, wet_dry.node, false, NO_LEN, false)
    cpp.addArgument(node, space_width.start or 0, space_width.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:constantSignal(value, start_point, length, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    value = verify_par(value)
    start_point, length = self:verifyInterval(start_point, length)
    local node = cpp.constantSignal(self.sample_rate, length, self.transform_mode)
    cpp.addArgument(node, value.start or 0, value.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:polysynthSwtPulse(freq_swt, freq_pulse, freq_mod_swt, freq_mod_pulse, swt_pulse_bias, pulse_shape, start_point, length, start_phase_swt, start_phase_pulse, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq_swt = verify_par(freq_swt)
    freq_pulse = verify_par(freq_pulse)
    freq_mod_swt = verify_par(freq_mod_swt)
    freq_mod_pulse = verify_par(freq_mod_pulse)
    swt_pulse_bias = verify_par(swt_pulse_bias)
    pulse_shape = verify_par(pulse_shape)
    start_point, length = self:verifyInterval(start_point, length)
    start_phase_swt = start_phase_swt or 0
    start_phase_pulse = start_phase_pulse or 0
    local node = cpp.polysynthSwtPulse(self.sample_rate, length, self.transform_mode, start_phase_swt, start_phase_pulse)
    cpp.addArgument(node, freq_swt.start or 0, freq_swt.node, false, NO_LEN, false)
    cpp.addArgument(node, freq_pulse.start or 0, freq_pulse.node, false, NO_LEN, false)
    cpp.addArgument(node, freq_mod_swt.start or 0, freq_mod_swt.node, false, NO_LEN, false)
    cpp.addArgument(node, freq_mod_pulse.start or 0, freq_mod_pulse.node, false, NO_LEN, false)
    cpp.addArgument(node, swt_pulse_bias.start or 0, swt_pulse_bias.node, false, NO_LEN, false)
    cpp.addArgument(node, pulse_shape.start or 0, pulse_shape.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:superwaveOscillator(freq, mix, detune, type, start_point, length, start_phase, seed, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq = verify_par(freq)
    mix = verify_par(mix)
    detune = verify_par(detune)
    type = type or cpp.SIN
    start_point, length = self:verifyInterval(start_point, length)
    start_phase = start_phase or 0
    seed = seed or math.random(0, 2147483647)
    local node = cpp.superwaveOscillator(self.sample_rate, length, self.transform_mode, type, start_phase, seed)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, false)
    cpp.addArgument(node, mix.start or 0, mix.node, false, NO_LEN, false)
    cpp.addArgument(node, detune.start or 0, detune.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:superwaveOscillatorShaped(freq, mix, detune, shape, type, start_point, length, start_phase, seed, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq = verify_par(freq)
    mix = verify_par(mix)
    detune = verify_par(detune)
    shape = verify_par(shape)
    type = type or cpp.PULSE
    start_point, length = self:verifyInterval(start_point, length)
    start_phase = start_phase or 0
    seed = seed or math.random(0, 2147483647)
    local node = cpp.superwaveOscillatorShaped(self.sample_rate, length, self.transform_mode, type, start_phase, seed)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, false)
    cpp.addArgument(node, mix.start or 0, mix.node, false, NO_LEN, false)
    cpp.addArgument(node, detune.start or 0, detune.node, false, NO_LEN, false)
    cpp.addArgument(node, shape.start or 0, shape.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:waveOscillator(freq, type, start_point, length, start_phase, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq = verify_par(freq)
    type = type or cpp.SIN
    start_point, length = self:verifyInterval(start_point, length)
    start_phase = start_phase or 0
    local node = cpp.waveOscillator(self.sample_rate, length, self.transform_mode, type, start_phase)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:waveOscillatorShaped(freq, shape, type, start_point, length, start_phase, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    freq = verify_par(freq)
    shape = verify_par(shape)
    type = type or cpp.PULSE
    start_point, length = self:verifyInterval(start_point, length)
    start_phase = start_phase or 0
    local node = cpp.waveOscillatorShaped(self.sample_rate, length, self.transform_mode, type, start_phase)
    cpp.addArgument(node, freq.start or 0, freq.node, false, NO_LEN, false)
    cpp.addArgument(node, shape.start or 0, shape.node, false, NO_LEN, false)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:whiteNoise(start_point, length, seed, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    start_point, length = self:verifyInterval(start_point, length)
    seed = seed or math.random(0, 2147483647)
    local node = cpp.whiteNoise(self.sample_rate, length, self.transform_mode, seed)
    self:registerRegularTransform(node, start_point, length)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:improvedStereo(pan, start_point, length, destination, dest_start_point, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    pan = verify_par(pan)
    start_point, length = self:verifyInterval(start_point, length)
    local node
    if self.bufferR then
        if destination then
            dest_start_point = dest_start_point + start_point
            if destination.bufferR then
                node = cpp.improvedStereo(self.sample_rate, length, cpp.STEREO_TO_STEREO)
                cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
                cpp.addBuffer(node, start_point, self.bufferR, NO_LEN, false)
                cpp.addBuffer(node, dest_start_point, destination.bufferL, NO_LEN, false)
                cpp.addBuffer(node, dest_start_point, destination.bufferR, NO_LEN, false)
            else
                error("cannot apply ImprovedStereo STEREO_TO_MONO")
            end
        else
            node = cpp.improvedStereo(self.sample_rate, length, cpp.STEREO_IN_PLACE)
            cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
            cpp.addBuffer(node, start_point, self.bufferR, NO_LEN, false)
        end
    else
        if destination then
            dest_start_point = dest_start_point + start_point
            if destination.bufferR then
                node = cpp.improvedStereo(self.sample_rate, length, cpp.MONO_TO_STEREO)
                cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
                cpp.addBuffer(node, dest_start_point, destination.bufferL, NO_LEN, false)
                cpp.addBuffer(node, dest_start_point, destination.bufferR, NO_LEN, false)
            else
                error("cannot apply ImprovedStereo MONO_TO_MONO")
            end
        else
            error("cannot apply ImprovedStereo MONO_IN_PLACE")
        end
    end
    cpp.addArgument(node, pan.start or 0, pan.node, false, NO_LEN, false)
    self:registerNode(node)
    self:registerInterval(start_point, length)
    if destination then
        destination:registerNode(node)
        destination:registerInterval(dest_start_point, length)
    end
    cpp.addNode(node)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:legacyStereo(pan, start_point, length, destination, dest_start_point, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    pan = verify_par(pan)
    start_point, length = self:verifyInterval(start_point, length)
    dest_start_point = dest_start_point + start_point
    local node
    if self.bufferR then
        if destination then
            if destination.bufferR then
                node = cpp.legacyStereo(self.sample_rate, length, cpp.STEREO_TO_STEREO)
                cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
                cpp.addBuffer(node, start_point, self.bufferR, NO_LEN, false)
                cpp.addBuffer(node, dest_start_point, destination.bufferL, NO_LEN, false)
                cpp.addBuffer(node, dest_start_point, destination.bufferR, NO_LEN, false)
            else
                error("cannot apply LegacyStereo STEREO_TO_MONO")
            end
        else
            node = cpp.legacyStereo(self.sample_rate, length, cpp.STEREO_IN_PLACE)
            cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
            cpp.addBuffer(node, start_point, self.bufferR, NO_LEN, false)
        end
    else
        if destination then
            if destination.bufferR then
                node = cpp.legacyStereo(self.sample_rate, length, cpp.MONO_TO_STEREO)
                cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
                cpp.addBuffer(node, dest_start_point, destination.bufferL, NO_LEN, false)
                cpp.addBuffer(node, dest_start_point, destination.bufferR, NO_LEN, false)
            else
                error("cannot apply LegacyStereo MONO_TO_MONO")
            end
        else
            error("cannot apply LegacyStereo MONO_IN_PLACE")
        end
    end
    cpp.addArgument(node, pan.start or 0, pan.node, false, NO_LEN, false)
    self:registerNode(node)
    self:registerInterval(start_point, length)
    destination:registerNode(node)
    destination:registerInterval(dest_start_point, length)
    cpp.addNode(node)
    if not dont_end_level then
        self:endLevel()
    end
end

function BufferWrapper:simpleStereo(pan, start_point, length, destination, dest_start_point, dont_end_level)
    if not dont_end_level then
        self:endLevel()
    end
    pan = verify_par(pan)
    start_point, length = self:verifyInterval(start_point, length)
    dest_start_point = dest_start_point + start_point
    local node
    if self.bufferR then
        if destination then
            if destination.bufferR then
                node = cpp.simpleStereo(self.sample_rate, length, cpp.STEREO_TO_STEREO)
                cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
                cpp.addBuffer(node, start_point, self.bufferR, NO_LEN, false)
                cpp.addBuffer(node, dest_start_point, destination.bufferL, NO_LEN, false)
                cpp.addBuffer(node, dest_start_point, destination.bufferR, NO_LEN, false)
            else
                error("cannot apply SimpleStereo STEREO_TO_MONO")
            end
        else
            node = cpp.simpleStereo(self.sample_rate, length, cpp.STEREO_IN_PLACE)
            cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
            cpp.addBuffer(node, start_point, self.bufferR, NO_LEN, false)
        end
    else
        if destination then
            if destination.bufferR then
                node = cpp.simpleStereo(self.sample_rate, length, cpp.MONO_TO_STEREO)
                cpp.addBuffer(node, start_point, self.bufferL, NO_LEN, false)
                cpp.addBuffer(node, dest_start_point, destination.bufferL, NO_LEN, false)
                cpp.addBuffer(node, dest_start_point, destination.bufferR, NO_LEN, false)
            else
                error("cannot apply SimpleStereo MONO_TO_MONO")
            end
        else
            error("cannot apply SimpleStereo MONO_IN_PLACE")
        end
    end
    cpp.addArgument(node, pan.start or 0, pan.node, false, NO_LEN, false)
    self:registerNode(node)
    self:registerInterval(start_point, length)
    destination:registerNode(node)
    destination:registerInterval(dest_start_point, length)
    cpp.addNode(node)
    if not dont_end_level then
        self:endLevel()
    end
end

--Variables

QuickFunction = {}
QuickFunction.__index = QuickFunction

local function ret_QF(p, argument)
    if verify_num(p) then
        return p
    else
        return p:ret(argument)
    end
end

local function par_QF(p)
    if verify_num(p) then
        return ParConst(p)
    else
        return p:par()
    end
end

local function Var_ret(self, argument)
    return argument
end

local function Var_par(self)
    return ParLerp(0, 0, 1, 1)
end

function Var()
    return setmetatable({
        ret = Var_ret,
        par = Var_par
    }, QuickFunction)
end

local function SetStart_ret(self, argument)
    return ret_QF(self.p1, argument - self.start)
end

local function SetStart_par(self)
    return par_QF(self.p1):offset(-self.start)
end

function SetStart(p1, start)
    return setmetatable({
        p1 = p1,
        start = start,
        ret = SetStart_ret,
        par = SetStart_par
    }, QuickFunction)
end

local function Add_ret(self, argument)
    return ret_QF(self.p1, argument) + ret_QF(self.p2, argument)
end

local function Add_par(self)
    return ParAdd(par_QF(self.p1), par_QF(self.p2))
end

function Add(p1, p2)
    return setmetatable({
        p1 = p1,
        p2 = p2,
        ret = Add_ret,
        par = Add_par
    }, QuickFunction)
end

local function Concat_ret(self, argument)
    if argument <= 0 then
        return ret_QF(self.args[1][1], 0)
    end
    for _, arg in ipairs(self.args) do
        if argument < arg[2] then
            return ret_QF(arg[1], argument)
        else
            argument = argument - arg[2]
        end
    end
    local last_arg = self.args[#self.args]
    return ret_QF(last_arg[1], last_arg[2])
end

local function Concat_par(self)
    local pars = {}
    for i, arg in ipairs(self.args) do
        pars[i] = par_QF(arg[1]):len(arg[2])
    end
    return ParConcat(unpack(pars))
end

function Concat(...)
    local i = 1
    local args = {}
    local temp
    for _, val in ipairs { ... } do
        if temp then
            if verify_num(val) then
                args[i] = { temp, val }
                i = i + 1
                temp = nil
            else
                if temp.len then
                    args[i] = { temp, temp.len }
                    i = i + 1
                else
                    error("expected: expr [, len], expr [, len]... ; only expressions without length require explicit length")
                end
                if getmetatable(val) == QuickFunction then
                    temp = val
                elseif verify_num(val) then
                    temp = Const(val)
                else
                    error("expected: expr [, len], expr [, len]... ; only expressions without length require explicit length")
                end
            end
        else
            if getmetatable(val) == QuickFunction then
                temp = val
            elseif verify_num(val) then
                temp = Const(val)
            else
                error("expected: expr [, len], expr [, len]... ; only expressions without length require explicit length")
            end
        end
    end
    if temp then
        if temp.len then
            args[i] = { temp, temp.len }
        else
            error("expected: expr [, len], expr [, len]... ; only expressions without length require explicit length")
        end
    end
    if #args == 0 then
        error("no args found")
    end
    return setmetatable({
        args = args,
        ret = Concat_ret,
        par = Concat_par
    }, QuickFunction)
end

local function Const_ret(self, argument)
    return self.v
end

local function Const_par(self)
    return ParConst(self.v)
end

function Const(v)
    return setmetatable({
        v = v,
        ret = Const_ret,
        par = Const_par
    }, QuickFunction)
end

local function ConstAt_ret(self, argument)
    return ret_QF(self.p1, argument + self.offset)
end

local function ConstAt_par(self)
    return ParConstAt(par_QF(self.p1):offset(self.offset))
end

function ConstAt(p1, offset)
    return setmetatable({
        p1 = p1,
        offset = offset,
        ret = Const_ret,
        par = Const_par
    }, QuickFunction)
end

local function Div_ret(self, argument)
    return ret_QF(self.p1, argument) / ret_QF(self.p2, argument)
end

local function Div_par(self)
    return ParDiv(par_QF(self.p1), par_QF(self.p2))
end

function Div(p1, p2)
    return setmetatable({
        p1 = p1,
        p2 = p2,
        ret = Div_ret,
        par = Div_par
    }, QuickFunction)
end

local function Exp_ret(self, argument)
    return ret_QF(self.p1, argument) ^ ret_QF(self.p2, argument)
end

local function Exp_par(self)
    return ParExp(par_QF(self.p1), par_QF(self.p2))
end

function Exp(p1, p2)
    return setmetatable({
        p1 = p1,
        p2 = p2,
        ret = Exp_ret,
        par = Exp_par
    }, QuickFunction)
end

local function Lerp_ret(self, argument)
    local weight = (argument - self.t1) / (self.t2 - self.t1)
    return (1 - weight) * ret_QF(self.p1, argument) + weight * ret_QF(self.p2, argument)
end

local function Lerp_par(self)
    return ParLerp(self.t1, par_QF(self.p1), self.t2, par_QF(self.p2))
end

function Lerp(t1, p1, t2, p2)
    if t1 == t2 then
        error("t1 should be different from t2")
    end
    return setmetatable({
        t1 = t1,
        p1 = p1,
        t2 = t2,
        p2 = p2,
        ret = Lerp_ret,
        par = Lerp_par
    }, QuickFunction)
end

local function Linear_ret(self, argument)
    local weight = argument / self.len
    return (1 - weight) * ret_QF(self.p1, argument) + weight * ret_QF(self.p2, argument)
end

local function Linear_par(self)
    return ParLerp(0, par_QF(self.p1), self.len, par_QF(self.p2))
end

function Linear(len, p1, p2)
    return setmetatable({
        len = len,
        p1 = p1,
        p2 = p2,
        ret = Linear_ret,
        par = Linear_par
    }, QuickFunction)
end

local function LinearExp_ret(self, argument)
    return lerp_exp(par_QF(self.p1), par_QF(self.p2), argument / self.len)
end

local function LinearExp_par(self)
    local p1 = ret_QF(self.p1, argument)
    local p2 = ret_QF(self.p2, argument)
    return ParLinearToExp(ParLerp(0, p1, self.len, p2), p1, p2)
end

function LinearExp(len, p1, p2)
    return setmetatable({
        len = len,
        p1 = p1,
        p2 = p2,
        ret = LinearExp_ret,
        par = LinearExp_par,
    }, QuickFunction)
end

local function LinearToExp_ret(self, argument)
    return map_lin_exp(self.v1, self.v2, ret_QF(self.p1, argument))
end

local function LinearToExp_par(self)
    return ParLinearToExp(par_QF(self.p1), self.v1, self.v2)
end

function LinearToExp(p1, v1, v2)
    return setmetatable({
        p1 = p1,
        v1 = v1,
        v2 = v2,
        ret = LinearToExp_ret,
        par = LinearToExp_par,
    }, QuickFunction)
end

local function Limit_ret(self, argument)
    local value = ret_QF(self.p1, argument)
    if value > 1 then
        return 1
    elseif value < -1 then
        return -1
    else
        return value
    end
end

local function Limit_par(self)
    return ParLimit(self.p1)
end

function Limit(p1)
    return setmetatable({
        p1 = p1,
        ret = Limit_ret,
        par = Limit_par
    }, QuickFunction)
end

local function Mul_ret(self, argument)
    return ret_QF(self.p1, argument) * ret_QF(self.p2, argument)
end

local function Mul_par(self)
    return ParMul(par_QF(self.p1), par_QF(self.p2))
end

function Mul(p1, p2)
    return setmetatable({
        p1 = p1,
        p2 = p2,
        ret = Mul_ret,
        par = Mul_par
    }, QuickFunction)
end

local function Oscillator_ret(self, argument)
    error("use pars instead")
end

local function Oscillator_par(self)
    return ParOscillator(par_QF(self.freq), par_QF(self.base), par_QF(self.amp), self.type, self.start_phase)
end

function Oscillator(freq, base, amp, type, start_phase)
    return setmetatable({
        freq = freq,
        base = base,
        amp = amp,
        type = type,
        start_phase = start_phase,
        ret = Oscillator_ret,
        par = Oscillator_par
    }, QuickFunction)
end

local function OscillatorShaped_ret(self, argument)
    error("use pars instead")
end

local function OscillatorShaped_par(self)
    return ParOscillatorShaped(par_QF(self.freq), par_QF(self.base), par_QF(self.amp), par_QF(self.shape), self.type, self.start_phase)
end

function OscillatorShaped(freq, base, amp, shape, type, start_phase)
    return setmetatable({
        freq = freq,
        base = base,
        amp = amp,
        shape = shape,
        type = type,
        start_phase = start_phase,
        ret = OscillatorShaped_ret,
        par = OscillatorShaped_par
    }, QuickFunction)
end

local function Parabola_ret(self, argument)
    local diff = self.v2 - self.v1
    local a = (2.0 * diff * (1.0 - 2.0 * self.shape)) / (self.len * self.len)
    local b = ((4.0 * self.shape - 1.0) * diff) / self.len
    return a * argument * argument + b * argument + self.v1
end

local function Parabola_par(self)
    return ParParabola(self.len, self.p1, self.p2, self.shape)
end

function Parabola(len, v1, v2, shape)
    return setmetatable({
        len = len,
        v1 = v1,
        v2 = v2,
        shape = shape,
        ret = Parabola_ret,
        par = Parabola_par
    }, QuickFunction)
end

local function ParabolaExp_ret(self, argument)
    local a = (2.0 * (1.0 - 2.0 * self.shape)) / (self.len * self.len)
    local b = (4.0 * self.shape - 1.0) / self.len
    return lerp_exp(self.v1, self.v2, a * argument * argument + b * argument)
end

local function ParabolaExp_par(self)
    return ParParabolaExp(self.len, self.p1, self.p2, self.shape)
end

function ParabolaExp(len, v1, v2, shape)
    return setmetatable({
        len = len,
        v1 = v1,
        v2 = v2,
        shape = shape,
        ret = ParabolaExp_ret,
        par = ParabolaExp_par
    }, QuickFunction)
end

local function Sub_ret(self, argument)
    return ret_QF(self.p1, argument) - ret_QF(self.p2, argument)
end

local function Sub_par(self)
    return ParSub(par_QF(self.p1), par_QF(self.p2))
end

function Sub(p1, p2)
    return setmetatable({
        p1 = p1,
        p2 = p2,
        ret = Sub_ret,
        par = Sub_par
    }, QuickFunction)
end

local function Tanh_ret(self, argument)
    local value = ret_QF(self.p1, argument)
    return math.tanh(value)
end

local function Tanh_par(self)
    return ParTanh(self.p1)
end

function Tanh(p1)
    return setmetatable({
        p1 = p1,
        ret = Tanh_ret,
        par = Tanh_par
    }, QuickFunction)
end

local function Vibrato_ret(self, argument)
    error("use pars instead")
end

local function Vibrato_par(self)
    return ParVibrato(par_QF(self.freq), par_QF(self.base), par_QF(self.exp_amp), self.type, self.start_phase)
end

function Vibrato(freq, base, exp_amp, type, start_phase)
    return setmetatable({
        freq = freq,
        base = base,
        exp_amp = exp_amp,
        type = type,
        start_phase = start_phase,
        ret = Vibrato_ret,
        par = Vibrato_par
    }, QuickFunction)
end

local function VibratoShaped_ret(self, argument)
    error("use pars instead")
end

local function VibratoShaped_par(self)
    return ParVibrato(par_QF(self.freq), par_QF(self.base), par_QF(self.exp_amp), par_QF(self.shape), self.type, self.start_phase)
end

function VibratoShaped(freq, base, exp_amp, shape, type, start_phase)
    return setmetatable({
        freq = freq,
        base = base,
        exp_amp = exp_amp,
        shape = shape,
        type = type,
        start_phase = start_phase,
        ret = VibratoShaped_ret,
        par = VibratoShaped_par
    }, QuickFunction)
end

function QuickFunction.__add(p1, p2)
    return Add(p1, p2)
end

function QuickFunction.__sub(p1, p2)
    return Sub(p1, p2)
end

function QuickFunction.__mul(p1, p2)
    return Mul(p1, p2)
end

function QuickFunction.__div(p1, p2)
    return Div(p1, p2)
end

function QuickFunction.__exp(p1, p2)
    return Exp(p1, p2)
end

function QuickFunction:offset(start)
    return SetStart(self, start)
end

--Rhythm

ConstantTimescale = {}
ConstantTimescale.__index = ConstantTimescale

function ConstantTimescale.new(start_point, bpm)
    return setmetatable({ start_point = start_point, beat_length = bpm_length(bpm) }, ConstantTimescale)
end

function ConstantTimescale:time(beat)
    return self.start_point + self.beat_length * beat
end

function ConstantTimescale:dur(beat)
    return self.beat_length * beat
end

VariableTimescale = {}
VariableTimescale.__index = VariableTimescale

function VariableTimescale.prepareSegment(seg, current_end_point)
    local p1 = bpm_length(seg.bpm1)
    local p2 = bpm_length(seg.bpm2)
    if (math.abs(p2 - p1) > 1e-8) then
        local v1 = 1 / p1
        local v2 = 1 / p2
        local le_mul = math.log(v2 / v1) / (v2 - v1)
        local le_add = math.log(v1) - v1 * le_mul
        local length = seg.beats * le_mul
        local new_end_point = current_end_point + length
        local b_c1 = (v2 - v1) * le_mul / length
        return {
            p1 = p1,
            p2 = p2,
            beats = seg.beats,
            start_point = current_end_point,
            end_point = new_end_point,
            constant = false,
            b_c1 = b_c1,
            b_c2 = math.exp(le_mul * v1 + le_add) / b_c1
        }, current_end_point + length
    else
        local length = seg.beats * p1
        local new_end_point = current_end_point + length
        return {
            p1 = p1,
            p2 = p2,
            beats = seg.beats,
            start_point = current_end_point,
            end_point = new_end_point,
            constant = true
        }, new_end_point
    end
end

function VariableTimescale.new(start_point, segs_data)
    local obj = {}
    local segments = {}
    local current_end_point = start_point
    if not segs_data[1] then
        error("Expected an array of segments")
    end
    for i, seg in ipairs(segs_data) do
        if verify_unum(seg.bpm1) and verify_unum(seg.bpm2) and verify_unum(seg.beats) then
            segments[i], current_end_point = VariableTimescale.prepareSegment(seg, current_end_point)
        else
            error("segment should have non-negative bpm1, bpm2 and beats")
        end
    end
    return setmetatable({
        start_point = start_point,
        end_point = current_end_point,
        start_tempo = segments[1].p1,
        end_tempo = segments[#segments].p2,
        segments = segments
    }, VariableTimescale)
end

local function getTimePointInSegment(segment, beat)
    if segment.constant then
        return beat * segment.p1
    else
        return math.log(beat / segment.b_c2 + 1) / segment.b_c1
    end
end

function VariableTimescale:time(beat)
    if beat < 0 then
        return self.start_point + beat * self.start_tempo
    end
    for i, seg in ipairs(self.segments) do
        if beat < seg.beats then
            return seg.start_point + getTimePointInSegment(seg, beat)
        end
        beat = beat - seg.beats
    end
    return self.end_point + beat * self.end_tempo
end

function VariableTimescale:dur(beat, start)
    start = start or 0
    return self:time(start + beat) - self:time(start)
end

Metre = {}
Metre.__index = Metre

function Metre.new(timescale, beats_in_bar)
    if type(timescale) ~= "table" or type(timescale.time) ~= "function" or type(timescale.dur) ~= "function" then
        error("timescale should be a ConstantTimescale, VariableTimescale or a Timescale-like type that defines function time(beats) and dur(beats [, start])")
    end
    return setmetatable({
        timescale = timescale,
        beats_in_bar = beats_in_bar,
        offset = 0
    }, Metre)
end

function Metre.newNoTimescale(beats_in_bar)
    return setmetatable({
        beats_in_bar = beats_in_bar,
    }, Metre)
end

function Metre:offset(beats)
    self.offset = self.offset + beats
end

function Metre:setOffset(beats)
    self.offset = beats
end

function Metre:beats(bars_or_beats, beats_or_nil)
    if beats_or_nil then
        return bars_or_beats * self.beats_in_bar + beats_or_nil
    else
        return bars_or_beats
    end
end

function Metre:dur(bars_or_beats, beats_or_nil, start_bars_or_beats, start_beats_or_nil)
    if beats_or_nil then
        bars_or_beats = bars_or_beats * self.beats_in_bar + beats_or_nil
    end
    start_bars_or_beats = start_bars_or_beats or 0
    if start_beats_or_nil then
        start_bars_or_beats = start_bars_or_beats * self.beats_in_bar + start_beats_or_nil
    end
    return self.timescale:dur(bars_or_beats, start_bars_or_beats)
end

function Metre:time(bars_or_beats, beats_or_nil)
    if beats_or_nil then
        return self.timescale:time(bars_or_beats * self.beats_in_bar + beats_or_nil)
    else
        return self.timescale:time(bars_or_beats)
    end
end

--Melody utility: pitch, chords, scales

Pitch = {}
Pitch.__index = Pitch

function halftones(num, value)
    if not verify_num(num) then
        error("num should be a number")
    end
    if value then
        if verify_num(value) then
            value = math.floor(value)
        else
            error("value should be a number")
        end
    else
        value = 0
    end
    return setmetatable({ octaves = num / 12.0, value = value }, Pitch)
end

function octaves(num, value)
    if not verify_num(num) then
        error("num should be a number")
    end
    if value then
        if verify_num(value) then
            value = math.floor(value)
        else
            error("value should be a number")
        end
    else
        value = 0
    end
    return setmetatable({ octaves = num, value = value }, Pitch)
end

function value_pitch(value)
    if not verify_num(value) then
        error("value should be a number")
    end
    value = math.floor(value)
    return setmetatable({ octaves = 0, value = value }, Pitch)
end

function Pitch.__add(p1, p2)
    if getmetatable(p1) == Pitch then
        if getmetatable(p2) == Pitch then
            return octaves(p1.octaves + p2.octaves, p1.value + p2.value)
        elseif verify_num(p2) then
            return octaves(p1.octaves, p1.value + p2)
        else
            error("second operant should be Pitch or a number")
        end
    elseif verify_num(p1) then
        return octaves(p2.octaves, p2.value + p1)
    else
        error("first operant should be Pitch or a number")
    end
end

function Pitch:getMultiplier()
    return 2 ^ self.octaves
end

function Pitch:getOctaves()
    return self.octaves
end

function Pitch:getNumberValue()
    return self.value
end

function Pitch:wrapOctave(num_in_octave)
    local pitch_value_octaves = math.floor(self.value / num_in_octave)
    local new_value = self.value - pitch_value_octaves * num_in_octave
    return octaves(pitch_value_octaves + self.octaves, new_value)
end

function Pitch:getFrequency(base_frequency)
    return base_frequency * 2 ^ self.octaves
end

local function verify_octaves_pitch(pitch)
    if getmetatable(pitch) == Pitch then
        return pitch
    elseif verify_num(pitch) then
        return octaves(pitch)
    else
        return nil
    end
end

local function verify_value_pitch(pitch)
    if getmetatable(pitch) == Pitch then
        return pitch
    elseif verify_num(pitch) then
        return value_pitch(pitch)
    else
        return nil
    end
end

Scale = {}
Scale.__index = Scale

local function verify_scale_note(note)
    if getmetatable(note) == Pitch then
        return note:getMultiplier()
    elseif verify_num(note) then
        return note
    else
        return nil
    end
end

function Scale.new(notes)
    local tab_notes = {}
    for i, n in ipairs(notes) do
        local value = verify_scale_note(n)
        if value then
            tab_notes[i - 1] = value
        else
            error("note should be Pitch or a number")
        end
    end
    return setmetatable({ multipliers = tab_notes, num_notes = #tab_notes + 1 }, Scale)
end

function Scale:mapPitch(note_in_scale)
    local verified = verify_value_pitch(note_in_scale)
    if verified then
        return verified:wrapOctave(self.num_notes)
    else
        error("note_in_scale should be Pitch or num")
    end
end

function Scale:getFrequency(base, pitch)
    local mapped = self:mapPitch(pitch)
    return base * self.multipliers[mapped.value] * mapped:getMultiplier()
end

Chord = {}
Chord.__index = Chord

function Chord.new(notes)
    local verified_notes = {}
    for i, pitch in ipairs(notes) do
        local verified = verify_value_pitch(pitch)
        if verified then
            verified_notes[i] = verified
        else
            error("notes should be array of Pitch or num")
        end
    end
    return setmetatable({ notes = verified_notes, num_notes = #verified_notes }, Chord)
end

function Chord:mapPitch(note_in_chord)
    local verified = verify_value_pitch(note_in_chord)
    if verified then
        verified = verified:wrapOctave(self.num_notes)
    else
        error("note_in_chord should be Pitch or num")
    end
    return self.notes[verified.value + 1] + octaves(verified.octaves)
end

function Chord:getNotes(scale)
    if getmetatable(scale) == Scale then
        local result = {}
        for i, pitch in ipairs(self.notes) do
            result[i] = scale:mapPitch(pitch)
        end
        return result
    else
        error("scale should be an Scale")
    end
end

function Chord:transpose(pitch)
    pitch = verify_value_pitch(pitch)
    if pitch then
        local new_notes = {}
        for i, note in ipairs(self.notes) do
            new_notes[i] = self:mapPitch(note + pitch)
        end
        return setmetatable({ notes = new_notes, num_notes = self.num_notes }, Chord)
    else
        error("pitch should be an Pitch or num")
    end
end

--Note type - output of melody

Note = {}
Note.__index = Note

function Note:dup()
    return setmetatable({
        pitch = self.pitch,
        duration = self.duration,
        start_point = self.start_point,
        scale = self.scale,
        chord = self.chord,
        default_scale = self.default_scale,
        vars = self.vars
    }, Note)
end

function Note:transpose(pitch)
    return setmetatable({
        pitch = self.pitch + pitch,
        duration = self.duration,
        start_point = self.start_point,
        scale = self.scale,
        chord = self.chord,
        default_scale = self.default_scale,
        vars = self.vars
    }, Note)
end

function Note:offset(offset)
    return setmetatable({
        pitch = self.pitch,
        duration = self.duration,
        start_point = self.start_point + offset,
        scale = self.scale,
        chord = self.chord,
        default_scale = self.default_scale,
        vars = self.vars
    }, Note)
end

function Note:dur(duration)
    return setmetatable({
        pitch = self.pitch,
        duration = duration,
        start_point = self.start_point,
        scale = self.scale,
        chord = self.chord,
        default_scale = self.default_scale,
        vars = self.vars
    }, Note)
end

--Melody initialization control

MelodyControl = {}

local MODE_MELODIC = 1
local MODE_CHORD = 2
local MODE_CHROMATIC = 3

function melodicMode()
    return setmetatable({ mode = MODE_MELODIC }, MelodyControl)
end

function chordMode()
    return setmetatable({ mode = MODE_CHORD }, MelodyControl)
end

function chromaticMode()
    return setmetatable({ mode = MODE_CHROMATIC }, MelodyControl)
end

function startCluster()
    return setmetatable({ do_advance_time = false }, MelodyControl)
end

function endCluster()
    return setmetatable({ do_advance_time = true }, MelodyControl)
end

function resetScale()
    return setmetatable({ reset_scale = true }, MelodyControl)
end

function setScale(new_scale)
    if getmetatable(new_scale) == Scale then
        return setmetatable({ scale = new_scale }, MelodyControl)
    else
        error("expected a Scale")
    end
end

function resetChord()
    return setmetatable({ reset_chord = true }, MelodyControl)
end

function setChord(new_chord)
    if getmetatable(new_chord) == Chord then
        return setmetatable({ chord = new_chord }, MelodyControl)
    else
        error("expected a Chord")
    end
end

function resetMetre()
    return setmetatable({ reset_metre = true }, MelodyControl)
end

function setMetre(new_metre)
    if getmetatable(new_metre) == Metre then
        return setmetatable({ metre = new_metre }, MelodyControl)
    end
end

function resetTimePoint()
    return setmetatable({ time_point = 0 }, MelodyControl)
end

function setTimePoint(time_point)
    if verify_num(time_point) then
        return setmetatable({ time_point = time_point }, MelodyControl)
    else
        error("expected a number")
    end
end

function advanceTimePoint(duration)
    if verify_num(duration) then
        return setmetatable({ advance_time_point = duration }, MelodyControl)
    else
        error("expected a number")
    end
end

function resetDefaultInterval()
    return setmetatable({ reset_interval = true }, MelodyControl)
end

function setDefaultInterval(new_interval)
    if verify_num(new_interval) then
        return setmetatable({ interval = new_interval }, MelodyControl)
    else
        error("expected a number")
    end
end

function resetDefaultDuration()
    return setmetatable({ reset_duration = true }, MelodyControl)
end

function setDefaultDuration(new_duration)
    if verify_num(new_duration) then
        return setmetatable({ duration = new_duration }, MelodyControl)
    else
        error("expected a number")
    end
end

function resetTranspose()
    return setmetatable({ reset_transpose = true }, MelodyControl)
end

function setTranspose(pitch)
    local verified = verify_octaves_pitch(pitch)
    if verified then
        return setmetatable({ transpose = verified }, MelodyControl)
    else
        error("expected a Pitch or number")
    end
end

function transpose(pitch)
    local verified = verify_octaves_pitch(pitch)
    if verified then
        return setmetatable({ transpose_add = verified }, MelodyControl)
    else
        error("expected a Pitch or number")
    end
end

--Melody creation and operations

Melody = {}
Melody.__index = Melody

function Melody.new(notes)
    local do_advance_time = true
    local mode = MODE_MELODIC
    local time_point = 0
    local scale, chord, metre, transpose, default_interval, default_duration

    local verified_notes = {}

    local index = 1
    for i, elem in ipairs(notes) do
        if getmetatable(elem) == MelodyControl then
            if elem.do_advance_time then
                do_advance_time = true
            elseif elem.do_advance_time == false then
                do_advance_time = false
            elseif elem.mode then
                mode = elem.mode
            elseif elem.reset_chord then
                chord = nil
            elseif elem.reset_duration then
                default_duration = nil
            elseif elem.duration then
                default_duration = elem.duration
            elseif elem.reset_interval then
                default_interval = nil
            elseif elem.interval then
                default_interval = elem.interval
            elseif elem.reset_transpose then
                transpose = nil
            elseif elem.transpose then
                transpose = elem.transpose
            elseif elem.transpose_add then
                transpose = transpose and transpose + elem.transpose_add or elem.transpose_add
            elseif elem.chord then
                chord = elem.chord
            elseif elem.reset_scale then
                scale = nil
            elseif elem.scale then
                scale = elem.scale
            elseif elem.reset_metre then
                metre = nil
            elseif elem.metre then
                metre = elem.metre
            elseif elem.time_point then
                time_point = elem.time_point
            elseif elem.advance_time_point then
                time_point = time_point + elem.advance_time_point
            end
        else
            local pitch, pitch2, duration, start_point, note_scale, note_chord, note_defaut_scale, vars
            pitch = verify_value_pitch(elem)
            if not pitch then
                if elem[1] then
                    pitch = elem[1]
                    if elem[2] then
                        duration = elem[2]
                        if elem[3] then
                            start_point = elem[3]
                        end
                    end
                else
                    if elem.pitch then
                        pitch = elem.pitch
                    end
                    if elem.duration then
                        duration = elem.duration
                    end
                    if elem.start_point then
                        start_point = elem.start_point
                    end
                end
                pitch = verify_value_pitch(pitch)
                pitch2 = verify_value_pitch(elem.pitch2)
                vars = elem.vars
            end
            if not pitch then
                error("pitch was not specified in the note")
            end
            if start_point then
                if not verify_num(start_point) then
                    if verify_num(start_point[1]) then
                        local note = verify_num(start_point[2]) or 0
                        if metre then
                            start_point = metre:beats(start_point[1], note)
                        else
                            error("metre is not set and start_point has bar and beat format")
                        end
                    else
                        error("start_point isn't a number and isn't an array {bar[, beat]}")
                    end
                end
            else
                start_point = time_point
            end
            if duration then
                if not verify_num(duration) then
                    if verify_num(duration[1]) then
                        local beat = verify_num(duration[2]) or 0
                        if metre then
                            duration = metre:beats(duration[1], beat)
                        else
                            error("metre is not set and duration has bar and beat format")
                        end
                    else
                        error("duration isn't a number and isn't an array {bar[, beat]}")
                    end
                end
            else
                if default_duration then
                    duration = default_duration
                else
                    error("default interval is not set and default duration isn't provided")
                end
            end
            if duration < 0 then
                error("duration is negative")
            end
            if mode ~= MODE_CHROMATIC then
                if scale then
                    note_scale = scale
                else
                    note_defaut_scale = true
                end
                if mode == MODE_CHORD then
                    note_chord = chord
                end
            end
            if transpose then
                pitch = pitch + transpose
                if pitch2 then
                    pitch2 = pitch2 + transpose
                end
            end
            verified_notes[index] = setmetatable({
                pitch = pitch,
                pitch2 = pitch2,
                duration = duration,
                start_point = start_point,
                scale = note_scale,
                chord = note_chord,
                default_scale = note_defaut_scale,
                vars = vars
            }, Note)
            index = index + 1
            if do_advance_time then
                if default_interval then
                    time_point = time_point + default_interval
                else
                    time_point = time_point + duration
                end
            end
        end
    end

    return setmetatable({
        notes = verified_notes
    }, Melody)
end

function Melody:getNotes(offset, base_frequency, timescale, default_scale)
    if not verify_num(offset) then
        error("offset should be a number")
    end
    if not verify_pnum(base_frequency) then
        error("base frequency should be a positive number")
    end
    if not (getmetatable(timescale) == ConstantTimescale or getmetatable(timescale) == VariableTimescale) then
        error("timescale should be a ConstantTimescale or a VariableTimescale")
    end
    if default_scale and (not getmetatable(default_scale) == Scale) then
        error("default scale exists and is not a Scale")
    end
    local result = {}
    for i, note in ipairs(self.notes) do
        local pitch = note.pitch
        local pitch2 = note.pitch2
        local frequency
        local frequency2
        if note.chord then
            pitch = note.chord:mapPitch(pitch)
        end
        if note.scale then
            frequency = note.scale:getFrequency(base_frequency, pitch)
            if pitch2 then
                frequency2 = note.scale:getFrequency(base_frequency, pitch2)
            end
        elseif note.default_scale then
            frequency = default_scale:getFrequency(base_frequency, pitch)
            if pitch2 then
                frequency2 = default_scale:getFrequency(base_frequency, pitch2)
            end
        else
            frequency = base_frequency * pitch:getMultiplier() * 2 ^ (pitch:getNumberValue() / 12)
            if pitch2 then
                frequency2 = base_frequency * pitch2:getMultiplier() * 2 ^ (pitch2:getNumberValue() / 12)
            end
        end
        local start_beat = offset + note.start_point
        local end_beats = start_beat + note.duration
        local start_point = timescale:time(start_beat)
        local duration = timescale:time(end_beats) - start_point
        if frequency2 then
            frequency = ParSlide(duration, frequency, frequency2)
        end
        result[i] = { frequency, duration, start_point, end_beats - start_beat, start_beat, vars = note.vars }
    end
    return result
end

function Melody:merge(other)
    if getmetatable(other) ~= Melody then
        error("expected a Melody")
    end
    local result_notes = {}
    local i = 1
    for _, note in ipairs(self.notes) do
        result_notes[i] = note
        i = i + 1
    end
    for _, note in ipairs(other.notes) do
        result_notes[i] = note
        i = i + 1
    end
    return setmetatable({
        notes = result_notes
    }, Melody)
end

function Melody:addNotes(notes)
    return self:merge(Melody.new(nodes))
end

function Melody:fragment(start_beat, duration_beats)
    if not (verify_num(start_beat) and verify_num(duration_beats)) then
        error("start_beat and duration_beats should be num")
    end
    local end_beat = start_beat + duration_beats
    local result_notes = {}
    local i = 1
    for _, note in ipairs(self.notes) do
        if note.start_point >= start_beat and note.start_point < end_beat then
            result_notes[i] = setmetatable({
                pitch = note.pitch,
                duration = note.duration,
                start_point = note.start_point - start_beat,
                scale = note.scale,
                chord = note.chord,
                default_scale = note.default_scale,
                vars = note.vars
            }, Note)
            i = i + 1
        end
    end
    return setmetatable({
        notes = result_notes
    }, Melody)
end

function Melody:removeFragment(start_beat, duration_beats)
    if not (verify_num(start_beat) and verify_num(duration_beats)) then
        error("start_beat and duration_beats should be num")
    end
    local end_beat = start_beat + duration_beats
    local result_notes = {}
    local i = 1
    for _, note in ipairs(self.notes) do
        if note.start_point < start_beat and note.start_point >= end_beat then
            result_notes[i] = note
            i = i + 1
        end
    end
    return setmetatable({
        notes = result_notes
    }, Melody)
end

function Melody:fragmentRepeat(num_repeats, start_beat, duration_beats, repeat_interval)
    if not (verify_num(start_beat) and verify_num(duration_beats)) then
        error("start_beat and duration_beats should be num")
    end
    if repeat_interval then
        if not verify_num(repeat_interval) then
            error("repeat_interval should be num")
        end
    else
        repeat_interval = duration_beats
    end
    local end_beat = start_beat + duration_beats
    local result_notes = {}
    local i = 1
    for _, note in ipairs(self.notes) do
        if note.start_point >= start_beat and note.start_point < end_beat then
            result_notes[i] = setmetatable({
                pitch = note.pitch,
                duration = note.duration,
                start_point = note.start_point - start_beat,
                scale = note.scale,
                chord = note.chord,
                default_scale = note.default_scale,
                vars = note.vars
            }, Note)
            i = i + 1
        end
    end
    local num_in_repeat = i - 1
    local repeat_offset = repeat_interval
    for _ = 2, num_repeats do
        for j = 1, num_in_repeat do
            local note = result_notes[j]
            result_notes[i] = setmetatable({
                pitch = note.pitch,
                duration = note.duration,
                start_point = note.start_point + repeat_offset,
                scale = note.scale,
                chord = note.chord,
                default_scale = note.default_scale,
                vars = note.vars
            }, Note)
            i = i + 1
        end
        repeat_offset = repeat_offset + repeat_interval
    end
    return setmetatable({
        notes = result_notes
    }, Melody)
end

function Melody:transpose(pitch)
    if getmetatable(pitch) ~= Pitch then
        if verify_num(pitch) then
            pitch = octaves(pitch)
        else
            error("expected a Pitch or number")
        end
    end

    local result_notes = {}
    for i, note in ipairs(self.notes) do
        result_notes[i] = setmetatable({
            pitch = note.pitch + pitch,
            duration = note.duration,
            start_point = note.start_point,
            scale = note.scale,
            chord = note.chord,
            default_scale = note.default_scale,
            vars = note.vars
        }, Note)
    end
    return setmetatable({
        notes = result_notes
    }, Melody)
end

function Melody:transposeNonChords(pitch)
    if getmetatable(pitch) ~= Pitch then
        if verify_num(pitch) then
            pitch = octaves(pitch)
        else
            error("expected a Pitch or number")
        end
    end

    local result_notes = {}
    for i, note in ipairs(self.notes) do
        result_notes[i] = setmetatble({
            pitch = not note.chord and note.pitch + pitch or note.pitch,
            duration = note.duration,
            start_point = note.start_point,
            scale = note.scale,
            chord = note.chord,
            default_scale = note.default_scale,
            vars = note.vars
        }, Note)
    end
    return setmetatable({
        notes = result_notes
    }, Melody)
end

function Melody:transposeChords(pitch)
    if getmetatable(pitch) ~= Pitch then
        if verify_num(pitch) then
            pitch = octaves(pitch)
        else
            error("expected a Pitch or number")
        end
    end

    local result_notes = {}
    for i, note in ipairs(self.notes) do
        result_notes[i] = setmetatable({
            pitch = note.chord and note.pitch + pitch or note.pitch,
            duration = note.duration,
            start_point = note.start_point,
            scale = note.scale,
            chord = note.chord,
            default_scale = note.default_scale,
            vars = note.vars
        }, Note)
    end
    return setmetatable({
        notes = result_notes
    }, Melody)
end

function Melody:changeStartPoint(start_point)
    if not verify_num(start_point) then
        error("expected a number")
    end

    local result_notes = {}
    for i, note in ipairs(self.notes) do
        result_notes[i] = setmetatable({
            pitch = note.pitch,
            duration = note.duration,
            start_point = note.start_point - start_point,
            scale = note.scale,
            chord = note.chord,
            default_scale = note.default_scale,
            vars = note.vars
        }, Note)
    end
    return setmetatable({
        notes = result_notes
    }, Melody)
end

function Melody:map(map_func)
    if type(map_func) ~= "function" then
        error("expected a function")
    end
    local result_notes = {}
    for i, note in ipairs(self.notes) do
        result_notes[i] = map_func(setmetatable({
            pitch = note.pitch,
            duration = note.duration,
            start_point = note.start_point,
            scale = note.scale,
            chord = note.chord,
            default_scale = note.default_scale,
            vars = note.vars
        }, Note))
    end
    return setmetatable({
        notes = result_notes
    }, Melody)
end

function Melody:mapToMultiple(map_func)
    if type(map_func) ~= "function" then
        error("expected a function")
    end
    local result_notes = {}
    local i = 1
    for _, note in ipairs(self.notes) do
        local map_result = map_func(setmetatable({
            pitch = note.pitch,
            duration = note.duration,
            start_point = note.start_point,
            scale = note.scale,
            chord = note.chord,
            default_scale = note.default_scale,
            vars = note.vars
        }, Note))
        for _, res_note in ipairs(map_result) do
            result_notes[i] = res_note
            i = i + 1
        end
    end
    return setmetatable({
        notes = result_notes
    }, Melody)
end

function Melody:mapNoDup(map_func)
    if type(map_func) ~= "function" then
        error("expected a function")
    end
    local result_notes = {}
    for i, note in ipairs(self.notes) do
        result_notes[i] = map_func(note)
    end
    return setmetatable({
        notes = result_notes
    }, Melody)
end

function Melody:filter(filter_func)
    if type(filter_func) ~= filter_func then
        error("expected a function")
    end
    local result_notes = {}
    local i = 1
    for _, note in ipairs(self.notes) do
        if filter_func(note) then
            result_notes[i] = note
            i = i + 1
        end
    end
    return setmetatable({
        notes = result_notes
    }, Melody)
end

function Melody.__add(m1, m2)
    if getmetatable(m1) ~= Melody then
        error("expected a Melody")
    end
    return m1:merge(m2)
end

--Timbres

Timbre = {}
Timbre.__index = Timbre

local template_func = function(buf, frequency, duration, start_point, duration_beats, start_beat, timbre_vars, note_vars)
end

function Timbre.new(func)
    return setmetatable({ func = func, vars = {} }, Timbre)
end

--note format: { frequency, duration, start_point, duration_beats, start_beat, vars = {} }

function Timbre:setVarsOffset(offset)
    self.vars_offset = offset
end

function Timbre:resetVarsOffset()
    self.vars_offset = nil
end

function Timbre:outputNotes(buf, notes)
    if getmetatable(buf) ~= BufferWrapper then
        error("buf should be a buffer")
    end
    local vars = {}
    if self.vars_offset and self.vars_offset ~= 0 then
        for k, v in pairs(self.vars) do
            if getmetatable(v) == ParWrapper then
                vars[k] = v:offset(-self.vars_offset)
            elseif getmetatable(v) == QuickFunction then
                vars[k] = SetStart(v, self.vars_offset)
            elseif type(v) == "number" then
                vars[k] = Const(v)
            else
                vars[k] = v
            end
        end
    else
        for k, v in pairs(self.vars) do
            if type(v) == "number" then
                vars[k] = Const(v)
            else
                vars[k] = v
            end
        end
    end
    for i, note in ipairs(notes) do
        self.func(buf, note[1], note[2], note[3], note[4], note[5], vars, note.vars or {})
    end
end

function Timbre:output(buf, offset, melody, base, timescale, scale)
    base = base or default_base
    timescale = timescale or default_timescale
    scale = scale or default_scale
    local notes = melody:getNotes(offset, base, timescale, scale)
    self:outputNotes(buf, notes)
end

function Timbre:outputRepeat(buf, offset, melody, base, timescale, scale)
    base = base or default_base
    timescale = timescale or default_timescale
    scale = scale or default_scale
    local notes = melody:getNotes(offset, base, timescale, scale)
    self:outputNotes(buf, notes)
end

-- Timbre compound - easier construction of timbres and more flexibility
-- not finished and will probably never be finished

local TimbreCompoundSegment = {}
TimbreCompoundSegment.__index = TimbreCompoundSegment

TimbreCompound = {}
TimbreCompound.__index = TimbreCompound
setmetatable(TimbreCompound, Timbre)

function TimbreCompound.new()
    return setmetatable({
        buffers = {},
        vars = {},
    }, TimbreCompound)
end

function TimbreCompound:addMonoBuffer(id)
    if type(id) ~= "string" then
        error("Expected a string buffer id")
    end
    if self.buffers[id] then
        error("Buffer '" .. id .. "' already exists")
    end
    self.buffers[id] = BufferWrapper.newMono()
end

function TimbreCompound:addStereoBuffer(id)
    if type(id) ~= "string" then
        error("Expected a string buffer id")
    end
    if self.buffers[id] then
        error("Buffer '" .. id .. "' already exists")
    end
    self.buffers[id] = BufferWrapper.newStereo()
end

function TimbreCompound:registerBuffer(id, buf)
    if type(id) ~= "string" then
        error("Expected a string buffer id")
    end
    if self.buffers[id] then
        error("Buffer '" .. id .. "' already exists")
    end
    if not getmetatable(buf) == BufferWrapper then
        error("buf should be a buffer")
    end
    self.buffers[id] = buf
end

function TimbreCompound:setBuffer(id, offset)
    if not id then
        self.current_buffer = nil
    elseif type(id) ~= "string" then
        error("Expected a string buffer id or false/nil")
    else
        local buf = self.buffers[id]
        if buf then
            error("Buffer '" .. id .. "' doesn't exist")
        end
        self.current_buffer = buf
        self.current_buffer_offset = offset
    end
end

function TimbreCompound:setTargetBuffer(id, offset)
    if not id then
        self.current_target_buffer = nil
    elseif type(id) ~= "string" then
        error("Expected a string buffer id or false/nil")
    else
        local buf = self.buffers[id]
        if buf then
            error("Buffer '" .. id .. "' doesn't exist")
        end
        self.current_target_buffer = buf
        self.current_target_buffer_offset = offset
    end
end

--[[
--Introduce unified time format(seconds/beats/bars+beats), make all Metres/Timbres/Melodies accept it
--
 ]]

--Delay

function delay(buf, layers, start_point, length)
    if not getmetatable(buf) == BufferWrapper then
        error("buf should be a buffer")
    end
    start_point = start_point or buf.start_point
    length = length or buf.end_point - buf.start_point
    if start_point ~= start_point then
        error("buffer has no length")
    end
    local stereo = buf.bufferR
    for i, layer in ipairs(layers) do
        layer.vol = verify_par(layer.vol)
        if not layer.vol then
            error("Invalid vol in layer " .. i)
        end
        layer.pan = verify_par(layer.pan)
        if not layer.pan then
            layer.pan = 0
        end
        layer.time = verify_num(layer.time)
        if not layer.time then
            error("Invalid time in layer " .. i)
        end
    end
    buf:endLevel()
    local buffers = {}
    for i, layer in ipairs(layers) do
        if stereo then
            buffers[i] = BufferWrapper.newStereo(buf.sample_rate)
        else
            buffers[i] = BufferWrapper.newMono(buf.sample_rate)
        end
        buf:copyBuffer(buffers[i], -start_point, layer.vol, start_point, length, true)
        buffers[i]:endLevel()
        if stereo then
            buffers[i]:improvedStereo(layer.pan)
            buffers[i]:endLevel()
        end
    end
    buf:endLevel()
    for i, layer in ipairs(layers) do
        buffers[i]:copyBuffer(buf, start_point + layer.time, layer.vol, 0, length, true)
        buffers[i]:endLevel()
    end
    buf:endLevel()
end

function delayLayers(num, time, ampMul, pan, panMul)
    ampMul = ampMul or 1
    pan = pan or 0
    panMul = panMul or 1
    local result = {}
    local delay_time = time
    local vol = ampMul
    for i = 1, num do
        result[i] = { vol = vol, pan = pan, time = delay_time }
        delay_time = delay_time + time
        pan = pan * panMul
        vol = vol * ampMul
    end
    return result
end

function delayLayersMulPow(num, time, ampMul, ampMulPow, pan, panMul)
    ampMul = ampMul or 1
    pan = pan or 0
    panMul = panMul or 1
    local result = {}
    local delay_time = time
    local vol = ampMul
    for i = 1, num do
        result[i] = { vol = vol, pan = pan, time = delay_time }
        delay_time = delay_time + time
        pan = pan * panMul
        ampMul = math.pow(ampMul, ampMulPow)
        vol = vol * ampMul
    end
    return result
end

function delayLayersSlowing(num, time, timeMul, ampMul, pan, panMul)
    ampMul = ampMul or 1
    pan = pan or 0
    panMul = panMul or 1
    local result = {}
    local delay_time = time
    local vol = ampMul
    for i = 1, num do
        result[i] = { vol = vol, pan = pan, time = delay_time }
        time = time * timeMul
        delay_time = delay_time + time
        pan = pan * panMul
        vol = vol * ampMul
    end
    return result
end

function delayLayersSlowingMulPow(num, time, timeMul, ampMul, ampMulPow, pan, panMul)
    ampMul = ampMul or 1
    pan = pan or 0
    panMul = panMul or 1
    local result = {}
    local delay_time = time
    local vol = ampMul
    for i = 1, num do
        result[i] = { vol = vol, pan = pan, time = delay_time }
        time = time * timeMul
        delay_time = delay_time + time
        pan = pan * panMul
        ampMul = math.pow(ampMul, ampMulPow)
        vol = vol * ampMul
    end
    return result
end

function unison_spread(spread, num, index)
    if num == 1 then
        return 0
    else
        return (((index - 1) / (num - 1) * 2) - 1) * spread
    end
end

--Common scales and chords

local modal = {
    aeolian = Scale.new { halftones(0), halftones(2), halftones(3), halftones(5), halftones(7), halftones(8), halftones(10) },
    dorian = Scale.new { halftones(0), halftones(2), halftones(3), halftones(5), halftones(7), halftones(9), halftones(10) },
    ionian = Scale.new { halftones(0), halftones(2), halftones(4), halftones(5), halftones(7), halftones(9), halftones(11) },
    locrian = Scale.new { halftones(0), halftones(1), halftones(3), halftones(5), halftones(6), halftones(8), halftones(10) },
    lydian = Scale.new { halftones(0), halftones(2), halftones(4), halftones(6), halftones(7), halftones(9), halftones(11) },
    mixolydian = Scale.new { halftones(0), halftones(2), halftones(4), halftones(5), halftones(7), halftones(9), halftones(10) },
    phrygian = Scale.new { halftones(0), halftones(1), halftones(3), halftones(5), halftones(7), halftones(8), halftones(10) }
}

scales = {
    major = modal.ionian,
    major6m = Scale.new { halftones(0), halftones(2), halftones(4), halftones(5), halftones(7), halftones(8), halftones(11) },
    major7m = modal.mixolydian,
    major6m7m = Scale.new { halftones(0), halftones(2), halftones(4), halftones(5), halftones(7), halftones(8), halftones(10) },
    minor = modal.aeolian,
    minor6 = modal.dorian,
    minor7 = Scale.new { halftones(0), halftones(2), halftones(3), halftones(5), halftones(7), halftones(8), halftones(11) },
    minor67 = Scale.new { halftones(0), halftones(2), halftones(3), halftones(5), halftones(7), halftones(9), halftones(11) },
    modal = modal,
    diminished = Scale.new { halftones(0), halftones(1), halftones(3), halftones(4), halftones(6), halftones(7), halftones(9), halftones(10) },
    augmented = Scale.new { halftones(0), halftones(2), halftones(4), halftones(6), halftones(8), halftones(10) },
}

chords = {
    trichord = Chord.new { 0, 2, 4 },
    tetrachord = Chord.new { 0, 2, 4, 6 },
    trichord7 = Chord.new { 0, 2, 4, 6 },
    trichord9 = Chord.new { 0, 2, 4, 6, 8 },
    trichord6 = Chord.new { 0, 2, 4, 5 },
    octaves = Chord.new { 0 }
}