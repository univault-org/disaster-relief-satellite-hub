#include <gtest/gtest.h>
#include <iostream>

int main(int argc, char **argv) {
    std::cout << "==== Starting all tests ====" << std::endl;
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    std::cout << "==== Finished all tests ====" << std::endl;
    return result;
}
