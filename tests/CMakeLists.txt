project(tests)

file(GLOB_RECURSE include "include/*.h")
add_executable(tests "src/main.cpp" ${include})

target_include_directories(tests PRIVATE
    "include"
)

set_target_properties(tests PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

add_executable(ptree_perf "src/p_tree.cpp" ${include})
target_include_directories(ptree_perf PRIVATE
    "include"
)
set_target_properties(ptree_perf PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_link_libraries(ptree_perf PRIVATE ptree)

add_executable(shader_test "src/shader_test.cpp" ${include})
target_include_directories(shader_test PRIVATE
    "include"
)
set_target_properties(shader_test PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_link_libraries(shader_test PRIVATE ev2)