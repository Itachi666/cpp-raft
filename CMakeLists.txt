cmake_minimum_required(VERSION 3.14)
project(raft)

set(CMAKE_CXX_STANDARD 17)

#link_libraries()

add_executable(server src/server.cpp)
add_executable(raft src/main.cpp src/Log.cpp src/Log.h src/node.cpp src/node.h)
target_link_libraries(raft jsoncpp)