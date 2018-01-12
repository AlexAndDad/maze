cmake_minimum_required(VERSION 3.9)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(HunterGate)
HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.19.226.tar.gz"
        SHA1 "8753f07f17a8a7b83b35bec6f6a098cd378113c6"
)

project(maze)
hunter_add_package(Sugar)
include(${SUGAR_ROOT}/cmake/Sugar)
include(sugar_include)
include(sugar_files)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14)
endif ()

sugar_include(src)

add_executable(maze ${SOURCE_FILES} main.cpp)
target_include_directories(maze PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src)
