cmake_minimum_required(VERSION 3.21)
project(example)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Ofast")
set(CMAKE_CXX_STANDARD 17)

find_package(ZLIB)
find_package(Threads)

#TODO: edit
#/home/---/libs/lib/libkissearch.so - without sudo
#/usr/local/lib/libkissearch.so

add_executable(${PROJECT_NAME} example.cpp)
target_link_libraries(${PROJECT_NAME} z Threads::Threads /usr/local/lib/libkissearch.so)