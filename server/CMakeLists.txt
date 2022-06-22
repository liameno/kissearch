cmake_minimum_required(VERSION 3.21)
project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Ofast")
set(CMAKE_CXX_STANDARD 17)

set(include include/httplib.h include/json.hpp)
set(src )

add_executable(${PROJECT_NAME} main.cpp ${include} ${src})

if (NOT TARGET kissearch)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
endif()

target_link_libraries(${PROJECT_NAME} kissearch)