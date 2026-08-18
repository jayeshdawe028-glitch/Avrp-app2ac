cmake_minimum_required(VERSION 3.12)

project(multirium)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/opus/include
)

# Optional dependencies
# find_package(shadowhook REQUIRED CONFIG)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--build-id")

# Subprojects
add_subdirectory(opus)
add_subdirectory(multirium)
# add_subdirectory(debugtools)

# Compiler optimizations
target_compile_options(opus PRIVATE -O3)
