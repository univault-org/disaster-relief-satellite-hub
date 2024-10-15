# Find GTest package
find_package(GTest REQUIRED)

# Function to create a test executable
function(create_test_executable name)
    add_executable(${name}
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        test_${name}.cpp
    )

    target_include_directories(${name}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/include/network
            ${CMAKE_SOURCE_DIR}/include/devices
            ${CMAKE_SOURCE_DIR}/lib/srpt-protocol/include
            ${CMAKE_SOURCE_DIR}/lib/srpt-protocol/src
            ${CMAKE_SOURCE_DIR}/lib/riif-ultrasonic/include
            ${CMAKE_SOURCE_DIR}/lib/riif-ultrasonic/src
    )

    target_link_libraries(${name}
        PRIVATE
            GTest::GTest
            GTest::Main
            disaster_relief_hub_lib
            srpt-protocol
            riif_ultrasonic
            ${LIBSODIUM_LIBRARIES}
    )

    gtest_discover_tests(${name})
endfunction()

# Create test executables
create_test_executable(satellite_communication)
create_test_executable(ultrasonic_communication)

# You can add more test executables here if needed
# create_test_executable(listing_feature)
# create_test_executable(preorder_feature)

# Optional: Add messages for debugging
message(STATUS "GTest include dirs: ${GTEST_INCLUDE_DIRS}")
message(STATUS "GTest libraries: ${GTEST_LIBRARIES}")

# Print include directories for debugging
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Include dir: ${dir}")
endforeach()

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
