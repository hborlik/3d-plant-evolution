cmake_minimum_required(VERSION 3.13)

project(gl_testapp C CXX)

include(cmake/get_cpm.cmake)

add_subdirectory(third-party)
add_subdirectory(ev2)
add_subdirectory(ev2_tests)
add_subdirectory(p-tree)
# add_subdirectory(testapp)
# add_subdirectory(vfc_base2022v2)
# add_subdirectory(SunnyBunnyBase2022)
# add_subdirectory(DeferBase22)