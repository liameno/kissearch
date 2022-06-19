cmake_minimum_required(VERSION 3.21)
project(kissearch)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Ofast")
set(CMAKE_CXX_STANDARD 17)

set(include include/str.h include/porter2.h include/document.h include/entry.h include/compression.h)
set(src src/porter2.cpp src/document.cpp src/entry.cpp src/compression.cpp)

find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} SHARED ${include} ${src})
target_include_directories(${PROJECT_NAME} PUBLIC include)

include(GNUInstallDirs)

install(FILES ${include} DESTINATION include/${PROJECT_NAME})
install(FILES ${src} DESTINATION src/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})