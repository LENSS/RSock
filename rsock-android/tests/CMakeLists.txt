cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_STANDARD 11)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ../src)

add_definitions(-DASIO_STANDALONE)

add_executable(runHrpCoreTests testHrpCore.cpp)
add_executable(runRMTests testRM.cpp testPILearner.cpp)
add_executable(runPILearnerSpeedTests testPILearnerSpeed.cpp)
add_executable(runMVETests testMVECore.cpp testMVEEstimator.cpp)
add_executable(runTxrxEngineTests testTxrxEngine.cpp)
add_executable(runHrpMessagesTests testHrpMessages.cpp)
add_executable(runHrpGraphTests testHrpGraph.cpp)

target_link_libraries(runHrpCoreTests hrp gtest gtest_main pthread)
target_link_libraries(runRMTests hrp gtest gtest_main pthread)
target_link_libraries(runPILearnerSpeedTests hrp gtest gtest_main pthread)
target_link_libraries(runMVETests hrp gtest gtest_main pthread)
target_link_libraries(runTxrxEngineTests hrp gtest gtest_main pthread)
target_link_libraries(runHrpMessagesTests hrp gtest gtest_main pthread)
target_link_libraries(runHrpGraphTests hrp gtest gtest_main pthread)