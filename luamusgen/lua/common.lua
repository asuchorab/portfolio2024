local c = {}
function c.reverbBuf(buf, ro)
    if buf:hasLength() then
        buf:simpleReverb(0.92, 0.4, 0.54, 1.0, 1.0, 0, buf.end_point + 1)
    end
end

function c.reverbBuf2(buf2, ro)
    if buf2:hasLength() then
        buf2:simpleReverb(0.78, 0.2, 0.8, 0.8, 1.0, nil, buf2.end_point - buf2.start_point + 1)
    end
end

function c.reverbBuf3(buf3, ro)
    if buf3:hasLength() then
        buf3:simpleReverseReverb(0.92, 0.3, 0.6, 0.9, 1.0, -ro:dur(8))
    end
end

function c.reverbBufD(bufD, ro)
    if bufD:hasLength() then
        delay(bufD, delayLayersMulPow(5, ro:dur(2.1), 0.6, 0.6, -0.25, -1.8))
    end
end

function c.reverbBufD2(bufD, ro)
    if bufD:hasLength() then
        delay(bufD, delayLayersMulPow(5, ro:dur(2.1), 0.55, 0.6, -0.25, -1.8))
    end
end

function c.reverbBufClm(buf, ro)
    if buf:hasLength() then
        buf:simpleReverb(0.75, 0.4, 0.5, 0.9, 1.0, 0, buf.end_point + 1)
    end
end

function c.reverbBufEndClm(buf, ro)
    if buf:hasLength() then
        buf:simpleReverb(0.9, 0.6, 0.3, 0.7, 0.8, 0, buf.end_point + 1)
        buf:simpleReverb(0.9, 0.6, 0.3, 0.7, 0.96, 0, buf.end_point + 1)
    end
end

function c.defaults()
    default_timescale = ConstantTimescale.new(0, 120 * 2)
    default_base = halftones(1):getFrequency(440)
    default_scale = scales.minor
    local m = Metre.new(default_timescale, 4)
    return m
endreturn c