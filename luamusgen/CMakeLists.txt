cmake_minimum_required(VERSION 3.13)
project(luamusgen)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(EXECUTABLE_NAME "luamusgen_debug")
    set(LIBRARY_NAME "libluamusgen_debug")
    set(TEST_NAME "luamusgentest_debug")
else()
    set(EXECUTABLE_NAME "luamusgen")
    set(LIBRARY_NAME "libluamusgen")
    set(TEST_NAME "luamusgentest")
endif()

include_directories(src include)
link_directories(libs)

set(LUAMUSGEN_SRC src/scriptloader/lua_segment_processing.cpp src/util/WAV_File.cpp src/util/Logger.cpp src/parameters/ParData.h src/parameters/ParDataConst.h src/parameters/ParDataArray.h src/parameters/kinds/ParLerp.cpp src/parameters/kinds/ParLerp.h src/buffer/Buffer.cpp src/buffer/Buffer.h src/transforms/kinds/generators/WhiteNoise.cpp src/transforms/kinds/generators/WhiteNoise.h src/util/RandomWrapper.hpp src/util/ThreadLocalRandom.cpp src/util/ThreadLocalRandom.h src/transforms/kinds/buffer_operations/CopyBuffer.cpp src/transforms/kinds/buffer_operations/CopyBuffer.h src/util/TimeUtil.h src/transforms/kinds/filters/other/ChangeVolume.h src/parameters/ParDeduce.hpp src/parameters/kinds/ParAdd.cpp src/parameters/kinds/ParAdd.h src/util/ClassUtil.h src/transforms/kinds/filters/butterworth/ButterworthLowPass.h src/transforms/kinds/filters/butterworth/ButterworthLowPass.cpp src/transforms/kinds/filters/butterworth/ButterworthHighPass.cpp src/transforms/kinds/filters/butterworth/ButterworthHighPass.h src/transforms/kinds/filters/other/ChangeVolume.cpp src/parameters/kinds/ParMul.cpp src/parameters/kinds/ParMul.h src/transforms/kinds/filters/butterworth/ButterworthBandPass.cpp src/transforms/kinds/filters/butterworth/ButterworthBandPass.h src/transforms/kinds/filters/butterworth/ButterworthBandStop.cpp src/transforms/kinds/filters/butterworth/ButterworthBandStop.h src/parameters/kinds/ParExp.cpp src/parameters/kinds/ParExp.h src/parameters/kinds/ParLinearExp.cpp src/parameters/kinds/ParLinearExp.h src/parameters/Parameter.cpp src/parameters/Parameter.h src/parameters/ParGenerator.h src/parameters/ParData.h src/parameters/ParDataView.h src/processgraph/ProcessGraph.cpp src/processgraph/ProcessGraph.h src/processgraph/ProcessNode.cpp src/processgraph/ProcessNode.h src/processgraph/kinds/ParameterNode.cpp src/processgraph/kinds/ParameterNode.h src/processgraph/kinds/TransformNode.cpp src/processgraph/kinds/TransformNode.h src/transforms/Transform.cpp src/transforms/Transform.h src/processgraph/kinds/BufferNode.cpp src/processgraph/kinds/BufferNode.h src/processgraph/kinds/DummyNode.cpp src/processgraph/kinds/DummyNode.h src/util/MathUtil.cpp src/util/MathUtil.h src/parameters/kinds/ParConst.cpp src/parameters/kinds/ParConst.h src/transforms/kinds/buffer_operations/BufferToWAV.cpp src/transforms/kinds/buffer_operations/BufferToWAV.h src/transforms/kinds/buffer_operations/BufferFromWAV.cpp src/transforms/kinds/buffer_operations/BufferFromWAV.h test/test_basic_graph.hpp src/transforms/kinds/generators/ConstantSignal.cpp src/transforms/kinds/generators/ConstantSignal.h test/test_linexp.hpp src/parameters/kinds/ParConcat.cpp src/parameters/kinds/ParConcat.h test/test_pars.h src/parameters/kinds/ParToArray.cpp src/parameters/kinds/ParToArray.h src/parameters/kinds/ParRepeat.cpp src/parameters/kinds/ParRepeat.h src/parameters/kinds/ParTanh.cpp src/parameters/kinds/ParTanh.h src/parameters/kinds/ParLimit.cpp src/parameters/kinds/ParLimit.h src/util/Oscillators.hpp src/transforms/kinds/filters/other/SaturationTanh.cpp src/transforms/kinds/filters/other/SaturationTanh.h src/transforms/kinds/filters/other/Limiter.cpp src/transforms/kinds/filters/other/Limiter.h src/transforms/kinds/buffer_operations/Normalize.cpp src/transforms/kinds/buffer_operations/Normalize.h src/transforms/kinds/buffer_operations/SoftClipCompressor.cpp src/transforms/kinds/buffer_operations/SoftClipCompressor.h src/transforms/kinds/filters/other/RingMod.cpp src/transforms/kinds/filters/other/RingMod.h src/transforms/kinds/filters/other/SimpleReverb.cpp src/transforms/kinds/filters/other/SimpleReverb.h src/transforms/kinds/filters/other/ExpShape.cpp src/transforms/kinds/filters/other/ExpShape.h src/transforms/kinds/stereo/SimpleStereo.cpp src/transforms/kinds/stereo/SimpleStereo.h src/transforms/kinds/filters/frequency_filters/PhaserLegacy.cpp src/transforms/kinds/filters/frequency_filters/PhaserLegacy.h src/transforms/kinds/stereo/LegacyStereo.cpp src/transforms/kinds/stereo/LegacyStereo.h src/transforms/kinds/stereo/SpacialTest.cpp src/transforms/kinds/stereo/SpacialTest.h src/transforms/kinds/stereo/ImprovedStereo.cpp src/transforms/kinds/stereo/ImprovedStereo.h src/parameters/kinds/ParOscillator.h src/parameters/kinds/ParOscillatorShaped.h src/transforms/kinds/filters/frequency_filters/FlangerFeedforward.cpp src/transforms/kinds/filters/frequency_filters/FlangerFeedforward.h src/transforms/kinds/filters/frequency_filters/FlangerFeedback.cpp src/transforms/kinds/filters/frequency_filters/FlangerFeedback.h src/transforms/kinds/filters/frequency_filters/FlangerSelfOscillation.cpp src/transforms/kinds/filters/frequency_filters/FlangerSelfOscillation.h src/transforms/kinds/generators/WaveOscillator.h src/transforms/kinds/generators/WaveOscillatorShaped.h src/util/ButterworthFilters.hpp src/parameters/kinds/ParVibrato.h src/parameters/kinds/ParVibratoShaped.h src/util/MoogFilters.hpp src/transforms/kinds/filters/moog/MoogLowPass.h src/transforms/kinds/filters/moog/MoogHighPass.h src/transforms/kinds/buffer_operations/BufferToRaw.cpp src/transforms/kinds/buffer_operations/BufferToRaw.h src/transforms/kinds/buffer_operations/BufferFromRaw.cpp src/transforms/kinds/buffer_operations/BufferFromRaw.h src/scriptloader/lua_segment_register.cpp src/scriptloader/lua_segment_register.h src/scriptloader/ExecutionManager.cpp src/scriptloader/ExecutionManager.h src/main.cpp src/parameters/kinds/ParSub.cpp src/parameters/kinds/ParSub.h src/parameters/kinds/ParDiv.cpp src/parameters/kinds/ParDiv.h src/parameters/kinds/ParConstAt.cpp src/parameters/kinds/ParConstAt.h src/transforms/kinds/generators/PolysynthSwtPulse.cpp src/transforms/kinds/generators/PolysynthSwtPulse.h src/transforms/kinds/generators/SuperwaveOscillator.h src/transforms/kinds/generators/SuperwaveOscillatorShaped.h src/transforms/kinds/filters/other/SimpleReverseReverb.cpp src/transforms/kinds/filters/other/SimpleReverseReverb.h src/util/ExpandableCircularQueue.h src/scriptloader/SharedLuaVariables.cpp src/scriptloader/SharedLuaVariables.h)
add_library(${LIBRARY_NAME} STATIC ${LUAMUSGEN_SRC})

if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(${LIBRARY_NAME} Threads::Threads)
    target_link_libraries(${LIBRARY_NAME} stdc++fs)
    target_link_libraries(${LIBRARY_NAME} luajit)
    target_link_libraries(${LIBRARY_NAME} dl)
    set(CMAKE_CXX_FLAGS -rdynamic)
endif()

if(WIN32)
    target_link_libraries(${LIBRARY_NAME} luajit)
    add_compile_definitions(${LIBRARY_NAME} _USE_MATH_DEFINES)
endif()

add_executable(${EXECUTABLE_NAME} src/main.cpp)
target_link_libraries(${EXECUTABLE_NAME} ${LIBRARY_NAME})

add_executable(${TEST_NAME} test_main.cpp test/test_blp.hpp test/test_basic_par.hpp)
target_link_libraries(${TEST_NAME} ${LIBRARY_NAME})

