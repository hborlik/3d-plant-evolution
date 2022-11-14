cmake_minimum_required(VERSION 3.15)

project(ev2 VERSION 0.0.1 LANGUAGES CXX)

file(GLOB_RECURSE sources src/*.cpp src/*.h)
file(GLOB_RECURSE headers include/*.h)

add_library(ev2 STATIC ${sources} ${headers})

set_target_properties(ev2 PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(ev2
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ev2 PUBLIC glm glad glfw3 tinyobjloader tinygltfloader reactphysics3d)

target_include_directories(ev2 PUBLIC ${OPENGL_INCLUDE_DIRS})
target_link_libraries(ev2 PUBLIC ${OPENGL_LIBRARY})

target_link_libraries(ev2 PRIVATE leb cbt)

if(WIN32)
else()
    target_link_libraries(ev2 PUBLIC dl X11 pthread)
endif()

message(STATUS "GLFW3 include ${glfw3}")
message(STATUS "GLFW3 link ${GLFW3_LIBRARY}")

message(STATUS "OPENGL include ${OPENGL_INCLUDE_DIRS}")
message(STATUS "OPENGL link ${OPENGL_LIBRARY}")

