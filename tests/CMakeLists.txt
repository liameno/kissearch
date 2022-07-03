cmake_minimum_required(VERSION 3.21)
project(tests)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Ofast")
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp)

find_package(Catch2 REQUIRED)

if (NOT TARGET kissearch)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
endif()

target_link_libraries(${PROJECT_NAME} kissearch Catch2::Catch2)