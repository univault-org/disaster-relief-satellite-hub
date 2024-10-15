#include <gtest/gtest.h>
#include "srpt_satellite.h"
#include "../include/network/satellite_hub.h"
#include "srpt.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <atomic>

class SatelliteHubTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::cout << "==== SatelliteHubTest::SetUp() called ====" << std::endl;
        std::cout << "Initializing SRPT..." << std::endl;
        ASSERT_TRUE(hub.initializeSRPT()) << "Failed to initialize SRPT";
        std::cout << "SRPT initialized successfully" << std::endl;
        std::cout << "Connecting to satellite..." << std::endl;
        ASSERT_TRUE(hub.connectToSatellite()) << "Failed to connect to satellite";
        std::cout << "Connected to satellite successfully" << std::endl;
    }

    void TearDown() override {
        std::cout << "SatelliteHubTest::TearDown() called" << std::endl;
    }

    SatelliteHub hub;
};

TEST_F(SatelliteHubTest, EstablishConnection) {
    std::cout << "Running EstablishConnection test" << std::endl;
    // Connection is established in SetUp
    EXPECT_TRUE(true) << "EstablishConnection test completed";
}

TEST_F(SatelliteHubTest, SendReceiveData) {
    std::string testData = "Test listing data";
    EXPECT_TRUE(hub.sendData(testData));
    
    std::string receivedData;
    EXPECT_TRUE(hub.receiveData(receivedData));
    EXPECT_EQ(testData, receivedData);
}

TEST_F(SatelliteHubTest, SendLargeData) {
    std::string largeData(1024 * 1024, 'A'); // 1MB of data
    EXPECT_TRUE(hub.sendData(largeData));
    
    std::string receivedData;
    EXPECT_TRUE(hub.receiveData(receivedData));
    EXPECT_EQ(largeData, receivedData);
}

TEST_F(SatelliteHubTest, ConcurrentSendReceive) {
    const int numMessages = 100;
    std::vector<std::string> sentMessages;
    std::vector<std::string> receivedMessages;
    std::atomic<bool> sendingComplete(false);

    auto sendFunc = [&]() {
        for (int i = 0; i < numMessages; ++i) {
            std::string msg = "Message " + std::to_string(i);
            EXPECT_TRUE(hub.sendData(msg));
            sentMessages.push_back(msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        sendingComplete.store(true);
    };

    auto receiveFunc = [&]() {
        while (!sendingComplete.load() || receivedMessages.size() < numMessages) {
            std::string msg;
            if (hub.receiveData(msg)) {
                receivedMessages.push_back(msg);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    std::thread sendThread(sendFunc);
    std::thread receiveThread(receiveFunc);

    sendThread.join();
    receiveThread.join();

    EXPECT_EQ(sentMessages.size(), receivedMessages.size());
    for (size_t i = 0; i < std::min(sentMessages.size(), receivedMessages.size()); ++i) {
        EXPECT_EQ(sentMessages[i], receivedMessages[i]) << "Mismatch at index " << i;
    }
}
