file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE include "include/*.h")
add_executable(3d_tree_ex ${sources} ${include})

target_include_directories(3d_tree_ex PRIVATE
    "include"
)

if(WIN32)
    set(CMAKE_CXX_FLAGS "-static-libgcc -static-libstdc++")
endif()

target_link_libraries(3d_tree_ex 
    ev2
    ptree
)

set_target_properties(3d_tree_ex PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# still need to install assets!

# copy assets to build directory
add_custom_command(
    TARGET 3d_tree_ex PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/asset $<TARGET_FILE_DIR:3d_tree_ex>/asset
)
