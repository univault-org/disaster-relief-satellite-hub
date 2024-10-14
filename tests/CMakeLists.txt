# Find GTest package
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Common test sources (if any)
set(COMMON_TEST_SOURCES
    # Add any common test source files here
)

# Purchase Flow Test
add_executable(test_purchase_flow
    test_purchase_flow.cpp
    ${COMMON_TEST_SOURCES}
)

target_include_directories(test_purchase_flow
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/devices
        ${CMAKE_SOURCE_DIR}/src/blockchain
        ${CMAKE_SOURCE_DIR}/src/voucher
        ${CMAKE_SOURCE_DIR}/src/crypto
)

target_link_libraries(test_purchase_flow
    PRIVATE
        GTest::GTest
        GTest::Main
        # Add any other libraries needed for the purchase flow test
)

# Delivery Flow Test
add_executable(test_delivery_flow
    test_delivery_flow.cpp
    ${COMMON_TEST_SOURCES}
)

target_include_directories(test_delivery_flow
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/devices
        ${CMAKE_SOURCE_DIR}/src/blockchain
        ${CMAKE_SOURCE_DIR}/src/voucher
        ${CMAKE_SOURCE_DIR}/src/crypto
)

target_link_libraries(test_delivery_flow
    PRIVATE
        GTest::GTest
        GTest::Main
        # Add any other libraries needed for the delivery flow test
)

# Discover and add the tests
include(GoogleTest)
gtest_discover_tests(test_purchase_flow)
gtest_discover_tests(test_delivery_flow)

# Optional: Add messages for debugging
message(STATUS "GTest include dirs: ${GTEST_INCLUDE_DIRS}")
message(STATUS "GTest libraries: ${GTEST_LIBRARIES}")