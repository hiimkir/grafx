cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(viewer3d VERSION 1.0)

add_subdirectory(doctest)
add_subdirectory(tests)
add_subdirectory(grafx)

# if downloaded from packet manager
# find_package(SDL3)

# build from source
include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG vendored/SDL
    )
FetchContent_MakeAvailable(SDL3)

# add dynamic lib to output folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_executable(viewer3d main.cpp)
target_link_libraries(viewer3d PRIVATE SDL3::SDL3)