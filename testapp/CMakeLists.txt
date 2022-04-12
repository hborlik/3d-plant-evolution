project(testapp)

file(GLOB_RECURSE sources "src/*.cpp")
add_executable(testapp ${sources})

target_include_directories(testapp PRIVATE
    "include"
)

target_link_libraries(testapp 
    ssre
)
