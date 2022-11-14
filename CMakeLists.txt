cmake_minimum_required(VERSION 3.24)

project(gl_testapp C CXX)

include(cmake/get_cpm.cmake)

add_subdirectory(third-party)
add_subdirectory(ev2)
add_subdirectory(3d_tree_ex)
add_subdirectory(tests)
add_subdirectory(submodules)