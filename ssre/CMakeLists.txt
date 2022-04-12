cmake_minimum_required(VERSION 3.15)

project(libssre VERSION 0.0.1 LANGUAGES CXX)

file(GLOB_RECURSE sources src/*.cpp src/*.h)
file(GLOB_RECURSE headers include/*.h)

add_library(ssre STATIC ${sources} ${headers})

set_target_properties(ssre PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(ssre
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ssre PUBLIC glm)
target_link_libraries(ssre PUBLIC glad)
target_link_libraries(ssre PUBLIC glfw3)
target_link_libraries(ssre PRIVATE tinyobjloader)

target_include_directories(ssre PUBLIC ${OPENGL_INCLUDE_DIRS})
target_link_libraries(ssre PUBLIC ${OPENGL_LIBRARY})

if(WIN32)
else()
    target_link_libraries(ssre PUBLIC dl X11 pthread)
endif()

message(STATUS "GLFW3 include ${glfw3}")
message(STATUS "GLFW3 link ${GLFW3_LIBRARY}")

message(STATUS "OPENGL include ${OPENGL_INCLUDE_DIRS}")
message(STATUS "OPENGL link ${OPENGL_LIBRARY}")

