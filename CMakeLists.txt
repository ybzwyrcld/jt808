cmake_minimum_required(VERSION 2.8)

project(jt808)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

option(JT808_BUILD_EXAMPLES "Build jt808 examples" OFF)

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O2 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wfatal-errors")

set(CMAKE_LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_INCLUDE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/include)

set(jt808_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(SYSTEM ${jt808_include_dirs})

aux_source_directory(src DIR_SRCS)

# add_subdirectory(nmeaparser)

add_library(${PROJECT_NAME}
  ${DIR_SRCS}
)

if(WIN32)
target_link_libraries(${PROJECT_NAME}
  ws2_32
)
endif(WIN32)

set_target_properties(${PROJECT_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${jt808_include_dirs})

if (JT808_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif (JT808_BUILD_EXAMPLES)
