#include <gtest/gtest.h>
#include "srpt_satellite.h"
#include "../include/network/satellite_hub.h"
#include "srpt.h"

TEST(SatelliteHubTest, EstablishConnection) {
    SatelliteHub hub;
    EXPECT_TRUE(hub.initializeSRPT());
    EXPECT_TRUE(hub.connectToSatellite());
}

TEST(SatelliteHubTest, SendReceiveData) {
    SatelliteHub hub;
    hub.initializeSRPT();
    hub.connectToSatellite();
    
    std::string testData = "Test listing data";
    EXPECT_TRUE(hub.sendData(testData));
    
    std::string receivedData;
    EXPECT_TRUE(hub.receiveData(receivedData));
    EXPECT_EQ(testData, receivedData);
}

