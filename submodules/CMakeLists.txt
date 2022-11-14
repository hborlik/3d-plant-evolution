

add_subdirectory(p-tree)


add_library(cbt INTERFACE)
target_include_directories(cbt INTERFACE libcbt)

add_library(leb INTERFACE)
target_include_directories(leb INTERFACE libleb)