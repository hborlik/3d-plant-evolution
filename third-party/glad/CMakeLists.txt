cmake_minimum_required(VERSION 3.13)

file(GLOB_RECURSE sources "src/*.c")

add_library(glad STATIC ${sources})

target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)