cmake_minimum_required(VERSION 3.16)
project(example LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add Raylib dependency (via package or source)
find_package(raylib QUIET)  # Look for system-installed Raylib

# If not found, use Raylib source
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 4.5.0  # Use the latest stable version or your preferred one
    )
    FetchContent_MakeAvailable(raylib)
endif()

# Add source files
add_executable(example main.cpp)

# Link Raylib library
target_link_libraries(example PRIVATE raylib)

# Add required flags for specific platforms
if (WIN32)
    target_link_libraries(example PRIVATE Winmm.lib)
elseif (APPLE)
    target_link_libraries(example PRIVATE "-framework Cocoa" "-framework IOKit" "-framework CoreFoundation" "-framework CoreVideo")
endif()
