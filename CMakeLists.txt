cmake_minimum_required(VERSION 3.10)
project(DisasterReliefSatelliteHub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages for srpt-protocol
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSODIUM REQUIRED libsodium)

# Add submodules
add_subdirectory(lib/srpt-protocol)
add_subdirectory(lib/riif-ultrasonic)

# Include directories
include_directories(include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/srpt-protocol/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/srpt-protocol/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/riif-ultrasonic/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/riif-ultrasonic/src)
include_directories(${LIBSODIUM_INCLUDE_DIRS})

# Add source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/business/*.cpp"
    "src/network/*.cpp"
    "src/devices/*.cpp"
)

# Set include directories for riif_ultrasonic
target_include_directories(riif_ultrasonic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/riif-ultrasonic/include
)

# Create main library
add_library(disaster_relief_hub_lib ${SOURCES})

# Link submodule libraries to main library
target_link_libraries(disaster_relief_hub_lib
    PUBLIC
        srpt-protocol
        riif_ultrasonic
        ${LIBSODIUM_LIBRARIES}
)

# Create main executable (uncomment when you want to build the main application)
# add_executable(${PROJECT_NAME} src/main.cpp)
# target_link_libraries(${PROJECT_NAME} 
#     PRIVATE 
#         disaster_relief_hub_lib
# )

# Find GTest package
find_package(GTest REQUIRED)

# Enable testing
enable_testing()

# Add subdirectory for tests
add_subdirectory(tests)

# Optional: Add messages for debugging
message(STATUS "Project source dir: ${PROJECT_SOURCE_DIR}")
message(STATUS "Project binary dir: ${PROJECT_BINARY_DIR}")

# Print include directories for debugging
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Include dir: ${dir}")
endforeach()
