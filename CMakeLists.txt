cmake_minimum_required(VERSION 3.21)
project(kissearch)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
set(CMAKE_CXX_STANDARD 17)

set(include include/str.h include/porter2.h include/lz4_frame_wrapper.h include/document.h include/entry.h)
set(src src/porter2.cpp include/lz4_frame_wrapper.h src/document.cpp src/entry.cpp)

find_library(LZ4_LIBRARY NAMES lz4)
add_executable(${PROJECT_NAME} main.cpp ${include} ${src})
target_link_libraries(${PROJECT_NAME} lz4)