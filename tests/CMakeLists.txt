cmake_minimum_required(VERSION 3.5)

# Manually set PortAudio paths
set(PORTAUDIO_INCLUDE_DIRS "/usr/local/Cellar/portaudio/19.7.0/include")
set(PORTAUDIO_LIBRARIES "/usr/local/Cellar/portaudio/19.7.0/lib/libportaudio.dylib")

# Find GTest package
find_package(GTest REQUIRED)

# Check if PortAudio paths are valid
if(NOT EXISTS "${PORTAUDIO_INCLUDE_DIRS}/portaudio.h")
    message(FATAL_ERROR "PortAudio header not found at ${PORTAUDIO_INCLUDE_DIRS}/portaudio.h")
endif()

if(NOT EXISTS "${PORTAUDIO_LIBRARIES}")
    message(FATAL_ERROR "PortAudio library not found at ${PORTAUDIO_LIBRARIES}")
endif()

message(STATUS "PortAudio include dir: ${PORTAUDIO_INCLUDE_DIRS}")
message(STATUS "PortAudio library: ${PORTAUDIO_LIBRARIES}")

# Set ggwave paths
set(GGWAVE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/ggwave/include")
set(GGWAVE_BUILD_DIR "${CMAKE_SOURCE_DIR}/lib/ggwave/build")

# Try to find the ggwave library
find_library(GGWAVE_LIBRARIES
    NAMES ggwave libggwave
    PATHS ${GGWAVE_BUILD_DIR}/src
    NO_DEFAULT_PATH
)

if(NOT GGWAVE_LIBRARIES)
    message(FATAL_ERROR "ggwave library not found in ${GGWAVE_BUILD_DIR}/src")
else()
    message(STATUS "Found ggwave library: ${GGWAVE_LIBRARIES}")
endif()

# Check if ggwave header exists
if(NOT EXISTS "${GGWAVE_INCLUDE_DIRS}/ggwave/ggwave.h")
    message(FATAL_ERROR "ggwave header not found at ${GGWAVE_INCLUDE_DIRS}/ggwave/ggwave.h")
endif()

message(STATUS "ggwave include dir: ${GGWAVE_INCLUDE_DIRS}")

# Add test executable
add_executable(runUnitTests
    test_preorder_feature.cpp
)

# Include directories
target_include_directories(runUnitTests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${PORTAUDIO_INCLUDE_DIRS}
        ${GGWAVE_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(runUnitTests
    PRIVATE
        ${GGWAVE_LIBRARIES}
        ${PORTAUDIO_LIBRARIES}
        GTest::GTest
        GTest::Main
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(runUnitTests)

# Optional: Add messages for debugging
message(STATUS "GTest include dirs: ${GTEST_INCLUDE_DIRS}")
message(STATUS "GTest libraries: ${GTEST_LIBRARIES}")

# Add these debug messages at the end of the file
message(STATUS "GGWAVE_LIBRARIES: ${GGWAVE_LIBRARIES}")
if(EXISTS "${GGWAVE_LIBRARIES}")
    message(STATUS "GGWAVE_LIBRARIES file exists")
else()
    message(WARNING "GGWAVE_LIBRARIES file does not exist")
endif()

message(STATUS "PORTAUDIO_LIBRARIES: ${PORTAUDIO_LIBRARIES}")
if(EXISTS "${PORTAUDIO_LIBRARIES}")
    message(STATUS "PORTAUDIO_LIBRARIES file exists")
else()
    message(WARNING "PORTAUDIO_LIBRARIES file does not exist")
endif()

message(STATUS "GTEST_LIBRARIES: ${GTEST_LIBRARIES}")

# Print include directories for debugging
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Include dir: ${dir}")
endforeach()

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")

# Add this line to ensure the ggwave library can be found at runtime
set_target_properties(runUnitTests PROPERTIES
    INSTALL_RPATH "${GGWAVE_BUILD_DIR}/src"
)
