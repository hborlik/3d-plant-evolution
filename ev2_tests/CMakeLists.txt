project(ev2_tests)

file(GLOB_RECURSE sources "src/*.cpp")
add_executable(ev2_tests ${sources})

target_include_directories(ev2_tests PRIVATE
    "include"
)

target_link_libraries(ev2_tests 
    ev2
)
