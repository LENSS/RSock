cmake_minimum_required(VERSION 2.6)
project(hrp)

if (APPLE)
set(CMAKE_MACOSX_RPATH ON)
endif (APPLE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -std=c++11")

include_directories(src include)
include_directories("staging_dir/include")
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)
include_directories("/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers/")
include_directories("$ENV{JAVA_HOME}/include")
include_directories("$ENV{JAVA_HOME}/include/darwin")

if (APPLE)
    link_directories("/usr/local/lib")
endif()

add_subdirectory(src)
#add_subdirectory(tests)

add_custom_target(hrpd)
add_dependencies(hrpd rsockd)

