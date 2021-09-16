cmake_minimum_required(VERSION 3.16)
project(PA093 VERSION 0.1.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})

include(conanbuildinfo)
conan_basic_setup(TARGETS)

add_subdirectory(src)
