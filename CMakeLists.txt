cmake_minimum_required(VERSION 3.10)
project(DisasterReliefSatelliteHub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create main executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Find GTest package
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Enable testing
enable_testing()

# Add subdirectory for tests
add_subdirectory(tests)

# Link libraries for the main executable
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        # Add any libraries needed for the main executable
        # For example:
        # ${GTEST_LIBRARIES}
        # pthread
)

# Optional: Add messages for debugging
message(STATUS "Project source dir: ${PROJECT_SOURCE_DIR}")
message(STATUS "Project binary dir: ${PROJECT_BINARY_DIR}")
