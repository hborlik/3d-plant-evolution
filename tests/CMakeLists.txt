project(tests)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE include "include/*.h")
add_executable(tests ${sources} ${include})

target_include_directories(tests PRIVATE
    "include"
)

target_link_libraries(tests 
    
)

set_target_properties(tests PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)