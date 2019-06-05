cmake_minimum_required(VERSION 3.14)
project(raft)

set(CMAKE_CXX_STANDARD 17)

#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

add_executable(raft src/main.cpp src/Log.cpp src/Log.h src/node.cpp src/node.h)
target_link_libraries(raft jsoncpp)