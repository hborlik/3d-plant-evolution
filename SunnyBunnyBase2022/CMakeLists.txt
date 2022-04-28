cmake_minimum_required(VERSION 3.4)
# include("${CMAKE_CURRENT_SOURCE_DIR}/FindGfxLibs.cmake")

# Name of the project
project(FBO)

# Enforce use of C++14
set(CMAKE_CXX_STANDARD_REQUIRED 14)
set(CMAKE_CXX_STANDARD 14)

# Use glob to get the list of all source files.
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/ext/*/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/ext/glad/src/*.c")

# We don't really need to include header and resource files to build, but it's
# nice to have them show up in IDEs.
file(GLOB_RECURSE HEADERS "src/*.h" "ext/*/*.h" "ext/glad/*/*.h")
file(GLOB_RECURSE GLSL "resources/*.glsl")

include_directories("ext")
include_directories("ext/glad/include")

# Set the executable.
add_executable(FBO ${SOURCES} ${HEADERS} ${GLSL})

# Helper function included from FindGfxLibs.cmake
# findGLFW3(${CMAKE_PROJECT_NAME})
# findGLM(${CMAKE_PROJECT_NAME})

set_target_properties(FBO PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    LINKER_LANGUAGE CXX
)

target_link_libraries(FBO PUBLIC glfw3 glm)

# OS specific options and libraries
if(NOT WIN32)

  message(STATUS "Adding GCC style compiler flags")
  add_compile_options("-Wall")
  add_compile_options("-pedantic")
  add_compile_options("-Werror=return-type")
 
  # TODO: The following links may be uneeded. 
  if(APPLE)
    # Add required frameworks for GLFW.
    target_link_libraries(FBO "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
  else()
    #Link the Linux OpenGL library
    target_link_libraries(FBO PUBLIC GL dl X11 pthread)
  endif()

else()

  # Link OpenGL on Windows
  target_link_libraries(FBO PUBLIC opengl32.lib)

endif()
