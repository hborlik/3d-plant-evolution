set(GLM_VERSION "0.9.9.5")
set(GLFW3_VERSION "3.3")

# glm
# default search dirs

include(ExternalProject)

CPMAddPackage(
    NAME glm
    GITHUB_REPOSITORY g-truc/glm
    GIT_TAG 0.9.9.8
)
if(glm_ADDED)
    # add_library(glm INTERFACE IMPORTED)
    target_include_directories(glm INTERFACE ${glm_SOURCE_DIR})
endif()

# ExternalProject_Add(GLM_UTIL
#     PREFIX glm
#     GIT_REPOSITORY https://github.com/g-truc/glm.git
#     GIT_TAG ${GLM_VERSION}
#     INSTALL_COMMAND ""
#     UPDATE_COMMAND ""
#     LOG_DOWNLOAD ON
# )
# ExternalProject_Get_Property(GLM_UTIL SOURCE_DIR)
# ExternalProject_Get_Property(GLM_UTIL BINARY_DIR)
# set(GLM_INCLUDE_DIR ${SOURCE_DIR})
# set(GLM_BINARY_DIR ${BINARY_DIR})
# message(STATUS "GLM Source Dir: ${SOURCE_DIR}")
# message(STATUS "GLM Binary Dir: ${BINARY_DIR}")

# add_library(glm INTERFACE)
# add_dependencies(glm INTERFACE GLM_UTIL)
# target_link_libraries(glm INTERFACE ${GLM_BINARY_DIR}/glm/libglm_static.a)
# target_include_directories(glm INTERFACE ${GLM_INCLUDE_DIR})

# glfw3

ExternalProject_Add(GLFW3_UTIL
    PREFIX glfw3
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG ${GLFW3_VERSION}
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    LOG_DOWNLOAD ON
)
ExternalProject_Get_Property(GLFW3_UTIL SOURCE_DIR)
ExternalProject_Get_Property(GLFW3_UTIL BINARY_DIR)
set(GLFW3_INCLUDE_DIR ${SOURCE_DIR}/include)
set(GLFW3_BINARY_DIR ${BINARY_DIR}/src)
message(STATUS "GLFW3 Source Dir: ${SOURCE_DIR}")
message(STATUS "GLFW3 Binary Dir: ${BINARY_DIR}")

add_library(glfw3 INTERFACE)
add_dependencies(glfw3 INTERFACE GLFW3_UTIL)
target_link_libraries(glfw3 INTERFACE ${GLFW3_BINARY_DIR}/libglfw3.a)
target_include_directories(glfw3 INTERFACE ${GLFW3_INCLUDE_DIR})

# OTHERS

find_package(OpenGL REQUIRED)

add_subdirectory(glad)
add_subdirectory(tinyobjloader)

add_library(tinygltfloader INTERFACE)
target_include_directories(tinygltfloader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tinygltfloader)