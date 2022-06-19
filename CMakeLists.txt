cmake_minimum_required(VERSION 3.21)
project(kissearch)

add_subdirectory("${PROJECT_SOURCE_DIR}/lib" "${PROJECT_SOURCE_DIR}/lib/build")
add_subdirectory("${PROJECT_SOURCE_DIR}/example" "${PROJECT_SOURCE_DIR}/example/build")