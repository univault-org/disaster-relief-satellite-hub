#include <gtest/gtest.h>
#include "riif_ultrasonic.h"
#include "../include/devices/local_communication.h"

TEST(UltrasonicTest, SecureKeyExchange) {
    LocalCommunication communication;
    EXPECT_TRUE(communication.initializeUltrasonic());
    EXPECT_TRUE(communication.performKeyExchange());
}

TEST(UltrasonicTest, SecureDataTransfer) {
    LocalCommunication communication;
    communication.initializeUltrasonic();
    communication.performKeyExchange();
    
    std::string testData = "Secure preorder data";
    EXPECT_TRUE(communication.sendSecureData(testData));
    
    std::string receivedData;
    EXPECT_TRUE(communication.receiveSecureData(receivedData));
    EXPECT_EQ(testData, receivedData);
}
