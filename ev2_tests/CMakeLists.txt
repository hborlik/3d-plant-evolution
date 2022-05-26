project(ev2_tests)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE include "include/*.h")
add_executable(ev2_tests ${sources} ${include})

target_include_directories(ev2_tests PRIVATE
    "include"
)

# Set where the ImGui files are stored
# set(IMGUI_PATH  "C:/Git/imgui")
    
# Compile as static library 
# file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp) 
# add_library("ImGui" STATIC ${IMGUI_SOURCES})
# target_include_directories("ImGui" PUBLIC ${IMGUI_PATH})

if(WIN32)
    set(CMAKE_CXX_FLAGS "-static-libgcc -static-libstdc++")
endif()

target_link_libraries(ev2_tests 
    ev2
    ptree
)

set_target_properties(ev2_tests PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)