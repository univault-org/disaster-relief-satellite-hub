#pragma once

#include "riif_ultrasonic.h"
#include <string>
#include <vector>

class LocalCommunication {
public:
    LocalCommunication();
    ~LocalCommunication();

    bool initializeUltrasonic();
    bool performKeyExchange();
    bool sendSecureData(const std::string& data);
    bool receiveSecureData(std::string& data);

private:
    RiifUltrasonic m_ultrasonic;
    std::vector<uint8_t> m_sharedKey;
};

