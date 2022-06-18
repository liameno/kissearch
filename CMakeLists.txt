cmake_minimum_required(VERSION 3.21)
project(kissearch)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
set(CMAKE_CXX_STANDARD 17)

set(include include/str.h include/porter2.h include/document.h include/entry.h include/compression.h)
set(src src/porter2.cpp src/document.cpp src/entry.cpp src/compression.cpp)

find_library(z ZLIB)
add_executable(${PROJECT_NAME} main.cpp ${include} ${src})
target_link_libraries(${PROJECT_NAME} z)