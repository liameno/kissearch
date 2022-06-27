cmake_minimum_required(VERSION 3.21)
project(kissearch)

add_subdirectory("${PROJECT_SOURCE_DIR}/example" "${PROJECT_SOURCE_DIR}/example/build")
add_subdirectory("${PROJECT_SOURCE_DIR}/server" "${PROJECT_SOURCE_DIR}/server/build")
add_subdirectory("${PROJECT_SOURCE_DIR}/tests" "${PROJECT_SOURCE_DIR}/tests/build")