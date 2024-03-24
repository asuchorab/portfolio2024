--setThreadCount(1)
--setMaxProcessedSegments(1)
--setPrintGraph(true)
--setSegmentsDirectory("seg")
--setUseFFI(false)
--setLoadVariables(false)
setProcessUnchangedFiles(true)

local common_prereq = {"common.lua", "test_melodies.lua"}
loadSegment("common.lua")
loadSegment("test_melodies.lua")
loadSegment("test_flnoacc3.lua", common_prereq)
loadSegment("test_poly3accsm.lua", common_prereq)