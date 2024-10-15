# Find GTest package
find_package(GTest REQUIRED)

# Common test sources (if any)
set(COMMON_TEST_SOURCES
    # Add any common test source files here
)

# Define test executables
set(TEST_EXECUTABLES
    test_satellite_communication
    test_ultrasonic_communication
    test_listing_feature
    test_preorder_feature
)

# Create test targets
foreach(test_name ${TEST_EXECUTABLES})
    add_executable(${test_name} 
        ${test_name}.cpp
        ${COMMON_TEST_SOURCES}
    )

    target_include_directories(${test_name}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/include/network
            ${CMAKE_SOURCE_DIR}/lib/srpt-protocol/include
            ${CMAKE_SOURCE_DIR}/lib/srpt-protocol/src
            ${CMAKE_SOURCE_DIR}/lib/riif-ultrasonic/include
    )

    target_link_libraries(${test_name}
        PRIVATE
            GTest::GTest
            GTest::Main
            disaster_relief_hub_lib
            srpt-protocol
            riif_ultrasonic
    )

    # Discover and add the tests
    gtest_discover_tests(${test_name})
endforeach()

# Optional: Add messages for debugging
message(STATUS "GTest include dirs: ${GTEST_INCLUDE_DIRS}")
message(STATUS "GTest libraries: ${GTEST_LIBRARIES}")

# Print include directories for debugging
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Include dir: ${dir}")
endforeach()


# Add this at the end of the file
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")

foreach(test_name ${TEST_EXECUTABLES})
    get_target_property(test_include_dirs ${test_name} INCLUDE_DIRECTORIES)
    message(STATUS "${test_name} include directories: ${test_include_dirs}")
endforeach()
